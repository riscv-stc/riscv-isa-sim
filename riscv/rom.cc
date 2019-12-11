#include "devices.h"

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

uart_device_t::uart_device_t()
{
}

bool uart_device_t::load(reg_t addr, size_t len, uint8_t* bytes)
{  
  memset(bytes, 0x5a, len);
  return true;
}

bool uart_device_t::store(reg_t addr, size_t len, const uint8_t* bytes)
{
  if (unlikely(!bytes || addr >= 0x1000))
    return false;

  if (addr == 0x100) {
    for (size_t index = 0; index < len; index++) {
      if (unlikely(0 == *bytes)) {
        for (int index = 0; index < data.size(); index++)
          std::cout << data[index];
        std::cout << std::flush;
        data.clear();
      }
      else {
        data.push_back(*bytes);
      }
      
      bytes++;
    }
  }
  
  return true;
}

uart_device_t::~uart_device_t()
{
  if (!data.empty()) {
    std::cout << static_cast<char*>(&data[0]) << std::flush;
    data.clear();
  }
}

