// See LICENSE for license details.
#ifndef _RISCV_DDR_MEM_H
#define _RISCV_DDR_MEM_H

#include <cstdlib>
#include <vector>
#include <condition_variable>
#include <mutex>
#include <thread>

#include "devices.h"

class processor_t;

class ddr_mem_t : public abstract_device_t {
 public:
  ddr_mem_t(std::vector<processor_t*>& procs, size_t size = 0x100000);
  bool load(reg_t addr, size_t len, uint8_t* bytes);
  bool store(reg_t addr, size_t len, const uint8_t* bytes);
  char* contents() { if(!data) init(); return data; }
  size_t size() { return length; }
 private:
  std::vector<processor_t*>& procs;
  char* data = NULL;
  size_t length;

  void init();

  void store_loop();

  std::unique_ptr<std::thread> store_thread;
  std::condition_variable_any store_cond;
  std::mutex store_mutex;

  bool store_dirty = false;
  reg_t store_dirty_start = UINT32_MAX;
  reg_t store_dirty_end = 0;
};

#endif
