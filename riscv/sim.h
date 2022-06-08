// See LICENSE for license details.

#ifndef _RISCV_SIM_H
#define _RISCV_SIM_H

#include "processor.h"
#include "devices.h"
#include "pcie_driver.h"
#include "debug_module.h"
#include "simif.h"
#include <fesvr/htif.h>
#include <fesvr/context.h>
#include <vector>
#include <string>
#include <memory>
#include <mutex>
#include <sys/types.h>

class mmu_t;
class remote_bitbang_t;

// this class encapsulates the processors and memory in a RISC-V machine.
class sim_t : public htif_t, public simif_t
{
public:
  sim_t(const char* isa, size_t _nprocs, size_t bank_id,
        char *hwsync_masks, uint32_t hwsync_timer_num, bool halted, reg_t start_pc,
        std::vector<std::pair<reg_t, mem_t*>> mems, size_t ddr_size,
        const std::vector<std::string>& args, const std::vector<int> hartids,
        unsigned progsize, unsigned max_bus_master_bits,
        bool require_authentication, suseconds_t abstract_delay_usec,
        bool support_hasel, bool support_abstract_csr_access, bool pcie_enabled, 
        size_t board_id, size_t chip_id, uint32_t coremask);
  ~sim_t();

  // run the simulation to completion
  int run(std::vector<std::string> load_files,
          std::vector<std::string> init_dump,
          std::vector<std::string> exit_dump,
          std::string dump_path);

  void dump_mems() override;
  void dump_mems(std::string prefix, reg_t addr, size_t len, int proc_id) override;

  void set_debug(bool value);
  void set_log(bool value);
  void set_histogram(bool value);
  void set_procs_debug(bool value);
  void set_dtb_enabled(bool value) {
    this->dtb_enabled = value;
  }
  void set_remote_bitbang(remote_bitbang_t* remote_bitbang) {
    this->remote_bitbang = remote_bitbang;
  }
  const char* get_dts() { if (dts.empty()) reset(); return dts.c_str(); }
  processor_t* get_core(size_t i) { return procs.at(i); }
  unsigned nprocs() const { return procs.size(); }
  void hart_reset(reg_t hart_map) {
    rst_mutex.lock();
    core_reset_n = hart_map;
    rst_mutex.unlock();
  }
  bool reset_signal(reg_t hart_id) { return core_reset_n & (0x1 << hart_id); };
  void clear_reset_signal(reg_t hart_id) {
    rst_mutex.lock();
    core_reset_n &= ~(0x1 << hart_id);
    rst_mutex.unlock();
  }
  // Callback for processors to let the simulation know they were reset.
  void proc_reset(unsigned id);

private:
  std::vector<std::pair<reg_t, mem_t*>> mems;
  mmu_t* debug_mmu;  // debug port into main memory
  std::vector<processor_t*> procs;
  size_t bank_id;
  char *hwsync_masks;
  reg_t start_pc;
  std::string dts;
  std::unique_ptr<rom_device_t> boot_rom;
  std::unique_ptr<clint_t> clint;
  bus_t bus;
  std::vector<bus_t*> local_bus;
  std::vector<bus_t*> sub_bus;
  pcie_driver_t *pcie_driver;
  volatile reg_t core_reset_n;
  std::mutex rst_mutex;

  hwsync_t *hwsync;
  share_mem_t *llb;
  share_mem_t *l1;

  processor_t* get_core(const std::string& i);
  void step(size_t n); // step through simulation
  static const size_t INTERLEAVE = 500;
  static const size_t INSNS_PER_RTC_TICK = 100; // 10 MHz clock for 1 BIPS core
  static const size_t CPU_HZ = 1000000000; // 1GHz CPU
  size_t current_step;
  size_t current_proc;
  bool debug;
  bool log;
  bool histogram_enabled; // provide a histogram of PCs
  bool dtb_enabled;
  bool pcie_enabled;
  remote_bitbang_t* remote_bitbang;

  std::vector<std::string> exit_dump;
  std::string dump_path;

