#include <fesvr/htif.h>

#include "devices.h"
#include "processor.h"
#include "pcie_driver.h"

mbox_device_t::mbox_device_t(pcie_driver_t *pcie, processor_t *p, misc_device_t *misc, bool pcie_enabled)
  : pcie_driver(pcie), p(p), pcie_enabled(pcie_enabled), misc_dev(misc)
{
  reset();
}

#ifdef MBOX_V1_ENABLE
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

#else
/* MBV2 Registers 64bit寄存器 */
#define MBV2_TXCFG_L            (0x00)
#define MBV2_TXCFG_H            (0x04)
#define MBV2_TXDAT_L            (0x08)
#define MBV2_TXDAT_H            (0x0C)
#define MBV2_RXREG_L            (0x10)
#define MBV2_RXREG_H            (0x14)
#define MBV2_STATUS             (0x18)
#define MBV2_INT_PEND           (0x20)
#define MBV2_INT_MASK           (0x28)      /* 0使能中断,1屏蔽 */

#define INT_REG_MASK            (0x1f)      /* 中断寄存器的有效位 */
#define INT_BIT_TX_DONE         (0)
#define INT_BIT_RX_VALID        (2)

#endif  /* #define MBOX_V1_ENABLE */

bool mbox_device_t::load(reg_t addr, size_t len, uint8_t* bytes)
{
    if (unlikely(!bytes || (size() <= addr+len))) {
        std::cout << "mbox: unsupported load register offset: " << hex << addr
            << " len: " << hex << len << std::endl;
        return false;
    }
#ifdef MBOX_V1_ENABLE
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

  memcpy(bytes, reg_base + addr, len);
#else
    switch(addr) {
    case MBV2_RXREG_L:
        if (cmd_value.empty())
            return false;

        for (int i = 0 ; i < len ; i+=4) {
            uint32_t value = cmd_value.front();
            cmd_value.pop();
            memcpy(bytes+i, &value, 4);
        }
        break;
    default:
        memcpy(bytes, (uint8_t*)reg_base+addr, len);
    }
#endif  /* #define MBOX_V1_ENABLE */
    return true;
}

bool mbox_device_t::store(reg_t addr, size_t len, const uint8_t* bytes)
{
    if (unlikely(!bytes || (size() <= addr+len))) {
        std::cout << "mbox: unsupported store register offset: " << hex << addr
            << " len: " << hex << len << std::endl;
        return false;
    }
#ifdef MBOX_V1_ENABLE
  if (MBOX_INT_PEND == addr) {
    uint32_t v = bytes[0] | (bytes[1] << 8) | (bytes[2] << 16) | (bytes[3] << 24);
    p->state.mextip &= ~v;
    p->state.mextip |= ((cmd_value.empty() ? 0 : 1) << RX_CFIFO_VAL) |
	    ((cmdext_value.empty() ? 0 : 1) << RX_EXT_CFIFO_VAL);
    *(uint32_t *)(reg_base + MBOX_INT_PEND) = p->state.mextip;
    return true;
  }

  memcpy(reg_base + addr, bytes, len);
  if (((MBOX_MTXCMD == addr) | (MBOX_MEXTTXCMD == addr)) &&
		((PCIE0_MBOX_MRXCMD == *(uint32_t *)reg_base) |
		(PCIE0_MBOX_MRXCMDEXT == *(uint32_t *)reg_base))) {
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
    *(uint32_t *)(reg_base + MBOX_INT_PEND) |= 1 << RX_CFIFO_VAL;
  }

  if (MBOX_MRXCMDEXT_ADDR == addr) {
    cmdext_count++;
    cmdext_value.push(*(uint32_t*)bytes);
    p->state.mextip = p->state.mextip | (1 << RX_EXT_CFIFO_VAL);
    *(uint32_t *)(reg_base + MBOX_INT_PEND) |= 1 << RX_EXT_CFIFO_VAL;
  }
#else
    switch(addr) {
    case MBV2_TXCFG_L:
        memcpy((uint8_t *)reg_base+addr, bytes, len);
        break;
    case MBV2_TXDAT_L:      /* 写入双字触发1次发送,发送CFG和DAT寄存器共16字节数据 */
        memcpy((uint8_t *)reg_base+addr, bytes, len);
        if (8 == len) {
            command_head_t cmd;
            cmd.code = CODE_INTERRUPT;
            cmd.addr = MBV2_TXDAT_L;
            cmd.len = 16;
            memcpy(cmd.data, (uint8_t *)reg_base+MBV2_TXCFG_L, 16);
            if(pcie_enabled) {
                pcie_driver->send((const uint8_t *)&cmd, PCIE_COMMAND_SEND_SIZE(cmd));
                uint32_t int_pend_val = 0;
                load(MBV2_INT_PEND, 4, (uint8_t*)(&int_pend_val));
                int_pend_val |= (1<<INT_BIT_TX_DONE);
                ro_register_write(MBV2_INT_PEND, (uint32_t)int_pend_val);
            }/*  */
        }
        break;
    /** 
     * 接收流程，pcie驱动端通过socket发起写操作，写16字节到RX寄存器
     * 收到数据后置位 RX_VALID，如果中断 enable 则产生中断
     */
    case MBV2_RXREG_L:
    {
        for (int i = 0 ; i < len; i+=4) {
            cmd_value.push(*(uint32_t*)((uint8_t *)bytes+i));
        }
        uint32_t int_pend_val = 0;
        load(MBV2_INT_PEND, 4, (uint8_t*)(&int_pend_val));
        int_pend_val |= (1<<INT_BIT_RX_VALID);
        ro_register_write(MBV2_INT_PEND, (uint32_t)int_pend_val);
    }
        break;
    case MBV2_INT_MASK:     /* 0是中断使能 */
        ro_register_write(addr, *(uint32_t*)bytes);
        break;
    case MBV2_INT_PEND:     /* 写 1 清相应位同时清 MIP.MEIP */
        {
        uint32_t clear_pend = *(uint32_t *)(bytes);
        uint32_t reg_int_pend = 0;
        load(MBV2_INT_PEND, 4, (uint8_t *)(&reg_int_pend));
        if (0 != reg_int_pend) {
            for (int i = 0 ; (1<<i) <= INT_REG_MASK ; i++) {
                if (((clear_pend & reg_int_pend & INT_REG_MASK)>>i) & 0x01) {
                    reg_int_pend &= ~(1<<i);
                }
            }
            *(uint32_t *)((uint8_t *)reg_base + MBV2_INT_PEND) = reg_int_pend;
            if (0 == reg_int_pend) {
                misc_dev->set_mcu_irq_status(MCU_IRQ_STATUS_BIT_NPC_MBOX_IRQ, false);
            }
        }
        }
        break;
    case MBV2_TXCFG_H:
    case MBV2_TXDAT_H:
    case MBV2_RXREG_H:
        return false;
    default:
        return false;
        break;
    }
#endif  /* #define MBOX_V1_ENABLE */
    return true;
}

