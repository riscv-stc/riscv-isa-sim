#include "devices.h"
#include "processor.h"
#include "pcie_driver.h"

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

misc_device_t::misc_device_t(processor_t* proc)
  : proc(proc), len(0x4000)
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

  if (addr == 0x100) {
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
  } else if (addr == 0x500) {
    // exit signal
    proc->set_exit();
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

mbox_device_t::mbox_device_t(pcie_driver_t *pcie, std::vector<processor_t*>& p) 
  : pcie_driver(pcie), procs(p)
{
  cmd_count = 0;
  cmdext_count = 0;
}

#define RX_CFIFO_VAL     0x2
#define RX_EXT_CFIFO_VAL 0x3
#define MBOX_MTXCFG      0x0
#define MBOX_MTXCMD      0x4
#define PCIE0_MBOX_MRXCMD    (0xC60A100C)
#define MBOX_MRXCMD_ADDR     (0x0c)
#define MBOX_MRXCMDEXT_ADDR  (0x10)
#define MBOX_INT_PEND        (0x20)
bool mbox_device_t::load(reg_t addr, size_t len, uint8_t* bytes)
{  
  if (unlikely(!bytes || addr >= 0x1000))
    return false;
  
  if (MBOX_MRXCMD_ADDR == addr) {
    if (cmd_value.empty())
      return false;
    
    uint32_t value = cmd_value.front();
    cmd_value.pop();
    cmd_count--;

    memcpy(bytes, &value, 4);
    if (cmd_value.empty()) {
      procs[0]->state.mextip &= ~(1 << RX_CFIFO_VAL);
      *(uint32_t *)(data + MBOX_INT_PEND) &= ~(1 << RX_CFIFO_VAL);
    }
    return true;
  }

  if (MBOX_MRXCMDEXT_ADDR == addr) {
    if (cmdext_value.empty())
      return false;
    
    uint32_t value = cmdext_value.front();
    cmdext_value.pop();
    cmdext_count--;

    memcpy(bytes, &value, 4);
    if (cmdext_value.empty()) {
      procs[0]->state.mextip &= ~(1 << RX_EXT_CFIFO_VAL);
      *(uint32_t *)(data + MBOX_INT_PEND) &= ~(1 << RX_EXT_CFIFO_VAL);
    }
    return true;
  }
    
  memcpy(bytes, data + addr, len);
  return true;
}

bool mbox_device_t::store(reg_t addr, size_t len, const uint8_t* bytes)
{
  if (unlikely(!bytes || addr >= 0x1000))
    return false;

  memcpy(data + addr, bytes, len);
  if (MBOX_MTXCMD == addr && PCIE0_MBOX_MRXCMD == *(uint32_t *)data) {
    command_head_t cmd;
    cmd.code = CODE_INTERRUPT;
    cmd.addr = 0;
    cmd.len = 4;
    *(uint32_t *)cmd.data = *(uint32_t *)bytes;
    pcie_driver->send((const uint8_t *)&cmd, sizeof(cmd));
  }

  if (MBOX_MRXCMD_ADDR == addr) {
    cmd_count++;
    cmd_value.push(*(uint32_t*)bytes);
    procs[0]->state.mextip = procs[0]->state.mextip | (1 << RX_CFIFO_VAL);
    *(uint32_t *)(data + MBOX_INT_PEND) |= 1 << RX_CFIFO_VAL;
  }

  if (MBOX_MRXCMDEXT_ADDR == addr) {
    cmdext_count++;
    cmdext_value.push(*(uint32_t*)bytes);
    procs[0]->state.mextip = procs[0]->state.mextip | (1 << RX_EXT_CFIFO_VAL);
    *(uint32_t *)(data + MBOX_INT_PEND) |= 1 << RX_EXT_CFIFO_VAL;
  }
    
  return true;
}

mbox_device_t::~mbox_device_t()
{
}

