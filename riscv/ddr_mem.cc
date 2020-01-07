#include <unistd.h>

#include "processor.h"
#include "ddr_mem.h"

ddr_mem_t::ddr_mem_t(std::vector<processor_t*>& procs, size_t size)
  : procs(procs), length(size)
{
  init();
}

void ddr_mem_t::init() {
  data = (char*)calloc(1, length);

  int block_size = 0x100000;
  for (int i = 0; i < length / block_size; i++) {
    int offset = block_size * i;
    int size = std::min(length - offset, (size_t)block_size);
    procs[0]->get_proxy()->ddrLoad(offset, size, (uint8_t *)data + offset);
  }
}

void ddr_mem_t::store_loop() {
  auto proxy = procs[0]->get_proxy();

  for (;;) {
    auto addr = 0;
    auto len = 0;
    {
      std::unique_lock<std::mutex> lock(store_mutex);
      store_cond.wait(lock);

      addr = store_dirty_start;
      len = store_dirty_end - store_dirty_start;

      if (!store_dirty || len == 0) continue;
      store_dirty_start = UINT32_MAX;
      store_dirty_end = 0;
      store_dirty = false;
    }

    proxy->ddrStore(addr, len, (uint8_t *)data + addr);
  }
}

bool ddr_mem_t::load(reg_t addr, size_t len, uint8_t* bytes)
{
  if(!data) init();

  // low addr use local caches
  if (addr + len < length) {
    memcpy(bytes, data + addr, len);
    return true;
  }

  // direct load from ddr module
  return procs[0]->get_proxy()->ddrLoad(addr, len, bytes);
}

bool ddr_mem_t::store(reg_t addr, size_t len, const uint8_t* bytes)
{
  // low addr use local caches
  if (addr + len < length) {
    memcpy(data + addr, bytes, len);
    return true;
  }

  // update dirty address ranges
  {
    std::lock_guard<std::mutex> lock(store_mutex);
    store_dirty = true;
    if (addr < store_dirty_start) store_dirty_start = addr;
    if (addr + len > store_dirty_end) store_dirty_end = addr + len;
  }

  // notify store loop thread to store to ddr module
  store_cond.notify_all();

  return true;
}