#include <fesvr/htif.h>

#include "devices.h"
#include "processor.h"
#include "pcie_driver.h"

mbox_device_t::mbox_device_t(pcie_driver_t *pcie, processor_t *p, misc_device_t *misc, bool pcie_enabled)
  : pcie_driver(pcie), p(p), pcie_enabled(pcie_enabled), misc_dev(misc)
{
  reset();
}

/* 64bit寄存器 */
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

bool mbox_device_t::load(reg_t addr, size_t len, uint8_t* bytes)
{
    if (unlikely(!bytes || addr >= size()))
        return false;
    
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
    return true;
}

bool mbox_device_t::store(reg_t addr, size_t len, const uint8_t* bytes)
{
    if (unlikely(!bytes || addr >= size()))
        return false;

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

    return true;
}

void mbox_device_t::reset()
{
    uint32_t val = 0;
    memset(reg_base, 0, size());
    val = INT_REG_MASK;
    store(MBV2_INT_MASK, 4, (uint8_t *)&val);

    while (!cmd_value.empty())
        cmd_value.pop();
}

mbox_device_t::~mbox_device_t()
{
}

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