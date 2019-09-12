// See LICENSE for license details.

#ifndef _RISCV_SIMIF_H
#define _RISCV_SIMIF_H

#include "decode.h"

enum access_unit {
  MCU,
  NCP,
  TCP, 
};

// this is the interface to the simulator used by the processors and memory
class simif_t
{
public:
  // should return NULL for MMIO addresses
  virtual char* addr_to_mem(reg_t addr) = 0;
  // used for MMIO addresses
  virtual bool mmio_load(reg_t addr, size_t len, uint8_t* bytes) = 0;
  virtual bool mmio_store(reg_t addr, size_t len, const uint8_t* bytes) = 0;
  // Callback for processors to let the simulation know they were reset.
  virtual void proc_reset(unsigned id) = 0;
  virtual void load_heap(const char *path) = 0;
  virtual void dump_heap(const char *path, size_t len) = 0;
  virtual access_unit set_aunit(access_unit unit) = 0;
};

#endif
