// See LICENSE for license details.
#ifndef _RISCV_DDR_MEM_H
#define _RISCV_DDR_MEM_H

#include <cstdlib>
#include <vector>

#include "devices.h"

class processor_t;

class ddr_mem_t : public abstract_device_t {
 public:
  ddr_mem_t(std::vector<processor_t*>& procs, size_t size = 0x100000);
  bool load(reg_t addr, size_t len, uint8_t* bytes);
  bool store(reg_t addr, size_t len, const uint8_t* bytes);
  char* contents() { if(!data) load_low_ddr(); return data; }
  size_t size() { return len; }
 private:
  std::vector<processor_t*>& procs;
  char* data = NULL;
  size_t len;

  void load_low_ddr();
};

#endif
