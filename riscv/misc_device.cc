#include <fesvr/htif.h>

#include "devices.h"
#include "processor.h"

#define UART_BASE  0x100
#define EXIT_BASE  0x500
#define DUMP_BASE  0x600

#define DUMP_START_OFFSET   0x0
#define DUMP_ADDR_OFFSET    0x4
#define DUMP_LEN_OFFSET     0x8


misc_device_t::misc_device_t(processor_t* proc)
  : proc(proc), len(0x4000), dump_count(0)
{
}

bool misc_device_t::load(reg_t addr, size_t len, uint8_t* bytes)
{
  memset(bytes, 0x5a, len);
  return true;
}

bool misc_device_t::store(reg_t addr, size_t len, const uint8_t* bytes)
{
  if (unlikely(!bytes || addr >= 0x1000))
    return false;

  if (addr == UART_BASE) {
    // uart device
    for (size_t index = 0; index < len; index++) {
      if (unlikely('\n' == *bytes)) {
        std::cout << "cpu" << proc->get_id() << ":\t";
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
  } else if (addr == EXIT_BASE) {
    // exit signal
    proc->set_exit();
  } else if (addr == DUMP_BASE + DUMP_START_OFFSET) {
    auto prefix_addr = *((uint32_t*)bytes);
    std::string prefix = "snapshot-" + to_string(dump_count);
    if (prefix_addr != 0) {
      prefix = proc->get_sim()->addr_to_mem(prefix_addr);
    } else {
      dump_count ++;
    }
    proc->get_sim()->dump_mems(prefix, dump_addr, dump_len, proc->get_id());
  } else if (addr == DUMP_BASE + DUMP_ADDR_OFFSET) {
    dump_addr = *((uint32_t*)bytes);
  } else if (addr == DUMP_BASE + DUMP_LEN_OFFSET) {
    dump_len = *((uint32_t*)bytes);
  }

  return true;
}

misc_device_t::~misc_device_t()
{
  if (!data.empty()) {
    std::cout << static_cast<char*>(&data[0]) << std::flush;
    data.clear();
  }
}
