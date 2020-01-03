// See LICENSE for license details.

#include "sim.h"
#include "mmu.h"
#include "dts.h"
#include "remote_bitbang.h"
#include <map>
#include <iostream>
#include <fstream>
#include <sstream>
#include <climits>
#include <cstdlib>
#include <cassert>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

//L1 buffer size adjust to 1.25M
//im buffer size adjust to 256K
#define ddr_mem_start        (0x00000000)
#define l1_buffer_start      (0xc0000000)
#define l1_buffer_size       (0x00140000)
#define im_buffer_start      (0xc0400000)
#define im_buffer_size       (0x00040000)
volatile bool ctrlc_pressed = false;
static void handle_signal(int sig)
{
  if (ctrlc_pressed)
    exit(-1);
  
  ctrlc_pressed = true;
  signal(sig, &handle_signal);
}

sim_t::sim_t(const char* isa, size_t nprocs, bool halted, reg_t start_pc,
             std::vector<std::pair<reg_t, mem_t*>> mems, size_t ddr_size,
             const std::vector<std::string>& args,
             std::vector<int> const hartids, unsigned progsize,
             unsigned max_bus_master_bits, bool require_authentication,
             suseconds_t abstract_delay_usec, bool support_hasel,
             bool support_abstract_csr_access, bool mac_enabled)
  : htif_t(args), mems(mems), procs(std::max(nprocs, size_t(1))),
    local_bus(std::max(nprocs, size_t(1))),
    start_pc(start_pc), current_step(0), current_proc(0), debug(false),
    histogram_enabled(false), dtb_enabled(true), remote_bitbang(NULL),
    debug_module(this, progsize, max_bus_master_bits, require_authentication,
        abstract_delay_usec, support_hasel,
        support_abstract_csr_access)
{
  //char add_debug_dev = 1;
  //char add_clint_dev = 1;

  signal(SIGINT, &handle_signal);
  mem_ac_enabled = mac_enabled;
  aunit = MCU;
  
  bus.add_device(SYSDMA0_BASE, new sysdma_device_t(0, procs));
  bus.add_device(SYSDMA1_BASE, new sysdma_device_t(1, procs));
  bus.add_device(SYSDMA2_BASE, new sysdma_device_t(2, procs));
  bus.add_device(SYSDMA3_BASE, new sysdma_device_t(3, procs));
  bus.add_device(SYSDMA4_BASE, new sysdma_device_t(4, procs));
  bus.add_device(SYSDMA5_BASE, new sysdma_device_t(5, procs));
  bus.add_device(SYSDMA6_BASE, new sysdma_device_t(6, procs));
  bus.add_device(SYSDMA7_BASE, new sysdma_device_t(7, procs));

  bus.add_device(0xc07f3000, new uart_device_t());
  for (auto& x : mems) {
      bus.add_device(x.first, x.second);
      //if (x.first <= DEBUG_START && (x.first + x.second->size()) > DEBUG_START)
      //  add_debug_dev = 0;
      //if (x.first <= CLINT_BASE && (x.first + x.second->size()) > CLINT_BASE)
      //  add_clint_dev = 0;
  }

  for (size_t i = 0; i < procs.size(); i++) {
    local_bus[i] = new bus_t();
    local_bus[i]->add_device(l1_buffer_start, new mem_t(l1_buffer_size));
    local_bus[i]->add_device(im_buffer_start, new mem_t(im_buffer_size));
  }

  debug_module.add_device(&bus);
  /* debug_module default base is DEBUG_START=0x80100.
   * if our memory is bigger than this, debug_module
   * may caught a mistake when read/write addr 0x80100,
   * for the cache will change to visit debug_module but
   * we want to visit mem infact.
   */
  //if (add_debug_dev)
  //  debug_module.add_device(&bus);

  debug_mmu = new mmu_t(this, NULL);

  if (hartids.size() == 0) {
    for (size_t i = 0; i < procs.size(); i++) {
      procs[i] = new processor_t(isa, this, i, i, halted);
    }
  }
  else {
    if (hartids.size() != procs.size()) {
      std::cerr << "Number of specified hartids doesn't match number of processors" << strerror(errno) << std::endl;
      exit(1);
    }
    for (size_t i = 0; i < procs.size(); i++) {
      procs[i] = new processor_t(isa, this, i, hartids[i], halted);
    }
  }

  clint.reset(new clint_t(procs));
  bus.add_device(CLINT_BASE, clint.get());

  /* the same with debug_module */
  //if (add_clint_dev)
  //  bus.add_device(CLINT_BASE, clint.get());

  if (ddr_size > 0) {
    bus.add_device(ddr_mem_start, new ddr_mem_t(procs, ddr_size));
  }
}

sim_t::~sim_t()
{
  for (size_t i = 0; i < procs.size(); i++) {
    delete procs[i];
    delete local_bus[i];
  }
  delete debug_mmu;
}

