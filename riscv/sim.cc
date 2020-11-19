// See LICENSE for license details.

#include "sim.h"
#include "mmu.h"
#include "hwsync.h"
#include "dts.h"
#include "remote_bitbang.h"
#include <map>
#include <iostream>
#include <fstream>
#include <sstream>
#include <climits>
#include <cstdlib>
#include <cassert>
#include <regex>
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
#define SRAM_START           (0xc1000000)
#define SRAM_SIZE            (0x80000)
#define MBOX_START           (0xc07f4000)

//llb size 0x2000000 =32MB
char *shm_l1_name = "L1";
char *shm_llb_name = "LLB";

volatile bool ctrlc_pressed = false;
extern std::function<int32_t(NL_STATUS)> pcie_driver_exit;
static void handle_signal(int sig)
{
  if (ctrlc_pressed) {
    shm_unlink("HWSYNC");
    shm_unlink(shm_l1_name);
    shm_unlink(shm_llb_name);
    pcie_driver_exit(STATUS_EXIT);
    exit(-1);
  }

  ctrlc_pressed = true;
  signal(sig, &handle_signal);
}

sim_t::sim_t(const char* isa, size_t nprocs, size_t bank_id,
            char *hwsync_masks, bool halted, reg_t start_pc,
             std::vector<std::pair<reg_t, mem_t*>> mems, size_t ddr_size,
             const std::vector<std::string>& args,
             std::vector<int> const hartids, unsigned progsize,
             unsigned max_bus_master_bits, bool require_authentication,
             suseconds_t abstract_delay_usec, bool support_hasel,
             bool support_abstract_csr_access)
  : htif_t(args), mems(mems), procs(std::max(nprocs, size_t(1))), bank_id(bank_id),
    hwsync_masks(hwsync_masks),
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

  if ((bank_id == 0) && has_hwsync_masks()) {
    shm_unlink(shm_l1_name);
    shm_unlink(shm_llb_name);
  }

  hwsync = new hwsync_t(nprocs, bank_id, hwsync_masks);
  bus.add_device(0xd0080000, hwsync);

  core_reset_n = 0;
  pcie_driver = new pcie_driver_t(this, procs, bank_id);
  bus.add_device(SRAM_START, new mem_t(SRAM_SIZE));
  // bus.add_device(MBOX_START, new mbox_device_t(pcie_driver, procs));

  for (auto& x : mems) {
      bus.add_device(x.first, x.second);
      //if (x.first <= DEBUG_START && (x.first + x.second->size()) > DEBUG_START)
      //  add_debug_dev = 0;
      //if (x.first <= CLINT_BASE && (x.first + x.second->size()) > CLINT_BASE)
      //  add_clint_dev = 0;
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
      procs[i] = new processor_t(isa, this, hwsync, i, i + bank_id * procs.size(), halted);
      assert (procs[i] != nullptr);
    }
  }
  else {
    if (hartids.size() != procs.size()) {
      std::cerr << "Number of specified hartids doesn't match number of processors" << strerror(errno) << std::endl;
      exit(1);
    }
    for (size_t i = 0; i < procs.size(); i++) {
      procs[i] = new processor_t(isa, this, hwsync, i, hartids[i], halted);
      assert (procs[i] != nullptr);
    }
  }

  for (size_t i = 0; i < procs.size(); i++) {
    local_bus[i] = new bus_t();
    mbox_device_t *box = new mbox_device_t(pcie_driver, procs[i]);

    if (hwsync_masks[0] != 0) {
      l1 = new share_mem_t(l1_buffer_size * 32, shm_l1_name, (i +  bank_id * procs.size()) * l1_buffer_size);
      local_bus[i]->add_device(l1_buffer_start, l1);
    }
    else {
      local_bus[i]->add_device(l1_buffer_start, new mem_t(l1_buffer_size));
    }

    local_bus[i]->add_device(im_buffer_start, new mem_t(im_buffer_size));
    local_bus[i]->add_device(0xc07f3000, new misc_device_t(procs[i]));
    local_bus[i]->add_device(MBOX_START, box);
    procs[i]->add_mbox(box);
  }

  // a cluster has 8 cores
  int cluster_id;
  if (hartids.size() == 0)
      cluster_id = 0;
  else
     cluster_id = hartids[0]/8;

  if (bank_id) {
     cluster_id = bank_id;
  }

  switch (cluster_id) {
    case 0:
      bus.add_device(SYSDMA0_BASE, new sysdma_device_t(0, procs));
      bus.add_device(SYSDMA1_BASE, new sysdma_device_t(1, procs));
      break;
    case 1:
      bus.add_device(SYSDMA2_BASE, new sysdma_device_t(2, procs));
      bus.add_device(SYSDMA3_BASE, new sysdma_device_t(3, procs));
      break;
    case 2:
      bus.add_device(SYSDMA4_BASE, new sysdma_device_t(4, procs));
      bus.add_device(SYSDMA5_BASE, new sysdma_device_t(5, procs));
      break;
    case 3:
      bus.add_device(SYSDMA6_BASE, new sysdma_device_t(6, procs));
      bus.add_device(SYSDMA7_BASE, new sysdma_device_t(7, procs));
      break;
  default:
      throw std::runtime_error("unsupported core id");
  }

  clint.reset(new clint_t(procs));
  bus.add_device(CLINT_BASE, clint.get());

  /* the same with debug_module */
  //if (add_clint_dev)
  //  bus.add_device(CLINT_BASE, clint.get());

  if (ddr_size > 0) {
    bus.add_device(ddr_mem_start, new mem_t(ddr_size));
  }

  if (hwsync_masks[0] != 0) {
    llb = new share_mem_t(LLB_BUFFER_SIZE, shm_llb_name, 0);
    bus.add_device(LLB_AXI0_BUFFER_START, llb);
    bus.add_device(LLB_AXI1_BUFFER_START, llb);
  }
  else {
    mem_t *llb = new mem_t(LLB_BUFFER_SIZE);
    bus.add_device(LLB_AXI0_BUFFER_START, llb);
    bus.add_device(LLB_AXI1_BUFFER_START, llb);
  }
}

