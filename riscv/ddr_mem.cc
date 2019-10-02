#include "processor.h"
#include "ddr_mem.h"

ddr_mem_t::ddr_mem_t(std::vector<processor_t*>& procs, size_t size)
  : procs(procs), len(size)
{
}

bool ddr_mem_t::load(reg_t addr, size_t len, uint8_t* bytes)
{
  return procs[0]->get_proxy()->ddrLoad(addr, len, bytes);
}

bool ddr_mem_t::store(reg_t addr, size_t len, const uint8_t* bytes)
{
  return procs[0]->get_proxy()->ddrStore(addr, len, bytes);
}

void ddr_mem_t::load_low_ddr() {
  data = (char*)calloc(1, len);
  procs[0]->get_proxy()->ddrLoad(0, len, (uint8_t *)data);
}