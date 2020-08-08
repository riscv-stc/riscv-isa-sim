// See LICENSE for license details.

#ifndef _RISCV_SIMIF_H
#define _RISCV_SIMIF_H

#include "decode.h"

enum local_device_type {
  L1_BUFFER,
  IM_BUFFER,
  IO_DEVICE,
};

// this is the interface to the simulator used by the processors and memory
class simif_t
{
public:
  // should return NULL for MMIO addresses
  virtual char* addr_to_mem(reg_t addr) = 0;
  virtual char* local_addr_to_mem(reg_t addr, uint32_t idx) = 0;
  virtual char* local_addr_to_mem_by_id(reg_t addr, uint32_t id) = 0;

  // check if address in memory of type
  virtual bool in_local_mem(reg_t addr, local_device_type type) = 0;
  // used for MMIO addresses
  virtual bool mmio_load(reg_t addr, size_t len, uint8_t* bytes) = 0;
  virtual bool mmio_store(reg_t addr, size_t len, const uint8_t* bytes) = 0;
  virtual bool local_mmio_load(reg_t addr, size_t len, uint8_t* bytes, uint32_t idx) = 0;
  virtual bool local_mmio_store(reg_t addr, size_t len, const uint8_t* bytes, uint32_t idx) = 0;

  // Callback for processors to let the simulation know they were reset.
  virtual void proc_reset(unsigned id) = 0;
  virtual void dump_mems(std::string prefix, reg_t addr, size_t len, int proc_id) = 0;
};

#endif