sim_t::~sim_t()
{
  delete hwsync;

  for (size_t i = 0; i < procs.size(); i++) {
    delete procs[i];
    delete local_bus[i];
  }
  delete debug_mmu;
  delete pcie_driver;

  if (has_hwsync_masks()) {
    delete llb;
    delete l1;
  }
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

void sim_t::dump_mems() {
  dump_mems("output_mem", exit_dump, dump_path);
}

void sim_t::dump_mems(std::string prefix, reg_t start, size_t len, int proc_id) {
  if (prefix == "") prefix = "snapshot";

  // dump single memory range
  char fname[256];
  if (start >= l1_buffer_start && start + len < SRAM_START) {
    snprintf(fname, sizeof(fname), "%s/%s@%d.0x%lx_0x%lx.dat",
          dump_path.c_str(), prefix.c_str(), proc_id, start, len);
    dump_mem(fname, start, len, proc_id, false);
  } else {
    snprintf(fname, sizeof(fname), "%s/%s@0x%lx_0x%lx.dat",
          dump_path.c_str(), prefix.c_str(), start, len);
    dump_mem(fname, start, len, -1, true);
  }
}

void sim_t::load_mem(const char *fname, reg_t addr, size_t len)
{
  memif_t mem(this);

  std::cout << "Load memory from " << fname
            << ", addr=0x" << std::hex << addr
            << ", len=0x" << std::hex << len << std::endl;
  std::string name = std::string(fname);
  std::string suffix_str = name.substr(name.find_last_of('.') + 1);

  if (suffix_str == "dat") {
    std::ifstream ifs(fname, std::ios::in);
    if (!ifs.is_open()) {
        std::cout << "Failed to open file." << std::endl;
        exit(1);
    }

    char buf[512];
    addr_t offset = 0;
    while(!ifs.eof()) {
      ifs.getline(buf, 512);
      char *p = buf;
      for (int i = 0; i < 64 && offset < len; i++, p += 5, offset += 2) {
        uint16_t data = (uint16_t)strtol(p, NULL, 16);
        mem.write(addr + offset, 2, &data);
      }
    }
    ifs.close();
  } else if (suffix_str == "bin") {
    std::ifstream ifs(fname, std::ios::in | std::ios::binary);
    if (!ifs.is_open()) {
      std::cout << "Failed to open file." << std::endl;
      exit(1);
    }

    char buf[2];
    for (addr_t offset = 0; !ifs.eof() && offset < len; offset += 2) {
      ifs.read(buf, sizeof(buf));
      mem.write(addr + offset, 2, buf);
    }
  } else {
      std::cout << "Unsupported file type " << suffix_str << std::endl;
      exit(1);
  }
}

void sim_t::load_mem(const char *fname, reg_t addr, size_t len, int proc_id)
{
  char *mem = local_addr_to_mem_by_id(addr, proc_id);

  std::cout << "Load memory from " << fname
            << ", addr=0x" << std::hex << addr
            << ", len=0x" << std::hex << len << std::endl;
  std::string name = std::string(fname);
  std::string suffix_str = name.substr(name.find_last_of('.') + 1);

  if (suffix_str == "dat") {
    std::ifstream ifs(fname, std::ios::in);
    if (!ifs.is_open()) {
        std::cout << "Failed to open file." << std::endl;
        exit(1);
    }

    char buf[512];
    int offset = 0;
    while(!ifs.eof()) {
      ifs.getline(buf, 512);
      char *p = buf;
      for (int i = 0; i < 64 && offset < len; i++, p += 5, offset += 2) {
        uint16_t data = (uint16_t)strtol(p, NULL, 16);
        memcpy(mem + offset, &data, 2);
      }
    }
    ifs.close();
  } else if (suffix_str == "bin") {
    std::ifstream ifs(fname, std::ios::in | std::ios::binary);
    if (!ifs.is_open()) {
      std::cout << "Failed to open file." << std::endl;
      exit(1);
    }

    char buf[2];
    for (addr_t offset = 0; !ifs.eof() && offset < len; offset += 2) {
      ifs.read(buf, sizeof(buf));
      memcpy(mem + offset, buf, 2);
    }
  } else {
      std::cout << "Unsupported file type " << suffix_str << std::endl;
      exit(1);
  }
}

void sim_t::dump_mem(const char *fname, reg_t addr, size_t len, int proc_id, bool space_end)
{
  char *mem = (proc_id >= 0)?
              local_addr_to_mem_by_id(addr, proc_id) :
              addr_to_mem(addr);

  std::cout << "Dump memory to " << fname
            << ", addr=0x" << std::hex << addr
            << ", len=0x" << std::hex << len << std::endl;
  std::string name = std::string(fname);
  std::string suffix_str = name.substr(name.find_last_of('.') + 1);

  if (suffix_str != "dat" && suffix_str != "bin") {
    std::cout << __FUNCTION__ << ": Unsupported file type " << suffix_str << std::endl;
    exit(1);
  }

  if (suffix_str == "dat") {
    std::ofstream ofs(fname, std::ios::out);
    if (!ofs.is_open()) {
        std::cout << "Failed to open file." << std::endl;
        exit(1);
    }

    uint16_t data;
    char buf[5];
    for (addr_t offset = 0; offset < len; offset += 2) {
      data = *((uint16_t *)(mem + offset));
      sprintf(buf, "%04x", data);
      ofs << buf;
      if ((offset + 2) % 128) {
        ofs << " ";
      } else {
        if (space_end) ofs << " ";
        ofs << endl;
      }
    }

    ofs.close();
  } else if (suffix_str == "bin") {
    std::ofstream ofs(fname, std::ios::out | std::ios::binary);
    if (!ofs.is_open()) {
        std::cout << "Failed to open file." << std::endl;
        exit(1);
    }

    for (addr_t offset = 0; offset < len; offset += 2) {
      ofs.write(mem + offset, 2);
    }
  }
}

void sim_t::load_mems(std::vector<std::string> load_files) {
  for (const std::string& fname : load_files) {
    // l1 file name format: <any>@<core_id|llb|ddr>.<ext>
    const std::regex re0(".*@([0-9]+|ddr|llb)\\.([a-z]+)");
    // llb, ddr file name format: <any>@<start>.<ext>
    const std::regex re1(".*@(0x[0-9a-fA-F]+)\\.([a-z]+)");
    // llb, ddr file name format: <any>@<start>_<len>.<ext>
    const std::regex re2(".*@(0x[0-9a-fA-F]+)_(0x[0-9a-fA-F]+)\\.([a-z]+)");
    // l1 file name format: <any>@<core_id|llb|ddr>.<start>.<ext>
    const std::regex re3(".*@([0-9]+|ddr|llb)\\.(0x[0-9a-fA-F]+)\\.([a-z]+)");
    // l1 file name format: <any>@<core_id|llb|ddr>.<start>_<len>.<ext>
    const std::regex re4(".*@([0-9]+|ddr|llb)\\.(0x[0-9a-fA-F]+)_(0x[0-9a-fA-F]+)\\.([a-z]+)");
    std::smatch match;

    if (std::regex_match(fname, match, re0)) {
      if (match[1] == "ddr") {
        reg_t start = 0;
        size_t len = 0xc0000000;
        load_mem(fname.c_str(), start, len);
      } else if (match[1] == "llb") {
        reg_t start = LLB_AXI0_BUFFER_START + bank_id * LLB_BANK_BUFFER_SIZE;
        size_t len = LLB_BANK_BUFFER_SIZE;
        load_mem(fname.c_str(), start, len);
      } else {
        auto proc_id = std::stoul(match[1], nullptr, 16);
        reg_t start = l1_buffer_start;
        size_t len = l1_buffer_size;
        load_mem(fname.c_str(), start, len, proc_id);
      }
    } else if (std::regex_match(fname, match, re1)) {
      auto start = std::stoul(match[1], nullptr, 16);
      load_mem(fname.c_str(), start, -1);
    } else if (std::regex_match(fname, match, re2)) {
      auto start = std::stoul(match[1], nullptr, 16);
      auto len = std::stoul(match[2], nullptr, 16);
      load_mem(fname.c_str(), start, len);
    } else if (std::regex_match(fname, match, re3)) {
      auto start = std::stoul(match[2], nullptr, 16);
      if (match[1] == "ddr" || match[1] == "llb") {
        load_mem(fname.c_str(), start, -1);
      } else {
        auto proc_id = std::stoul(match[1], nullptr, 16);
        load_mem(fname.c_str(), start, -1, proc_id);
      }

    } else if (std::regex_match(fname, match, re4)) {
      auto start = std::stoul(match[2], nullptr, 16);
      auto len = std::stoul(match[3], nullptr, 16);
      if (match[1] == "ddr" || match[1] == "llb") {
        load_mem(fname.c_str(), start, len);
      } else {
        auto proc_id = std::stoul(match[1], nullptr, 16);
        load_mem(fname.c_str(), start, len, proc_id);
      }

    } else {
      std::cout << "Invalid load file " << fname << std::endl;
      exit(1);
    }
  }
}

void sim_t::dump_mems(std::string prefix, std::vector<std::string> mems, std::string path) {
  char fname[256];

  for (const std::string& mem: mems) {
    if (mem == "l1") {
      // dump whole l1 buffer for all procs
      for (auto i=0u; i< nprocs(); i++) {
        snprintf(fname, sizeof(fname), "%s/%s@%d.dat", path.c_str(), prefix.c_str(), procs[i]->get_id());
        dump_mem(fname, l1_buffer_start, l1_buffer_size, procs[i]->get_id(), true);
      }
    } else if (mem == "llb") {
      // dump whole llb
      snprintf(fname, sizeof(fname), "%s/%s@llb.dat", path.c_str(), prefix.c_str());
      dump_mem(fname, LLB_AXI0_BUFFER_START + bank_id * LLB_BANK_BUFFER_SIZE, LLB_BANK_BUFFER_SIZE, -1);
    } else {
      // dump memory range, format: <start>:<len>
      const std::regex re("(0x[0-9a-fA-F]+):(0x[0-9a-fA-F]+)");
      std::smatch match;
      if (!std::regex_match(mem, match, re)) {
        std::cout << "Invalid dump format " << mem << std::endl;
        exit(1);
      }
      auto start = std::stoul(match[1], nullptr, 16);
      auto len = std::stoul(match[2], nullptr, 16);
      if (start >= l1_buffer_start && start + len < SRAM_START) {
        // dump l1 address range
        for (auto i=0u; i< nprocs(); i++) {
          snprintf(fname, sizeof(fname),
            "%s/%s@%d.0x%lx_0x%lx.dat",
            path.c_str(), prefix.c_str(), procs[i]->get_id(), start, len);
          dump_mem(fname, start, len, procs[i]->get_id(), true);
        }
      } else {
        // dump llb or ddr range
        snprintf(fname, sizeof(fname), "%s/%s@ddr.0x%lx_0x%lx.dat", path.c_str(), prefix.c_str(), start, len);
        dump_mem(fname, start, len, -1, true);
      }
    }
  }
}

int sim_t::run(std::vector<std::string> load_files,
               std::vector<std::string> init_dump,
               std::vector<std::string> exit_dump_,
               std::string dump_path_)
{
  exit_dump = exit_dump_;
  dump_path = dump_path_;

  int stat;
  host = context_t::current();
  target.init(sim_thread_main, this);
  load_mems(load_files);

  if (init_dump.size() > 0)
    dump_mems("input_mem", init_dump, dump_path);

  stat = htif_t::run();

  if (exit_dump.size() > 0)
    dump_mems("output_mem", exit_dump, dump_path);
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

  return result;
}

bool sim_t::mmio_store(reg_t addr, size_t len, const uint8_t* bytes)
{
  bool result = false;

  if (addr + len < addr)
    return false;

  result = bus.store(addr, len, bytes);

  return result;
}

bool sim_t::local_mmio_load(reg_t addr, size_t len, uint8_t* bytes, uint32_t idx)
{
  bool result = false;

  if (addr + len < addr)
    return false;

  result = local_bus[idx]->load(addr, len, bytes);

  return result;
}

bool sim_t::local_mmio_store(reg_t addr, size_t len, const uint8_t* bytes, uint32_t idx)
{
  bool result = false;

  if (addr + len < addr)
    return false;

  result = local_bus[idx]->store(addr, len, bytes);

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

bool sim_t::in_local_mem(reg_t addr, local_device_type type) {
  auto desc = local_bus[0]->find_device(addr);

  if (type == IO_DEVICE) {
    if (auto mem = dynamic_cast<misc_device_t *>(desc.second)) {
      if (addr - desc.first <= mem->size()) {
        return true;
      }
    }

    if (auto mem = dynamic_cast<mbox_device_t *>(desc.second)) {
      if (addr - desc.first <= mem->size()) {
        return true;
      }
    }

    return false;
  }

  if ((type == L1_BUFFER) && has_hwsync_masks()) {
    if (auto mem = dynamic_cast<share_mem_t *>(desc.second)) {
      reg_t start_addr = l1_buffer_start;
      if (desc.first == start_addr && addr - desc.first <= l1_buffer_size) {
        return true;
      }
    }
  } else {
    if (auto mem = dynamic_cast<mem_t *>(desc.second)) {
      reg_t start_addr = (type == L1_BUFFER)? l1_buffer_start: im_buffer_start;
      if (desc.first == start_addr && addr - desc.first <= mem->size()) {
        return true;
      }
    }
  }

  return false;
}

char* sim_t::addr_to_mem(reg_t addr) {
  std::ostringstream err;

  auto desc = bus.find_device(addr);
  if ((in_local_mem(addr, L1_BUFFER) ||
   (addr >= LLB_AXI0_BUFFER_START) && (addr < LLB_AXI0_BUFFER_START + LLB_BUFFER_SIZE) ||
   (addr >= LLB_AXI1_BUFFER_START) && (addr < LLB_AXI1_BUFFER_START + LLB_BUFFER_SIZE)) &&
    has_hwsync_masks()) {
    if (auto mem = dynamic_cast<share_mem_t *>(desc.second)) {
      if (addr - desc.first < mem->size())
          return mem->contents() + (addr - desc.first);
      return NULL;
    }
  } else {
    if (auto mem = dynamic_cast<mem_t *>(desc.second)) {
      if (addr - desc.first < mem->size())
          return mem->contents() + (addr - desc.first);
      return NULL;
    }
  }

  return NULL;
}

char* sim_t::local_addr_to_mem(reg_t addr, uint32_t idx) {
  std::ostringstream err;

  // addr on local bus (l1 | im cache)
  auto desc = local_bus[idx]->find_device(addr);
  if ((in_local_mem(addr, L1_BUFFER) ||
    (addr >= LLB_AXI0_BUFFER_START) && (addr < LLB_AXI0_BUFFER_START + LLB_BUFFER_SIZE) ||
    (addr >= LLB_AXI1_BUFFER_START) && (addr < LLB_AXI1_BUFFER_START + LLB_BUFFER_SIZE)) &&
    has_hwsync_masks()) {
    if (auto mem = dynamic_cast<share_mem_t *>(desc.second)) {
      if (addr - desc.first < mem->size()) {
          return mem->contents() + (addr - desc.first);
      }
      return NULL;
    }
  } else {
    if (auto mem = dynamic_cast<mem_t *>(desc.second)) {
      if (addr - desc.first < mem->size())
          return mem->contents() + (addr - desc.first);
      return NULL;
    }
  }
  return NULL;
}

char* sim_t::local_addr_to_mem_by_id(reg_t addr, uint32_t id) {
  for (unsigned i = 0; i < nprocs(); i++) {
    processor_t *proc = get_core(i);
    if (proc->get_id() == id) {
      return local_addr_to_mem(addr, proc->get_idx());
    }
  }

  return NULL;
}

char* sim_t::local_addr_to_mem_by_id_cluster(reg_t addr, uint32_t id) {
  char *mem_ptr = NULL;
  int32_t mem_offset = 0;

  if (has_hwsync_masks()) {
    //get current bank first core id and addr
    processor_t *proc = get_core(0);
    mem_ptr = local_addr_to_mem(addr, 0);

    mem_offset = (id - proc->get_id()) * l1_buffer_size;
    mem_ptr = mem_ptr + mem_offset;
    return mem_ptr;
  }
  else
    return local_addr_to_mem_by_id(addr, id);
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
