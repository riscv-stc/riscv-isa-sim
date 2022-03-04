#include <fesvr/htif.h>

#include "devices.h"
#include "processor.h"

#define UART_BASE  0x100
#define EXIT_BASE  0x500
#define DUMP_BASE  0x600

#define DUMP_START_OFFSET   0x0
#define DUMP_ADDR_OFFSET    0x8
#define DUMP_LEN_OFFSET     0x10

#define MCU_IRQ_STATUS_MASK     0x1fff
#define MCU_IRQ_ENABLE_MASK     0x1fff
#define MCU_IRQ_CLEAR_MASK      0x1fff

misc_device_t::misc_device_t(processor_t* proc)
  : proc(proc), buf_len(0x4000), dump_count(0)
{
    reg_base = new uint8_t[MISC_SIZE];
    memset(reg_base, 0, MISC_SIZE);
}

bool misc_device_t::load(reg_t addr, size_t len, uint8_t* bytes)
{
    if ((nullptr==reg_base) || (nullptr==bytes)) {
        return false;
    }
    if (/* (addr<0) ||  */(addr+len>=MISC_SIZE)) {
        return false;
    }
    memcpy(bytes, (uint8_t *)reg_base + addr, len);

    return true;
}

bool misc_device_t::store(reg_t addr, size_t len, const uint8_t* bytes)
{
  if (unlikely(!bytes || addr >= MISC_SIZE))
    return false;

    switch(addr) {
    case UART_BASE:     // uart device
        {
        for (size_t index = 0; index < len; index++) {
        if (unlikely('\n' == *bytes)) {
            std::cout << "cpu" << proc->get_id() << ":\t";
            for (int index = 0; index < buf.size(); index++)
            std::cout << buf[index];
            std::cout << std::endl;
            buf.clear();
        }
        else if (likely('\0' != *bytes)) {
            buf.push_back(*bytes);
        }

        bytes++;
        }
        }
        break;
    case EXIT_BASE:     // exit signal
        proc->set_exit();
        break;
    case (DUMP_BASE + DUMP_START_OFFSET):
        {
        auto prefix_addr = *((uint32_t*)bytes);
        std::string prefix = "snapshot-" + to_string(dump_count);
        if (prefix_addr != 0) {
        prefix = proc->get_sim()->addr_to_mem(prefix_addr);
        } else {
        dump_count ++;
        }
        proc->get_sim()->dump_mems(prefix, dump_addr, dump_len, proc->get_id());
        }
        break;
    case (DUMP_BASE + DUMP_ADDR_OFFSET):
        dump_addr = *((uint32_t*)bytes);
        break;
    case (DUMP_BASE + DUMP_LEN_OFFSET):
        dump_len = *((uint32_t*)bytes);
        break;
    case MCU_IRQ_STATUS_OFFSET:
    case MCU_IRQ_ENABLE_OFFSET:
        {
        memcpy((uint8_t *)reg_base+addr, bytes, len);
        *(uint32_t *)((uint8_t *)reg_base+addr) &= MCU_IRQ_ENABLE_MASK;
        uint32_t reg_status = *(uint32_t *)((uint8_t *)reg_base+MCU_IRQ_STATUS_OFFSET);
        uint32_t reg_enable = *(uint32_t *)((uint8_t *)reg_base+MCU_IRQ_ENABLE_OFFSET);
        if (reg_status & reg_enable) {
            proc->state.mip |= MIP_MEIP;
        }
        }
        break;
    case MCU_IRQ_CLEAR_OFFSET:
        {
        memcpy((uint8_t *)reg_base+addr, bytes, len);
        *(uint32_t *)((uint8_t *)reg_base+addr) &= MCU_IRQ_CLEAR_MASK;
        uint32_t reg_clear = *(uint32_t *)((uint8_t *)reg_base+addr);
        uint32_t reg_status = *(uint32_t *)((uint8_t *)reg_base+MCU_IRQ_STATUS_OFFSET);
        if (reg_status & reg_clear) {
            for (int i = 0 ; i < 32 ; i++) {
                if (((reg_status&reg_clear)>>i) & 0x01) {
                    *(uint32_t *)((uint8_t *)reg_base+MCU_IRQ_STATUS_OFFSET) &= ~(1<<i);
                }
            }
            proc->state.mip &= (~MIP_MEIP);
        }
        memset((uint8_t *)reg_base+addr, 0, 4);
        }
        break;
    default:
        memcpy((uint8_t *)reg_base+addr, bytes, len);
        break;
    }

  return true;
}

misc_device_t::~misc_device_t()
{
  if (!buf.empty()) {
    std::cout << static_cast<char*>(&buf[0]) << std::flush;
    buf.clear();
  }
  delete reg_base;
}
