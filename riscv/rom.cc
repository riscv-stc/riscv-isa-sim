#include <fesvr/htif.h>

#include "devices.h"
#include "processor.h"

rom_device_t::rom_device_t(std::vector<char> data)
  : data(data)
{
}

bool rom_device_t::load(reg_t addr, size_t len, uint8_t* bytes)
{
  if (addr + len > data.size())
    return false;
  memcpy(bytes, &data[addr], len);
  return true;
}

bool rom_device_t::store(reg_t addr, size_t len, const uint8_t* bytes)
{
  return false;
}