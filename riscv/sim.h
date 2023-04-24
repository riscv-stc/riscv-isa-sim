// See LICENSE for license details.

#ifndef _RISCV_SIM_H
#define _RISCV_SIM_H

#include "debug_module.h"
#include "devices.h"
#include "log_file.h"
#include "processor.h"
#include "simif.h"
#include "bank.h"
#include "apifc.h"

#include <fesvr/htif.h>
#include <fesvr/context.h>
#include <vector>
#include <string>
#include <memory>
#include <sys/types.h>
#include "ap_mbox.h"
#include "soc_apb.h"
#include "noc_addr.h"

class mmu_t;
class remote_bitbang_t;

// this class encapsulates the processors and memory in a RISC-V machine.
class sim_t : public htif_t, public simif_t
{
public:
  sim_t(const char* isa, const char* priv, const char* varch, size_t _nprocs, int _nbanks,
        size_t id_first_bank, size_t die_id, char *hwsync_masks, uint32_t hwsync_timer_num, bool halted, bool real_time_clint,
        reg_t initrd_start, reg_t initrd_end, const char* bootargs,
        reg_t start_pc, std::vector<std::pair<reg_t, mem_t*>> mems, size_t ddr_size,
        std::vector<std::pair<reg_t, abstract_device_t*>> plugin_devices,
        const std::vector<std::string>& args, const std::vector<int> hartids,
        const debug_module_config_t &dm_config, const char *log_path,
        bool dtb_enabled, const char *dtb_file, bool pcie_enabled, bool file_name_with_bank_id, 
        size_t board_id, size_t chip_id, size_t session_id, uint32_t coremask, const char *atuini);
  ~sim_t();

  // run the simulation to completion
  int run(std::vector<std::string> load_files,
          std::vector<std::string> init_dump,
          std::vector<std::string> exit_dump,
          std::string dump_path);

  void dump_mems() override;
  void dump_mems(std::string prefix, reg_t addr, size_t len, int proc_id) override;

  void set_debug(bool value);
  void set_histogram(bool value);

  // Configure logging
  //
  // If enable_log is true, an instruction trace will be generated. If
  // enable_commitlog is true, so will the commit results (if this
  // build was configured without support for commit logging, the
  // function will print an error message and abort).
  void configure_log(bool enable_log, bool enable_commitlog);

  void set_procs_debug(bool value);
  void set_remote_bitbang(remote_bitbang_t* remote_bitbang) {
    this->remote_bitbang = remote_bitbang;
  }
  const char* get_dts() { if (dts.empty()) reset(); return dts.c_str(); }
  
  unsigned nprocs(void) const { return sim_nprocs; }
  unsigned nbanks(void) const { return sim_nbanks; }

  bank_t* get_bank(int idx) { return banks.at(idx); }
  int get_bankid(int idxinsim) const { return  idxinsim/core_num_of_bank + id_first_bank;};
  processor_t* get_core_by_idxinsim(int idxinsim);        /* index 从0开始 */
  int get_idxinbank(int idxinsim) const { return  idxinsim%core_num_of_bank;};
  int get_id_first_bank(void) const {return id_first_bank;};
  int coreid_to_idxinsim(int coreid);
  int get_groupID_from_coreID(int coreid) {return hwsync->get_groupID_from_coreID(coreid);}
  void npc_pld_clr(int coreid) {hwsync->pld_clr(coreid);};
  void hpe_debug_backdoor(void);

  void hart_core_reset(int id)
  {
    rst_mutex.lock();
    core_reset_n |= ((1<<id)&0xffffffff);
    rst_mutex.unlock();
  }
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

  void a53_step(void)
  {
    if (likely(pcie_enabled && apifc)) {
      apifc->process_data();
    }
  };
  void pcie_step(void)
  {
    if (likely(pcie_enabled && pcie_driver)) {
      pcie_driver->process_data();
    }
  }
private:
  std::vector<std::pair<reg_t, mem_t*>> mems;
  std::vector<std::pair<reg_t, abstract_device_t*>> plugin_devices;
  mmu_t* debug_mmu;  // debug port into main memory
  std::vector<bank_t *> banks;   /* 一个spike最多支持4个bank. bank id == bank index */
  int core_num_of_bank;             /* 一个bank内的core数 */
  size_t id_first_bank;             /* 多bank时第一个bank的id */
  int sim_nprocs;
  int sim_nbanks;
  size_t die_id;
  char *hwsync_masks;
  reg_t initrd_start;
  reg_t initrd_end;
  const char* bootargs;
  reg_t start_pc;
  std::string dts;
  std::string dtb;
  std::string dtb_file;
  bool dtb_enabled;
  std::unique_ptr<rom_device_t> boot_rom;
  std::unique_ptr<clint_t> clint;
  log_file_t log_file;
  bus_t glb_bus;
  soc_apb_t *soc_apb = nullptr;
  ap_mbox_t *p2ap_mbox = nullptr;
  ap_mbox_t *n2ap_mbox = nullptr;
  pcie_driver_t *pcie_driver = nullptr;
  bank_misc_dev_t *bank_misc[4] = {nullptr};
  volatile reg_t core_reset_n;
  std::mutex rst_mutex;

