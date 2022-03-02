// See LICENSE for license details.

#include "sim.h"
#include "mmu.h"
#include "hwsync.h"
#include "dts.h"
#include "remote_bitbang.h"
#include "byteorder.h"
#include <fstream>
#include <map>
#include <iostream>
#include <sstream>
#include <climits>
#include <cstdlib>
#include <cassert>
#include <regex>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

//L1 buffer size adjust to 1.288M, 0xc0000000-0xc0148000
//im buffer size adjust to 256K, 0xc0400000-0xc0440000
//Index RAM size 80k, 0xc0500000-0xc0514000


#define SRAM_START           (0xD3D80000)   /* NPUV2 IRAM 512K */
#define SRAM_SIZE            (0x80000)

//coremask configure base address
#define STC_VALID_NPCS_BASE  (SRAM_START + 0x4400)

volatile bool ctrlc_pressed = false;

static void handle_signal(int sig)
{
  if (ctrlc_pressed) {
    exit(-1);
  }

  ctrlc_pressed = true;
  signal(sig, &handle_signal);
}

sim_t::sim_t(const char* isa, const char* priv, const char* varch,
             size_t nprocs, int _nbanks, size_t id_first_bank, size_t die_id, char *hwsync_masks, uint32_t hwsync_timer_num,
             bool halted, bool real_time_clint,
             reg_t initrd_start, reg_t initrd_end, const char* bootargs,
             reg_t start_pc, std::vector<std::pair<reg_t, mem_t*>> mems,size_t ddr_size,
             std::vector<std::pair<reg_t, abstract_device_t*>> plugin_devices,
             const std::vector<std::string>& args,
             std::vector<int> const hartids,
             const debug_module_config_t &dm_config,
             const char *log_path,
             bool dtb_enabled, const char *dtb_file, bool pcie_enabled, size_t board_id, 
             size_t chip_id, size_t session_id, uint32_t coremask, const char *ipaini)
  : htif_t(args, this),
    mems(mems),
    plugin_devices(plugin_devices),
    /* procs(std::max(nprocs, size_t(1))),  */
    banks(std::max(size_t(_nbanks+id_first_bank), size_t(1))),
    core_num_of_bank(nprocs/_nbanks),
    id_first_bank(id_first_bank),
    sim_nprocs(nprocs),
    sim_nbanks(_nbanks),
    die_id(die_id),
    hwsync_masks(hwsync_masks),
    initrd_start(initrd_start),
    initrd_end(initrd_end),
    bootargs(bootargs),
    start_pc(start_pc),
    dtb_file(dtb_file ? dtb_file : ""),
    dtb_enabled(dtb_enabled),
    log_file(log_path),
    current_step(0),
    current_proc(0),
    debug(false),
    histogram_enabled(false),
    log(false),
    pcie_enabled(pcie_enabled),
    remote_bitbang(NULL),
    debug_module(this, dm_config)
{
    core_reset_n = 0;
    signal(SIGINT, &handle_signal);

    if (! (hartids.empty() || hartids.size() == nprocs)) {
        std::cerr << "Number of specified hartids ("
                << hartids.size()
                << ") doesn't match number of processors ("
                << nprocs << ").\n";
        exit(1);
    }

    /* 创建并添加 hs */
    if ('\0' == hwsync_masks[0]) {
        int i = 0;
        uint32_t masks = 0xffffffff;
        if (!hartids.empty()) {
            for (i = 0 ; i < (int)(hartids.size()) ; i++) {
                masks &= ~(1 << hartids[i]);
            }
        } else {
            for (i = 0 ; i < (int)(this->nprocs()) ; i++) {
                masks &= ~(1 << (i + get_id_first_bank() * core_num_of_bank));
            }
        }
        sprintf(hwsync_masks,"0x%x",masks);
    }
    hwsync = new hwsync_t(hwsync_masks, hwsync_timer_num);
    glb_bus.add_device(HWSYNC_START, hwsync);

    glb_bus.add_device(SRAM_START, new mem_t(SRAM_SIZE));

    /* 添加 llb */
    mem_t *llb = new mem_t(LLB_BANK_BUFFER_SIZE*4);
    glb_bus.add_device(LLB_AXI0_BUFFER_START, llb);
    glb_bus.add_device(LLB_AXI1_BUFFER_START, llb);

    /* 创建bank */
    if ((1<sim_nbanks) && (0!=(sim_nprocs%sim_nbanks))) {
        std::cerr << "Number of nprocs ("
                << nprocs
                << ") must be a multiple of nbanks("
                << sim_nbanks << ").\n";
        exit(1);
    }
    for (size_t i = get_id_first_bank() ; i < nbanks()+get_id_first_bank() ; i++) {
        banks[i] = new bank_t(isa, priv, varch, this, ddr_size, hwsync,
                log_file.get(), pcie_enabled, board_id, chip_id,core_num_of_bank, i,
                hartids, halted, ipaini);
    }

    debug_module.add_device(&glb_bus);
    debug_mmu = new mmu_t(this, get_bank(get_id_first_bank()), NULL, nullptr);

    for (auto& x : mems)
        glb_bus.add_device(x.first, x.second);

    for (auto& x : plugin_devices)
        glb_bus.add_device(x.first, x.second);

    //write cluster-ID'cormask to 0xC1004400 + ID*4
    for (size_t i = get_id_first_bank() ; i < nbanks()+get_id_first_bank(); i++) {
        char *mem = addr_to_mem(STC_VALID_NPCS_BASE + i * 4);
        *(uint32_t *)mem = (coremask & (0xff << i * 8)) >> i * 8;
    }

    make_dtb();

  void *fdt = (void *)dtb.c_str();
  //handle clic
  clint.reset(new clint_t(this, /* procs */ CPU_HZ / INSNS_PER_RTC_TICK, real_time_clint));
  reg_t clint_base;
  if (fdt_parse_clint(fdt, &clint_base, "riscv,clint0")) {
    glb_bus.add_device(CLINT_BASE, clint.get());
  } else {
    glb_bus.add_device(clint_base, clint.get());
  }
  
  //per core attribute
  int cpu_offset = 0, rc;
  size_t cpu_idx = 0;
  cpu_offset = fdt_get_offset(fdt, "/cpus");
  if (cpu_offset < 0)
    return;

  for (cpu_offset = fdt_get_first_subnode(fdt, cpu_offset); cpu_offset >= 0;
       cpu_offset = fdt_get_next_subnode(fdt, cpu_offset)) {

    if (cpu_idx >= nprocs)
      break;

    //handle pmp
    reg_t pmp_num = 0, pmp_granularity = 0;
    if (fdt_parse_pmp_num(fdt, cpu_offset, &pmp_num) == 0) {
      get_core_by_idxinsim(cpu_idx)->set_pmp_num(pmp_num);
    }

    if (fdt_parse_pmp_alignment(fdt, cpu_offset, &pmp_granularity) == 0) {
      get_core_by_idxinsim(cpu_idx)->set_pmp_granularity(pmp_granularity);
    }

    //handle mmu-type
    char mmu_type[256] = "";
    rc = fdt_parse_mmu_type(fdt, cpu_offset, mmu_type);
    if (rc == 0) {
      get_core_by_idxinsim(cpu_idx)->set_mmu_capability(IMPL_MMU_SBARE);
      if (strncmp(mmu_type, "riscv,sv32", strlen("riscv,sv32")) == 0) {
        get_core_by_idxinsim(cpu_idx)->set_mmu_capability(IMPL_MMU_SV32);
      } else if (strncmp(mmu_type, "riscv,sv39", strlen("riscv,sv39")) == 0) {
        get_core_by_idxinsim(cpu_idx)->set_mmu_capability(IMPL_MMU_SV39);
      } else if (strncmp(mmu_type, "riscv,sv48", strlen("riscv,sv48")) == 0) {
        get_core_by_idxinsim(cpu_idx)->set_mmu_capability(IMPL_MMU_SV48);
      } else if (strncmp(mmu_type, "riscv,sbare", strlen("riscv,sbare")) == 0) {
        //has been set in the beginning
      } else {
        std::cerr << "core ("
                  << hartids.size()
                  << ") doesn't have valid 'mmu-type'"
                  << mmu_type << ").\n";
        exit(1);
      }
    }

    cpu_idx++;
  }

  if (cpu_idx != nprocs) {
      std::cerr << "core number in dts ("
                <<  cpu_idx
                << ") doesn't match it in command line ("
                << nprocs << ").\n";
      exit(1);
  }
}