void sim_thread_main(void* arg)
{
  ((sim_t*)arg)->main();
}

void sim_t::main()
{
  if (!debug && log)
    set_procs_debug(true);

  while (!done())
  {
    if (debug || ctrlc_pressed)
      interactive();
    else
      step(INTERLEAVE);
    if (remote_bitbang) {
      remote_bitbang->tick();
    }
  }
}

void sim_t::load_heap(const char *fname, reg_t off, size_t len)
{
  memif_t mem(this);
  if (!fname)
    return;

  set_aunit(NCP, 0);
  std::cout << "Load heap off 0x" << std::hex << off
            << " len 0x" << std::hex << len
            << " to file " << fname << std::endl;
  std::string name = std::string(fname);
  std::string suffix_str = name.substr(name.find_last_of('.') + 1);

  if (suffix_str == "dat") {
    std::ifstream ifs(fname, std::ios::in);
    if (!ifs.is_open()) {
        std::cout << __FUNCTION__ << ": Error opening file";
        goto out;;
    }

    char buf[512];
    addr_t addr = off;
    while(!ifs.eof()) {
      ifs.getline(buf, 512);
      char *p = buf;
      for (int i = 0; i < 64; i++, p += 5, addr += 2) {
        uint16_t data = (uint16_t)strtol(p, NULL, 16);
        mem.write(addr, 2, &data);
      }
    }
    ifs.close();
  } else if (suffix_str == "bin") {
    std::ifstream ifs(fname, std::ios::in | std::ios::binary);
    if (!ifs.is_open()) {
        std::cout << __FUNCTION__ << ": Error opening file";
        goto out;
    std::cout << "ERROR: unsupport load bin file now......" << std::endl;
    exit(1);
    }
  } else {
      std::cout << __FUNCTION__ << ": Unsupported file type " << suffix_str << std::endl;
      exit(1);
  }
  
out:
  set_aunit(MCU, 0);
}

void sim_t::dump_heap(const char *fname, reg_t off, size_t len)
{
  memif_t mem(this);
  if (!fname)
    return;

  set_aunit(NCP, 0);
  std::cout << "Dump heap off 0x" << std::hex << off
            << " len 0x" << std::hex << len
            << " to file " << fname << std::endl;
  std::string name = std::string(fname);
  std::string suffix_str = name.substr(name.find_last_of('.') + 1);

  if (suffix_str == "dat") {
    std::ofstream ofs(fname, std::ios::out);
    if (!ofs.is_open()) {
        std::cout << __FUNCTION__ << ": Error opening file";
        goto out;
    }

    uint16_t data;
    char buf[5];
    for (addr_t addr = 0; addr < len; addr += 2) {
      mem.read(off + addr, 2, &data);
      sprintf(buf, "%04x", data);
      ofs << buf << " ";
      if (!((addr + 2) % 128))
        ofs << endl;
    }

    ofs.close();
  } else if (suffix_str == "bin") {
    std::ifstream ifs(fname, std::ios::in | std::ios::binary);
    if (!ifs.is_open()) {
        std::cout << __FUNCTION__ << ": Error opening file";
        goto out;
    std::cout << "ERROR: unsupport load bin file now......" << std::endl;
    exit(1);
    }
  } else {
      std::cout << __FUNCTION__ << ": Unsupported file type " << suffix_str << std::endl;
      exit(1);
  }
  
out:
  set_aunit(MCU, 0);
}

int sim_t::run(const char *fname_load, const char *fname_dump, addr_t off, size_t len)
{
  int stat;
  host = context_t::current();
  target.init(sim_thread_main, this);
  load_heap(fname_load, off, len);
  stat = htif_t::run();
  dump_heap(fname_dump, off, len);
  return stat;
}

void sim_t::step(size_t n)
{
  for (size_t i = 0, steps = 0; i < n; i += steps)
  {
    steps = std::min(n - i, INTERLEAVE - current_step);
    procs[current_proc]->step(steps);

    current_step += steps;
    if (current_step == INTERLEAVE)
    {
      current_step = 0;
      procs[current_proc]->get_mmu()->yield_load_reservation();
      if (++current_proc == procs.size()) {
        current_proc = 0;
        clint->increment(INTERLEAVE / INSNS_PER_RTC_TICK);
      }

      host->switch_to();
    }
  }
}

void sim_t::set_debug(bool value)
{
  debug = value;
}

void sim_t::set_log(bool value)
{
  log = value;
}

void sim_t::set_histogram(bool value)
{
  histogram_enabled = value;
  for (size_t i = 0; i < procs.size(); i++) {
    procs[i]->set_histogram(histogram_enabled);
  }
}

void sim_t::set_procs_debug(bool value)
{
  for (size_t i=0; i< procs.size(); i++)
    procs[i]->set_debug(value);
}