  std::vector<std::string> exit_dump;
  std::string dump_path;

  processor_t* get_core_by_id(int procid);
  processor_t* get_core_by_id(const std::string& i);
  void step(size_t n); // step through simulation
  static const size_t INTERLEAVE = 5000;
  static const size_t INSNS_PER_RTC_TICK = 100; // 10 MHz clock for 1 BIPS core
  static const size_t CPU_HZ = 1000000000; // 1GHz CPU
  size_t current_step;
  size_t current_proc;
  bool debug;
  bool histogram_enabled; // provide a histogram of PCs
  bool log;
  bool pcie_enabled;
  bool file_name_with_bank_id;
  remote_bitbang_t* remote_bitbang;
  hwsync_t *hwsync;
  apifc_t *apifc = nullptr;

  int sysdma_addr_to_bankid(reg_t addr);

  // memory-mapped I/O routines
  bool is_upper_mem(reg_t addr);
  int get_bankid_by_uppermem(reg_t addr);

  bool is_bottom_ddr(reg_t addr) const ;
  reg_t bottom_ddr_to_upper(reg_t addr,int bankid) const;

  char* addr_to_mem(reg_t addr);
  char* bank_addr_to_mem(reg_t addr, uint32_t bank_id);
  char* npc_addr_to_mem(reg_t addr, uint32_t bank_id, uint32_t idxinbank);

  bool in_mmio(reg_t addr);
  bool mmio_load(reg_t addr, size_t len, uint8_t* bytes);
  bool mmio_store(reg_t addr, size_t len, const uint8_t* bytes);

  bool bank_mmio_load(reg_t addr, size_t len, uint8_t* bytes, uint32_t bank_id);
  bool bank_mmio_store(reg_t addr, size_t len, const uint8_t* bytes, uint32_t bank_id);

  bool npc_mmio_load(reg_t addr, size_t len, uint8_t* bytes, uint32_t bank_id, uint32_t idx);
  bool npc_mmio_store(reg_t addr, size_t len, const uint8_t* bytes, uint32_t bank_id, uint32_t idx);

  sys_irq_t *get_sysirq(void) {return soc_apb->get_sysirq();};

  void make_dtb();
  void set_rom();

  const char* get_symbol(uint64_t addr);

  // presents a prompt for introspection into the simulation
  void interactive();

  // functions that help implement interactive()
  void interactive_help(const std::string& cmd, const std::vector<std::string>& args);
  void interactive_quit(const std::string& cmd, const std::vector<std::string>& args);
  void interactive_run(const std::string& cmd, const std::vector<std::string>& args, bool noisy);
  void interactive_run_noisy(const std::string& cmd, const std::vector<std::string>& args);
  void interactive_run_silent(const std::string& cmd, const std::vector<std::string>& args);
  void interactive_vreg(const std::string& cmd, const std::vector<std::string>& args);
  void interactive_reg(const std::string& cmd, const std::vector<std::string>& args);
  void interactive_freg(const std::string& cmd, const std::vector<std::string>& args);
  void interactive_fregh(const std::string& cmd, const std::vector<std::string>& args);
  void interactive_fregs(const std::string& cmd, const std::vector<std::string>& args);
  void interactive_fregd(const std::string& cmd, const std::vector<std::string>& args);
  void interactive_pc(const std::string& cmd, const std::vector<std::string>& args);
  void interactive_mem(const std::string& cmd, const std::vector<std::string>& args);
  void interactive_str(const std::string& cmd, const std::vector<std::string>& args);
  void interactive_until(const std::string& cmd, const std::vector<std::string>& args, bool noisy);
  void interactive_until_silent(const std::string& cmd, const std::vector<std::string>& args);
  void interactive_until_noisy(const std::string& cmd, const std::vector<std::string>& args);
  void interactive_dump_mem(const std::string& cmd, const std::vector<std::string>& args);
  reg_t get_reg(const std::vector<std::string>& args);
  freg_t get_freg(const std::vector<std::string>& args);
  reg_t get_mem(const std::vector<std::string>& args);
  reg_t get_pc(const std::vector<std::string>& args);
  reg_t get_pc_by_id(int procid);

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
  void set_target_endianness(memif_endianness_t endianness);
  memif_endianness_t get_target_endianness() const;

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