sim_t::~sim_t()
{
    delete hwsync;
    for (int i = get_id_first_bank() ; i < (int)nbanks()+get_id_first_bank() ; i++) {
        delete banks[i];
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

void sim_t::dump_mems() {
  dump_mems("output_mem", exit_dump, dump_path);
}

void sim_t::dump_mems(std::string prefix, reg_t start, size_t len, int proc_id) 
{
    char fname[256];
    if (prefix == "") prefix = "snapshot";

    // dump single memory range
    if (is_upper_mem(start)) {        /* 高端内存 */
        snprintf(fname, sizeof(fname), "%s/%s@ddr.0x%lx_0x%lx.dat", dump_path.c_str(), prefix.c_str(),start, len);
        dump_mem(fname, start, len, proc_id, true);
    } else if (npc_addr_to_mem(start,get_id_first_bank(),0)) {        /* npc_bus */
        snprintf(fname, sizeof(fname),"%s/%s@%d.0x%lx_0x%lx.dat",
            dump_path.c_str(), prefix.c_str(), proc_id, start, len);
        dump_mem(fname, start, len, proc_id, false);
    } else if (bank_addr_to_mem(start,get_id_first_bank())) {         /* bank_bus */
        snprintf(fname, sizeof(fname), "%s/%s@ddr.0x%lx_0x%lx.dat", dump_path.c_str(), prefix.c_str(), start, len);
        dump_mem(fname, start, len, proc_id, true);
    } else {      /* glb_bus */
        snprintf(fname, sizeof(fname), "%s/%s@ddr.0x%lx_0x%lx.dat", dump_path.c_str(), prefix.c_str(), start, len);
        dump_mem(fname, start, len, -1, true);
    }
}

void sim_t::dump_mems(std::string prefix, std::vector<std::string> mems, std::string path) {
  char fname[256];

  for (const std::string& mem: mems) {
    if (mem == "l1") {
      // dump whole l1 buffer for all procs
      for (int i=0; i < (int)nprocs(); i++) {
        snprintf(fname, sizeof(fname), "%s/%s@%d.dat", path.c_str(), prefix.c_str(), get_core_by_idxinsim(i)->get_id());
        dump_mem(fname, l1_buffer_start, l1_buffer_size, get_core_by_idxinsim(i)->get_id(), true);
      }
    } else if (mem == "llb") {
      // dump whole llb, 4*10=40M
      for (int i = 0 ; i < 4 ; i++) {
        snprintf(fname, sizeof(fname), "%s/%s_b%d@llb.dat", path.c_str(), prefix.c_str(),i);
        dump_mem(fname, LLB_AXI0_BUFFER_START+i*LLB_BANK_BUFFER_SIZE, LLB_BANK_BUFFER_SIZE, -1);
      }
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

      if (is_upper_mem(start)) {        /* 高端内存 */
        snprintf(fname, sizeof(fname), "%s/%s@ddr.0x%lx_0x%lx.dat", path.c_str(), prefix.c_str(),start, len);
        dump_mem(fname, start, len, get_bank(get_bankid_by_uppermem(start))->get_core_by_idxinbank(0)->get_id(), true);
      } else if (npc_addr_to_mem(start,get_id_first_bank(),0)) {         /* npc_bus */
        for (int i=0; i < (int)nprocs(); i++) {
            snprintf(fname, sizeof(fname),"%s/%s@%d.0x%lx_0x%lx.dat",
                path.c_str(), prefix.c_str(), get_core_by_idxinsim(i)->get_id(), start, len);
            dump_mem(fname, start, len, get_core_by_idxinsim(i)->get_id(), true);
        }
      } else if (bank_addr_to_mem(start,get_id_first_bank())) {         /* bank_bus */
        for (int i = get_id_first_bank() ; i < (int)(get_id_first_bank()+nbanks()) ; i++) {
            snprintf(fname, sizeof(fname), "%s/%s_b%d@ddr.0x%lx_0x%lx.dat", path.c_str(), prefix.c_str(), i, start, len);
            dump_mem(fname, start, len, get_bank(i)->get_core_by_idxinbank(0)->get_id(), true);
        }
      } else {      /* glb_bus */
        snprintf(fname, sizeof(fname), "%s/%s@ddr.0x%lx_0x%lx.dat", path.c_str(), prefix.c_str(), start, len);
        dump_mem(fname, start, len, -1, true);
      }
    }
  }
}

void sim_t::dump_mem(const char *fname, reg_t addr, size_t len, int proc_id, bool space_end)
{
    char *mem = nullptr;
    int idxinsim = coreid_to_idxinsim(proc_id);
    int bankid = get_bankid(idxinsim);
    int idxinbank = get_idxinbank(idxinsim);

    if ((0>idxinsim) || (!((mem=npc_addr_to_mem(addr,bankid,idxinbank)) || (mem=bank_addr_to_mem(addr,bankid))))) {
        mem = addr_to_mem(addr);
    }
    if (!mem) {
        std::cerr << "Dump addr 0x" << std::hex << addr << "error." << std::endl;
        return ;
    }

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
        ofs << std::endl;
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

    // output_mem_b2@llb.bin
    if (std::regex_match(fname, match, re0)) {
      if (match[1] == "ddr") {
        reg_t start = 0;
        size_t len = 0xc0000000;
        load_mem(fname.c_str(), start, len);
      } else if (match[1] == "llb") {
        std::string bankstr = {};
        int pos = fname.find('@');
        if (0 >= pos) {
             std::cerr << "File name format error." << std::endl;
        }
        bankstr = fname.substr(0,pos);
        pos = bankstr.rfind("_b");
        if (0 >= pos) {
             std::cerr << "File name format error, not found \"_bx\"" << std::endl;
        }
        bankstr = bankstr.substr(pos+2);
        auto bankid = std::stoul(bankstr, nullptr, 16);

        reg_t start = LLB_AXI0_BUFFER_START+bankid*LLB_BANK_BUFFER_SIZE;
        size_t len = LLB_BANK_BUFFER_SIZE;
        load_mem(fname.c_str(), start, len, -1);
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
          if (is_upper_mem(start)) {
            load_mem(fname.c_str(), start, len, -1);
          } else {
            std::string bankstr = {};
            int pos = fname.find('@');
            if (0 >= pos) {
                    std::cerr << "File name format error." << std::endl;
            }
            bankstr = fname.substr(0,pos);
            pos = bankstr.rfind("_b");
            if (0 >= pos) {
                    std::cerr << "File name format error, not found \"_bx\"" << std::endl;
            }
            bankstr = bankstr.substr(pos+2);
            auto bankid = std::stoul(bankstr, nullptr, 16);

            if (match[1] == "llb") {
                load_mem(fname.c_str(), start, len, -1);
            } else {
                load_mem(fname.c_str(), start, len, get_bank(bankid)->get_core_by_idxinbank(0)->get_id());
            }
          }
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
    char *mem = nullptr;
    int idxinsim = coreid_to_idxinsim(proc_id);
    int bankid = get_bankid(idxinsim);
    int idxinbank = get_idxinbank(idxinsim);

    if ((0>idxinsim) || (!((mem=npc_addr_to_mem(addr,bankid,idxinbank)) || (mem=bank_addr_to_mem(addr,bankid))))) {
        mem = addr_to_mem(addr);
    }
    if (!mem) {
        std::cerr << "Load addr 0x" << std::hex << addr << "error." << std::endl;
        return ;
    }

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
      for (int i = 0; i < 64 && (size_t)offset < len; i++, p += 5, offset += 2) {
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

int sim_t::run(std::vector<std::string> load_files,
               std::vector<std::string> init_dump,
               std::vector<std::string> exit_dump_,
               std::string dump_path_)
{
  int stat;

  exit_dump = exit_dump_;
  dump_path = dump_path_;

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
    get_core_by_idxinsim(current_proc)->step(steps);     /* procs[current_proc] */

    current_step += steps;
    if (current_step == INTERLEAVE)
    {
      current_step = 0;
      get_core_by_idxinsim(current_proc)->get_mmu()->yield_load_reservation();
      if (++current_proc == nprocs()) {
        current_proc = 0;
        clint->increment(INTERLEAVE / INSNS_PER_RTC_TICK);
      }

      host->switch_to();
    }
  }

  // if all procs in wfi, sleep to avoid 100% cpu usage.
  if (n > 1) {
    int wfi_count = 0;
    for (size_t p = 0; p < nprocs(); p++) {
      auto state = get_core_by_idxinsim(p)->get_state();
      if (state->wfi_flag && !state->async_started)
        wfi_count++;
    }

    if (wfi_count == (int)nprocs())
      usleep(100000);
  }
}

void sim_t::set_debug(bool value)
{
  debug = value;
}

void sim_t::set_histogram(bool value)
{
    histogram_enabled = value;
    for (size_t i = 0; i < nprocs(); i++) {
        get_core_by_idxinsim((int)i)->set_histogram(histogram_enabled);
    }
}

void sim_t::configure_log(bool enable_log, bool enable_commitlog)
{
  log = enable_log;

  if (!enable_commitlog)
    return;

#ifndef RISCV_ENABLE_COMMITLOG
  fputs("Commit logging support has not been properly enabled; "
        "please re-build the riscv-isa-sim project using "
        "\"configure --enable-commitlog\".\n",
        stderr);
  abort();
#else

    for (int i = 0 ; i < nprocs() ; i++) {
        get_core_by_idxinsim(i)->enable_log_commits();
    }

#endif
}

void sim_t::set_procs_debug(bool value)
{
    for (size_t i=0; i< nprocs(); i++)
        get_core_by_idxinsim(i)->set_debug(value);
}

static bool paddr_ok(reg_t addr)
{
  return (addr >> MAX_PADDR_BITS) == 0;
}

bool sim_t::mmio_load(reg_t addr, size_t len, uint8_t* bytes)
{
    if (addr + len < addr || !paddr_ok(addr + len - 1))
        return false;
    return glb_bus.load(addr & 0xffffffff, len, bytes);
}

bool sim_t::mmio_store(reg_t addr, size_t len, const uint8_t* bytes)
{
    if (addr + len < addr || !paddr_ok(addr + len - 1))
        return false;
    return glb_bus.store(addr & 0xffffffff, len, bytes);
}

bool sim_t::bank_mmio_load(reg_t addr, size_t len, uint8_t* bytes, uint32_t bank_id)
{
    return get_bank(bank_id)->bank_mmio_load(addr, len, bytes);
}

bool sim_t::bank_mmio_store(reg_t addr, size_t len, const uint8_t* bytes, uint32_t bank_id)
{
    return get_bank(bank_id)->bank_mmio_store(addr, len, bytes);
}

bool sim_t::npc_mmio_load(reg_t addr, size_t len, uint8_t* bytes,
            uint32_t bank_id, uint32_t idxinbank)
{
    return get_bank(bank_id)->npc_mmio_load(addr, len, bytes, idxinbank);
}

bool sim_t::npc_mmio_store(reg_t addr, size_t len, const uint8_t* bytes,
            uint32_t bank_id, uint32_t idxinbank)
{
    return get_bank(bank_id)->npc_mmio_store(addr, len, bytes, idxinbank);
}

void sim_t::make_dtb()
{
  if (!dtb_file.empty()) {
    std::ifstream fin(dtb_file.c_str(), std::ios::binary);
    if (!fin.good()) {
      std::cerr << "can't find dtb file: " << dtb_file << std::endl;
      exit(-1);
    }

    std::stringstream strstream;
    strstream << fin.rdbuf();

    dtb = strstream.str();
  } else {
    dts = make_dts(INSNS_PER_RTC_TICK, CPU_HZ, initrd_start, initrd_end, bootargs, this, mems);
    dtb = dts_compile(dts);
  }
}

void sim_t::set_rom()
{
  const int reset_vec_size = 8;

  start_pc = start_pc == reg_t(-1) ? get_entry_point() : start_pc;

  uint32_t reset_vec[reset_vec_size] = {
    0x297,                                      // auipc  t0,0x0
    0x28593 + (reset_vec_size * 4 << 20),       // addi   a1, t0, &dtb
    0xf1402573,                                 // csrr   a0, mhartid
    get_core_by_idxinsim(0)->get_xlen() == 32 ?
      0x0182a283u :                             // lw     t0,24(t0)
      0x0182b283u,                              // ld     t0,24(t0)
    0x28067,                                    // jr     t0
    0,
    (uint32_t) (start_pc & 0xffffffff),
    (uint32_t) (start_pc >> 32)
  };
  if (get_target_endianness() == memif_endianness_big) {
    int i;
    // Instuctions are little endian
    for (i = 0; reset_vec[i] != 0; i++)
      reset_vec[i] = to_le(reset_vec[i]);
    // Data is big endian
    for (; i < reset_vec_size; i++)
      reset_vec[i] = to_be(reset_vec[i]);

    // Correct the high/low order of 64-bit start PC
    if (get_core_by_idxinsim(0)->get_xlen() != 32)
      std::swap(reset_vec[reset_vec_size-2], reset_vec[reset_vec_size-1]);
  } else {
    for (int i = 0; i < reset_vec_size; i++)
      reset_vec[i] = to_le(reset_vec[i]);
  }

  std::vector<char> rom((char*)reset_vec, (char*)reset_vec + sizeof(reset_vec));

  std::string dtb;
  if (!dtb_file.empty()) {
    std::ifstream fin(dtb_file.c_str(), std::ios::binary);
    if (!fin.good()) {
      std::cerr << "can't find dtb file: " << dtb_file << std::endl;
      exit(-1);
    }

    std::stringstream strstream;
    strstream << fin.rdbuf();

    dtb = strstream.str();
  } else {
    dts = make_dts(INSNS_PER_RTC_TICK, CPU_HZ, initrd_start, initrd_end, bootargs, this, mems);
    dtb = dts_compile(dts);
  }

  rom.insert(rom.end(), dtb.begin(), dtb.end());
  const int align = 0x1000;
  rom.resize((rom.size() + align - 1) / align * align);

  boot_rom.reset(new rom_device_t(rom));
  glb_bus.add_device(DEFAULT_RSTVEC, boot_rom.get());
}

/* 访问sim内的 mem_t 类型资源, 如果是40bit地址额外找bank_bus */
char* sim_t::addr_to_mem(reg_t addr)
{
    int bankid = 0;

    if (!paddr_ok(addr))
        return NULL;

    if (is_upper_mem(addr)) {        /* 高端内存，访问upper region ddr建议使用bank级接口 */
        bankid = get_bankid_by_uppermem(addr);
        if (0 <= bankid) {
            return bank_addr_to_mem(addr, bankid);
        }
    } else {
        auto desc = glb_bus.find_device(addr);
        if (auto mem = dynamic_cast<mem_t *>(desc.second)) {
            if (addr - desc.first < mem->size())
                return mem->contents() + (addr - desc.first);
            return NULL;
        }
    }

    return NULL;
}

char* sim_t::bank_addr_to_mem(reg_t addr, uint32_t bank_id) {
    return get_bank(bank_id)->bank_addr_to_mem(addr);
}

char* sim_t::npc_addr_to_mem(reg_t addr, uint32_t bank_id, uint32_t idxinbank) {
    return get_bank(bank_id)->npc_addr_to_mem(addr, idxinbank);
}

const char* sim_t::get_symbol(uint64_t addr)
{
  return htif_t::get_symbol(addr);
}

bool sim_t::is_upper_mem(reg_t addr)
{
    for (int i = get_id_first_bank() ; i < (int)nbanks()+get_id_first_bank() ; i++) {
        if ((0 == (int)die_id) && (addr >= (reg_t)(GLB_DIE0_UPPER_REGION_BANK0_START_ADDR + i*GLB_UPPER_REGION_SIZE))
            && (addr < (reg_t)(GLB_DIE0_UPPER_REGION_BANK0_START_ADDR + (i+1)*GLB_UPPER_REGION_SIZE))) {
            return true;
        } else if ((1 == (int)die_id) && (addr >= (reg_t)(GLB_DIE1_UPPER_REGION_BANK0_START_ADDR + i*GLB_UPPER_REGION_SIZE))
            && (addr < (reg_t)(GLB_DIE1_UPPER_REGION_BANK0_START_ADDR + (i+1)*GLB_UPPER_REGION_SIZE))) {
            return true;
        }
    }
    return false;
}

bool sim_t::is_bottom_ddr(reg_t addr) const
{
    if (addr < GLB_BOTTOM_REGION_SIZE)
        return true;
    else 
        return false;
}

reg_t sim_t::bottom_ddr_to_upper(reg_t addr, int bankid) const
{
    if (!is_bottom_ddr(addr))
        return 0;

    return  GLB_DIE0_UPPER_REGION_BANK0_START_ADDR+(GLB_UPPER_REGION_SIZE*bankid) + addr;
}

int sim_t::get_bankid_by_uppermem(reg_t addr)
{
    int i = 0;

    for (int i = get_id_first_bank() ; i < (int)nbanks()+get_id_first_bank() ; i++) {
        if ((addr >= (reg_t)(GLB_DIE0_UPPER_REGION_BANK0_START_ADDR + i*GLB_UPPER_REGION_SIZE))
            && (addr < (reg_t)(GLB_DIE0_UPPER_REGION_BANK0_START_ADDR + (i+1)*GLB_UPPER_REGION_SIZE))) {
            return i;
        } else if ((addr >= (reg_t)(GLB_DIE1_UPPER_REGION_BANK0_START_ADDR + i*GLB_UPPER_REGION_SIZE))
            && (addr < (reg_t)(GLB_DIE1_UPPER_REGION_BANK0_START_ADDR + (i+1)*GLB_UPPER_REGION_SIZE))) {
            return i;
        }
    }
    return -1;
}

void sim_t::reset()
{
  if (dtb_enabled)
    set_rom();
}

void sim_t::idle()
{
  target.switch_to();
}

void sim_t::read_chunk(addr_t taddr, size_t len, void* dst)
{
  assert(len == 8);
  auto data = debug_mmu->to_target(debug_mmu->load_uint64(taddr));
  memcpy(dst, &data, sizeof data);
}

void sim_t::write_chunk(addr_t taddr, size_t len, const void* src)
{
  assert(len == 8);
  target_endian<uint64_t> data;
  memcpy(&data, src, sizeof data);
  debug_mmu->store_uint64(taddr, debug_mmu->from_target(data));
}

void sim_t::set_target_endianness(memif_endianness_t endianness)
{
#ifdef RISCV_ENABLE_DUAL_ENDIAN
  assert(endianness == memif_endianness_little || endianness == memif_endianness_big);

  bool enable = endianness == memif_endianness_big;
  debug_mmu->set_target_big_endian(enable);
  for (size_t i = 0; i < nprocs(); i++) {
    get_core_by_idxinsim(i)->get_mmu()->set_target_big_endian(enable);
    get_core_by_idxinsim(i)->reset();
  }
#else
  assert(endianness == memif_endianness_little);
#endif
}

memif_endianness_t sim_t::get_target_endianness() const
{
#ifdef RISCV_ENABLE_DUAL_ENDIAN
  return debug_mmu->is_target_big_endian()? memif_endianness_big : memif_endianness_little;
#else
  return memif_endianness_little;
#endif
}

void sim_t::proc_reset(unsigned id)
{
  debug_module.proc_reset(id);
}

int sim_t::coreid_to_idxinsim(int coreid)
{
    for (int idxinsim = 0; idxinsim < (int)nprocs() ; idxinsim++) {
        if (coreid == (int)get_core_by_idxinsim(idxinsim)->get_id())
            return idxinsim;
    }

    return -1;
}

/* index, 从0开始, 0 ~ nprocs-1 */
processor_t* sim_t::get_core_by_idxinsim(int idxinsim)
{
    bank_t *pbank = nullptr;

    pbank = get_bank(get_bankid(idxinsim));
    if (pbank)
        return pbank->get_core_by_idxinbank(get_idxinbank(idxinsim));
    else
        return nullptr;
}

processor_t* sim_t::get_core_by_id(int procid)
{
    int idxinsim = 0;

    idxinsim = coreid_to_idxinsim(procid);
    if (0 <= idxinsim) {
        return get_core_by_idxinsim(idxinsim);
    } else {
        return nullptr;
    }
}

void sim_t::set_bank_finish(int bankid, bool finish)
{
    bank_t *bank = get_bank(bankid);
    if (bank) {
        bank->set_bank_finish(finish);
    } 
}

bool sim_t::is_bank_finish(int bankid)
{
    bank_t *bank = get_bank(bankid);
    if (bank) {
        return bank->is_bank_finish();
    } else {
        return true;
    }
}
