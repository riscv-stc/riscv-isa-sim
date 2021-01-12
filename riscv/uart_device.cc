#include <fesvr/htif.h>

#include "devices.h"
#include "processor.h"

#define UART0_THR  0x0
#define UART0_LSR  0x14

uart_device_t::uart_device_t()
{
}

bool uart_device_t::load(reg_t addr, size_t len, uint8_t* bytes)
{
  memset(bytes, 0x20, len);
  return true;
}

bool uart_device_t::store(reg_t addr, size_t len, const uint8_t* bytes)
{
  if (unlikely(!bytes || addr >= 0x30))
    return false;

  if (addr == UART0_THR) {
    // uart device
    for (size_t index = 0; index < len; index++) {
      if (unlikely('\n' == *bytes)) {
        std::cout << "uart0" << ":\t";
        for (int index = 0; index < data.size(); index++)
          std::cout << data[index];
        std::cout << std::endl;
        data.clear();
      }
      else if (likely('\0' != *bytes)) {
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