void mbox_device_t::reset()
{
#ifdef MBOX_V1_ENABLE
  cmd_count = 0;
  cmdext_count = 0;
  p->state.mextip = 0;
  memset(reg_base, 0, 4096);

  while (!cmd_value.empty())
    cmd_value.pop();

  while (!cmdext_value.empty())
    cmdext_value.pop();
#else
    uint32_t val = 0;
    memset(reg_base, 0, size());
    val = INT_REG_MASK;
    store(MBV2_INT_MASK, 4, (uint8_t *)&val);

    while (!cmd_value.empty())
        cmd_value.pop();
#endif  /* #define MBOX_V1_ENABLE */
}

mbox_device_t::~mbox_device_t()
{
}

#ifndef MBOX_V1_ENABLE
bool mbox_device_t::ro_register_write(reg_t addr, uint32_t val)
{
    if (addr > size()-sizeof(val))
        return false;
    
    switch(addr) {
    case MBV2_INT_MASK:     /* 0是中断使能 */
    case MBV2_INT_PEND:     /* 1是中断产生 */
        {
        uint32_t reg_int_pend = 0;
        uint32_t reg_int_mask = 0;
        *(uint32_t *)((uint8_t *)reg_base+addr) = (val & INT_REG_MASK);
        if (!cmd_value.empty()) {
            *(uint32_t *)((uint8_t *)reg_base+MBV2_INT_PEND) |= (1<<INT_BIT_RX_VALID);
        }
        load(MBV2_INT_PEND, 4, (uint8_t *)(&reg_int_pend));
        load(MBV2_INT_MASK, 4, (uint8_t *)(&reg_int_mask));
        reg_int_mask = (~reg_int_mask) & INT_REG_MASK;
        if (reg_int_pend & reg_int_mask) {
            misc_dev->set_mcu_irq_status(MCU_IRQ_STATUS_BIT_NPC_MBOX_IRQ, true);
        }
        }
        break;
    default:
        return false;
    }
    
    return true;
}

bool mbox_device_t::ro_register_write(reg_t addr, uint64_t val)
{
    if (addr > size()-sizeof(val))
        return false;
    
    switch(addr) {
    case MBV2_INT_MASK:
    case MBV2_INT_PEND:
        return ro_register_write(addr, (uint32_t)(val&0xffffffff));
        break;
    default:
        return false;
    }
    
    return true;
}
#endif  /* #define MBOX_V1_ENABLE */