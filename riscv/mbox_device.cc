#include <fesvr/htif.h>

#include "devices.h"
#include "processor.h"
#include "pcie_driver.h"

mbox_device_t::mbox_device_t(pcie_driver_t *pcie, processor_t *p, bool pcie_enabled)
  : pcie_driver(pcie), p(p), pcie_enabled(pcie_enabled)
{
  cmd_count = 0;
  cmdext_count = 0;
}

#define RX_CFIFO_VAL     0x2
#define RX_EXT_CFIFO_VAL 0x4
#define MBOX_MTXCFG         (0x0)
#define MBOX_MTXCMD         (0x4)
#define MBOX_MEXTTXCMD      (0x8)
#define PCIE0_MBOX_MRXCMD    (0xE30A100C)
#define PCIE0_MBOX_MRXCMDEXT    (0xE30A1010)
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
    // if (cmd_value.empty()) {
      // p->state.mextip &= ~(1 << RX_CFIFO_VAL);
      // *(uint32_t *)(data + MBOX_INT_PEND) &= ~(1 << RX_CFIFO_VAL);
    // }
    return true;
  }

  if (MBOX_MRXCMDEXT_ADDR == addr) {
    if (cmdext_value.empty())
      return false;

    uint32_t value = cmdext_value.front();
    cmdext_value.pop();
    cmdext_count--;

    memcpy(bytes, &value, 4);
    // if (cmdext_value.empty()) {
      // p->state.mextip &= ~(1 << RX_EXT_CFIFO_VAL);
      // *(uint32_t *)(data + MBOX_INT_PEND) &= ~(1 << RX_EXT_CFIFO_VAL);
    //}
    return true;
  }

  memcpy(bytes, data + addr, len);
  return true;
}

bool mbox_device_t::store(reg_t addr, size_t len, const uint8_t* bytes)
{
  if (unlikely(!bytes || addr >= 0x1000))
    return false;

  if (MBOX_INT_PEND == addr) {
    uint32_t v = bytes[0] | (bytes[1] << 8) | (bytes[2] << 16) | (bytes[3] << 24);
    p->state.mextip &= ~v;
    p->state.mextip |= ((cmd_value.empty() ? 0 : 1) << RX_CFIFO_VAL) |
	    ((cmdext_value.empty() ? 0 : 1) << RX_EXT_CFIFO_VAL);
    *(uint32_t *)(data + MBOX_INT_PEND) = p->state.mextip;
    return true;
  }

  memcpy(data + addr, bytes, len);
  if (((MBOX_MTXCMD == addr) | (MBOX_MEXTTXCMD == addr)) &&
		((PCIE0_MBOX_MRXCMD == *(uint32_t *)data) |
		(PCIE0_MBOX_MRXCMDEXT == *(uint32_t *)data))) {
    command_head_t cmd;
    cmd.code = CODE_INTERRUPT;
    cmd.addr = addr;
    cmd.len = 4;
    *(uint32_t *)cmd.data = *(uint32_t *)bytes;
    if(pcie_enabled)
    	pcie_driver->send((const uint8_t *)&cmd, sizeof(cmd));
  }

  if (MBOX_MRXCMD_ADDR == addr) {
    cmd_count++;
    cmd_value.push(*(uint32_t*)bytes);
    p->state.mextip = p->state.mextip | (1 << RX_CFIFO_VAL);
    *(uint32_t *)(data + MBOX_INT_PEND) |= 1 << RX_CFIFO_VAL;
  }

  if (MBOX_MRXCMDEXT_ADDR == addr) {
    cmdext_count++;
    cmdext_value.push(*(uint32_t*)bytes);
    p->state.mextip = p->state.mextip | (1 << RX_EXT_CFIFO_VAL);
    *(uint32_t *)(data + MBOX_INT_PEND) |= 1 << RX_EXT_CFIFO_VAL;
  }

  return true;
}

void mbox_device_t::reset()
{
  cmd_count = 0;
  cmdext_count = 0;
  p->state.mextip = 0;
  memset(data, 0, 4096);

  while (!cmd_value.empty())
    cmd_value.pop();

  while (!cmdext_value.empty())
    cmdext_value.pop();
}

mbox_device_t::~mbox_device_t()
{
}