  // memory-mapped I/O routines
  char* addr_to_mem(reg_t addr);
  char* local_addr_to_mem(reg_t addr, uint32_t idx);
  char* local_addr_to_mem_by_id(reg_t addr, uint32_t id);
  char* local_addr_to_mem_by_id_cluster(reg_t addr, uint32_t id);
  bool in_local_mem(reg_t addr, local_device_type type);
  bool mmio_load(reg_t addr, size_t len, uint8_t* bytes);
  bool mmio_store(reg_t addr, size_t len, const uint8_t* bytes);
  bool local_mmio_load(reg_t addr, size_t len, uint8_t* bytes, uint32_t idx);
  bool local_mmio_store(reg_t addr, size_t len, const uint8_t* bytes, uint32_t idx);
  void make_dtb();

  int8_t high_mem_bank_id_from_addr(reg_t addr);
  bool in_high_mem(reg_t addr);
  char* sub_bus_addr_to_mem(reg_t addr);

  // presents a prompt for introspection into the simulation
  void interactive();

  // functions that help implement interactive()
  void interactive_help(const std::string& cmd, const std::vector<std::string>& args);
  void interactive_quit(const std::string& cmd, const std::vector<std::string>& args);
  void interactive_run(const std::string& cmd, const std::vector<std::string>& args, bool noisy);
  void interactive_run_noisy(const std::string& cmd, const std::vector<std::string>& args);
  void interactive_run_silent(const std::string& cmd, const std::vector<std::string>& args);
  void interactive_reg(const std::string& cmd, const std::vector<std::string>& args);
  void interactive_freg(const std::string& cmd, const std::vector<std::string>& args);
  void interactive_fregs(const std::string& cmd, const std::vector<std::string>& args);
  void interactive_fregd(const std::string& cmd, const std::vector<std::string>& args);
  void interactive_vreg(const std::string& cmd, const std::vector<std::string>& args);
  void interactive_vregh(const std::string& cmd, const std::vector<std::string>& args);
  void interactive_pc(const std::string& cmd, const std::vector<std::string>& args);
  void interactive_mem(const std::string& cmd, const std::vector<std::string>& args);
  void interactive_str(const std::string& cmd, const std::vector<std::string>& args);
  void interactive_until(const std::string& cmd, const std::vector<std::string>& args, bool noisy);
  void interactive_until_silent(const std::string& cmd, const std::vector<std::string>& args);
  void interactive_until_noisy(const std::string& cmd, const std::vector<std::string>& args);
  reg_t get_reg(const std::vector<std::string>& args);
  freg_t get_freg(const std::vector<std::string>& args);
  vreg_t get_vreg(const std::vector<std::string>& args);
  reg_t get_mem(const std::vector<std::string>& args);
  reg_t get_pc(const std::vector<std::string>& args);

  friend class processor_t;
  friend class mmu_t;
  friend class debug_module_t;

  // htif
  friend void sim_thread_main(void*);
  void main();

  context_t* host;
  context_t target;
  void reset();
  void idle();
  void read_chunk(addr_t taddr, size_t len, void* dst);
  void write_chunk(addr_t taddr, size_t len, const void* src);
  size_t chunk_align() { return 8; }
  size_t chunk_max_size() { return 8; }

  void load_mems(std::vector<std::string> load_files);
  void dump_mems(std::string prefix, std::vector<std::string> mems, std::string path);
  void load_mem(const char *path, reg_t off, size_t len);
  void load_mem(const char *path, reg_t off, size_t len, int proc_id);
  void dump_mem(const char *path, reg_t off, size_t len, int proc_id, bool space_end = false);

public:
  // Initialize this after procs, because in debug_module_t::reset() we
  // enumerate processors, which segfaults if procs hasn't been initialized
  // yet.
  debug_module_t debug_module;
  bool has_hwsync_masks() {return (hwsync_masks[0] == 0) ? false : true;};
};

extern volatile bool ctrlc_pressed;

#endif