bool sim_t::mmio_load(reg_t addr, size_t len, uint8_t* bytes)
{
  bool result = false;
  
  if (addr + len < addr)
    return false;
  
  result = bus.load(addr, len, bytes);
  if (mem_ac_enabled && unlikely(!result && (MCU == aunit)))  
    std::cout << "mcu can't load addr:" << hex << addr \
      << " in aunit: " << aunit << std::endl;
  
  return result;
}

bool sim_t::mmio_store(reg_t addr, size_t len, const uint8_t* bytes)
{
  bool result = false;

  if (addr + len < addr)
    return false;
  
  result = bus.store(addr, len, bytes);
  if (mem_ac_enabled && unlikely(!result && (MCU == aunit)))
    std::cout << "mcu can't store addr:" << hex << addr \
      << " in aunit: " << aunit << std::endl;
  
  return result;
}

void sim_t::make_dtb()
{
  const int reset_vec_size = 8;

  start_pc = start_pc == reg_t(-1) ? get_entry_point() : start_pc;

  uint32_t reset_vec[reset_vec_size] = {
    0x297,                                      // auipc  t0,0x0
    0x28593 + (reset_vec_size * 4 << 20),       // addi   a1, t0, &dtb
    0xf1402573,                                 // csrr   a0, mhartid
    get_core(0)->get_xlen() == 32 ?
      0x0182a283u :                             // lw     t0,24(t0)
      0x0182b283u,                              // ld     t0,24(t0)
    0x28067,                                    // jr     t0
    0,
    (uint32_t) (start_pc & 0xffffffff),
    (uint32_t) (start_pc >> 32)
  };

  std::vector<char> rom((char*)reset_vec, (char*)reset_vec + sizeof(reset_vec));

  dts = make_dts(INSNS_PER_RTC_TICK, CPU_HZ, procs, mems);
  std::string dtb = dts_compile(dts);

  rom.insert(rom.end(), dtb.begin(), dtb.end());
  const int align = 0x1000;
  rom.resize((rom.size() + align - 1) / align * align);

  boot_rom.reset(new rom_device_t(rom));
  bus.add_device(DEFAULT_RSTVEC, boot_rom.get());
}

char* sim_t::addr_to_mem(reg_t addr) {
  addr = addr & 0x00000000ffffffff;
  std::ostringstream err;

  // addr on local bus (l1 | im cache)
  auto desc = local_bus[aproc_idx]->find_device(addr);
  if (auto mem = dynamic_cast<mem_t *>(desc.second)) {
    // MCU/TCP could not access im_buffer
    if (mem_ac_enabled && unlikely(desc.first == im_buffer_start && (TCP == aunit))) {
      err << "tcp access illegal address, addr=0x" << hex << addr << "(im cache)";
      throw std::runtime_error(err.str());
    }

    if (addr - desc.first < mem->size())
        return mem->contents() + (addr - desc.first);
    return NULL;
  }

  // addr on global bus (ddr)
  desc = bus.find_device(addr);
  if (auto mem = dynamic_cast<mem_t *>(desc.second)) {
    if (mem_ac_enabled && unlikely((NCP == aunit) || (TCP == aunit))) {
      auto unit = NCP == aunit? "ncp": "tcp";
      err << unit << " access illegal address, addr=0x" << hex << addr << "(ddr)";
      // throw std::runtime_error(err.str()); FIXME, maybe multi-thread has problem to check aunit
    }
    if (addr - desc.first < mem->size())
        return mem->contents() + (addr - desc.first);
    return NULL;
  } else if (auto mem = dynamic_cast<ddr_mem_t *>(desc.second)) {
    if (mem_ac_enabled && unlikely((NCP == aunit) || (TCP == aunit))) {
      auto unit = NCP == aunit? "ncp": "tcp";
      err << unit << " access illegal address, addr=0x" << hex << addr << "(ddr)";
      // throw std::runtime_error(err.str()); FIXME, maybe multi-thread has problem to check aunit
    }
    if (addr - desc.first < mem->size())
        return mem->contents() + (addr - desc.first);
    return NULL;
  }

  return NULL;
}

// htif

void sim_t::reset()
{
  if (dtb_enabled)
    make_dtb();
}

void sim_t::idle()
{
  target.switch_to();
}

void sim_t::read_chunk(addr_t taddr, size_t len, void* dst)
{
  assert(len == 8);
  auto data = debug_mmu->load_uint64(taddr);
  memcpy(dst, &data, sizeof data);
}

void sim_t::write_chunk(addr_t taddr, size_t len, const void* src)
{
  assert(len == 8);
  uint64_t data;
  memcpy(&data, src, sizeof data);
  debug_mmu->store_uint64(taddr, data);
}

void sim_t::proc_reset(unsigned id)
{
  debug_module.proc_reset(id);
}
