// See LICENSE for license details.

#ifndef _RISCV_SIMIF_H
#define _RISCV_SIMIF_H

#include "decode.h"
#include <iostream>
#include "bankif.h"
#include "soc_apb.h"

class bank_t;
class processor_t;

// this is the interface to the simulator used by the processors and memory
class simif_t
{
public:
  virtual bool is_bottom_ddr(reg_t addr) const = 0;
  virtual reg_t bottom_ddr_to_upper(reg_t addr,int bankid) const = 0;

  virtual char* addr_to_mem(reg_t addr) = 0;
  virtual char* bank_addr_to_mem(reg_t addr, uint32_t bank_id) = 0;
  virtual char* npc_addr_to_mem(reg_t addr, uint32_t bank_id, uint32_t idxinbank) = 0;

  // used for MMIO addresses
  virtual bool mmio_load(reg_t addr, size_t len, uint8_t* bytes) = 0;
  virtual bool mmio_store(reg_t addr, size_t len, const uint8_t* bytes) = 0;

  virtual bool bank_mmio_load(reg_t addr, size_t len, uint8_t* bytes, uint32_t bank_id) = 0;
  virtual bool bank_mmio_store(reg_t addr, size_t len, const uint8_t* bytes, uint32_t bank_id) = 0;

  virtual bool npc_mmio_load(reg_t addr, size_t len, uint8_t* bytes, uint32_t bank_id, uint32_t idxinbank) = 0;
  virtual bool npc_mmio_store(reg_t addr, size_t len, const uint8_t* bytes, uint32_t bank_id, uint32_t idxinbank) = 0;

  // Callback for processors to let the simulation know they were reset.
  virtual void proc_reset(unsigned id) = 0;
  virtual void hart_reset(reg_t hart_map) = 0;
  virtual void hart_core_reset(int id) = 0;
  virtual bool reset_signal(reg_t hart_id) = 0;
  virtual void clear_reset_signal(reg_t hart_id) = 0;
  virtual const char* get_symbol(uint64_t addr) = 0;
  virtual void dump_mems(std::string prefix, reg_t addr, size_t len, int proc_id) = 0;

    // get processor
    virtual unsigned nprocs(void) const = 0;
    virtual unsigned nbanks(void) const = 0;
    virtual bank_t* get_bank(int idx) = 0;
    virtual int get_bankid(int idxinsim) const = 0;
    virtual int get_idxinbank(int idxinsim) const = 0;
    virtual int get_id_first_bank(void) const = 0;
    virtual int coreid_to_idxinsim(int coreid) = 0;
    virtual processor_t* get_core_by_idxinsim(int idxinsim) = 0;

    virtual sys_irq_t *get_sysirq(void) = 0;
    virtual int get_groupID_from_coreID(int coreid) = 0;

    virtual void a53_step(void) = 0;
    virtual void pcie_step(void) = 0;
};

#endif
