#include <fesvr/htif.h>

#include "devices.h"
#include "processor.h"
#include "pcie_driver.h"
#include "mbox_device.h"

mbox_device_t::mbox_device_t(simif_t *simif) : sim(simif)
{
}

mbox_device_t::~mbox_device_t()
{
}

bool mbox_device_t::load(reg_t addr, size_t len, uint8_t* bytes)
{
    uint32_t val32 = 0;
    uint64_t val64 = 0;
    int fifo_size = 0;
    bool isempty = true;

    if (unlikely(!bytes || ((size()<addr+len)) || ((4!=len) && (8!=len)))) {
        std::cout << "mbox: unsupported load register offset: " << hex << addr
            << " len: " << hex << len << std::endl;
        return false;
    }

    switch(addr) {
    case MBOX_RX_CFG_DATA:
    case MBOX_RX_CFG_DATA+4:
        if (rx_fifo.empty()) {      /* 接收fifo空时读失败，中断置位 RX_FIFO_UDR */
            load(MBOX_INT_PEND, 4, (uint8_t*)&val32);
            *(uint32_t *)(reg_base+MBOX_INT_PEND) = val32 | MBOX_INT_RX_FIFO_UDR;
            irq_update();
            break;
        }

        val64 = rx_fifo.front();
        if (MBOX_RX_CFG_DATA+4 == addr) {
            val64 = val64 >> 32;
        }
        if (((8==len) && (MBOX_RX_CFG_DATA==addr)) || (MBOX_RX_CFG_DATA+4==addr)) {
            rx_fifo.pop();
        }
        memcpy(bytes, &val64, len);
        break;
    default:
        memcpy(bytes, reg_base+addr, len);
        break;
    }

    return true;
}

bool mbox_device_t::store(reg_t addr, size_t len, const uint8_t* bytes)
{
    uint32_t val32 = 0;
    uint64_t txcfg = 0;
    uint64_t txdat = 0;
    uint32_t irq_pend = 0;

    if (unlikely(!bytes || ((size()<addr+len)) || ((4!=len) && (8!=len)))) {
        std::cout << "mbox: unsupported store register offset: " << hex << addr
            << " len: " << hex << len << std::endl;
        return false;
    }

    switch(addr) {
    case MBOX_TX_CFG:
    case MBOX_TX_CFG+4:
        memcpy(reg_base+addr, bytes, len);
        break;
    case MBOX_TX_DATA:
    case MBOX_TX_DATA+4:
        memcpy(reg_base+addr, bytes, len);
        if (((MBOX_TX_DATA==addr)&&(8==len)) || (MBOX_TX_DATA+4==addr)&&(4==len)) {
            load(MBOX_TX_CFG, 8, (uint8_t*)&txcfg);
            load(MBOX_TX_DATA, 8, (uint8_t*)&txdat);
            send_msg(txcfg, txdat);
        }
        break;
    case MBOX_RX_CFG_DATA:
    case MBOX_RX_CFG_DATA+4:
        memcpy(reg_base+addr, bytes, len);
        if (((8==len) && (MBOX_RX_CFG_DATA==addr)) || (MBOX_RX_CFG_DATA+4==addr)) {
            uint64_t rx = 0;
            uint32_t int_pend_val = 0;

            rx = *(uint64_t*)(reg_base+MBOX_RX_CFG_DATA);
            rx_fifo.push(rx);
            if (0 == rx_fifo.size()%2) {
                load(MBOX_INT_PEND, 4, (uint8_t*)(&int_pend_val));
                int_pend_val |= MBOX_INT_RX_VALID;
                *(uint32_t*)(reg_base+MBOX_INT_PEND) = int_pend_val;
                irq_update();
            }
        }
        break;
    case MBOX_INT_PEND:     /* R/W1TC */
        memcpy(&val32, bytes, 4);
        load(MBOX_INT_PEND, 4, (uint8_t *)(&irq_pend));
        if (0 != irq_pend) {
            irq_pend &= ~val32;
            *(uint32_t *)(reg_base+addr) = irq_pend;
            if (0 == irq_pend) {
                irq_generate(false);
            }
        }
        break;
    case MBOX_INT_MASK:     /* 0是中断使能 */
        memcpy(reg_base+addr, bytes, 4);
        irq_update();
        break;
    case MBOX_STATUS:       /* RO */
        return false;
    default:
        memcpy(reg_base+addr, bytes, len);
        break;
    }

    return true;
}

int mbox_device_t::send_msg(uint64_t txcfg, uint64_t txdat)
{
    int ret = 0;
    int bank_id = 0;
    int idx_in_bank = 0;
    uint8_t func = 0;
    uint8_t dst_id = 0;

    processor_t *proc = nullptr;

    dst_id = (txcfg >> 8) & 0xff;
    if (dst_id & (1<<7)) {   /* mgmt, pcie and a53 */
        func = dst_id & 0x1f;
        switch(func) {
        case 0x10:      /* pcie_mbox pf */
            sim->mmio_store(PCIE_MBOX_LOC_PF_BASE+MBOX_RX_CFG_DATA, 8, (uint8_t *)&txcfg);
            sim->mmio_store(PCIE_MBOX_LOC_PF_BASE+MBOX_RX_CFG_DATA, 8, (uint8_t *)&txdat);
            break;
        case 0x11:      /* p2ap_mbox */
            sim->mmio_store(P2AP_MBOX_LOC_BASE+MBOX_RX_CFG_DATA, 8, (uint8_t *)&txcfg);
            sim->mmio_store(P2AP_MBOX_LOC_BASE+MBOX_RX_CFG_DATA, 8, (uint8_t *)&txdat);
            break;
        case 0x12:      /* n2ap_mbox */
            sim->mmio_store(N2AP_MBOX_LOC_BASE+MBOX_RX_CFG_DATA, 8, (uint8_t *)&txcfg);
            sim->mmio_store(N2AP_MBOX_LOC_BASE+MBOX_RX_CFG_DATA, 8, (uint8_t *)&txdat);
            break;
        default:
            printf("mbox: unsupported dst_id func 0x%x \r\n", func);
            ret = -1;
            break;
        }
    } else {        /* npc */
        bank_id = (dst_id >> 3) & 0x03;
        idx_in_bank = dst_id & 0x07;

        proc = sim->get_core_by_idxinsim(sim->coreid_to_idxinsim(dst_id));
        if (proc) {
            sim->npc_mmio_store(MBOX_START+MBOX_RX_CFG_DATA, 8, (uint8_t *)&txcfg, bank_id, idx_in_bank);
            sim->npc_mmio_store(MBOX_START+MBOX_RX_CFG_DATA, 8, (uint8_t *)&txdat, bank_id, idx_in_bank);
        } else {
            printf("mbox send_msg error, core %d not found \r\n", dst_id);
            ret = -2;
        }
    }

    return ret;
}

void mbox_device_t::irq_update(void)
{
    uint32_t reg_int_pend = 0;
    uint32_t reg_int_mask = 0;

    load(MBOX_INT_PEND, 4, (uint8_t *)(&reg_int_pend));
    load(MBOX_INT_MASK, 4, (uint8_t *)(&reg_int_mask));
    reg_int_mask = (~reg_int_mask) & MBOX_INT_ALL;
    if (reg_int_pend & reg_int_mask) {
        irq_generate(true);
    }
}

void mbox_device_t::irq_generate(bool dir)
{
    printf("mbox_device_t::irq_generate \r\n");
}

np_mbox_t::np_mbox_t(simif_t *simif, misc_device_t *misc_dev) : 
    sim(simif), misc(misc_dev), mbox_device_t(simif)
{
    reset();
}

np_mbox_t::~np_mbox_t()
{
}

void np_mbox_t::reset(void)
{
    uint32_t val = 0;

    while (!mbox_device_t::rx_fifo.empty())
        mbox_device_t::rx_fifo.pop();

    memset(reg_base, 0, size());

    val = (1<<5) | (1<<6) | (0xf<<8) | (1<<13) | (1<<21);
    *(uint32_t*)(reg_base+MBOX_STATUS) = val;

    val = ~((uint32_t)0);
    *(uint32_t*)(reg_base+MBOX_INT_MASK) = val;
}

void np_mbox_t::irq_generate(bool dir)
{
    if (misc) {
        misc->set_mcu_irq_status(MCU_IRQ_STATUS_BIT_NPC_MBOX_IRQ, dir);
    }
    if (dir) {
        printf("np_mbox_t::irq_generate \r\n");
    }
}

pcie_mbox_t::pcie_mbox_t(simif_t *simif, pcie_driver_t *pcie_driver) : 
    sim(simif), pcie(pcie_driver), mbox_device_t(simif)
{
    reset();
}

pcie_mbox_t::~pcie_mbox_t()
{
}

void pcie_mbox_t::reset(void)
{
    uint32_t val = 0;

    while (!mbox_device_t::rx_fifo.empty())
        mbox_device_t::rx_fifo.pop();

    memset(reg_base, 0, size());

    val = (1<<5) | (1<<6) | (0xf<<8) | (1<<13) | (1<<21);
    *(uint32_t*)(reg_base+MBOX_STATUS) = val;

    val = ~((uint32_t)0);
    *(uint32_t*)(reg_base+MBOX_INT_MASK) = val;
}

void pcie_mbox_t::irq_generate(bool dir)
{
    uint64_t txcfg = 0;
    uint64_t txdat = 0;
    if (pcie) {
        /**
         * 这部分逻辑还需要和驱动联调
         * 1. 收到消息如何通知驱动
         * 2. 驱动如何获取数据，驱动读寄存器还是spike发中断时携带消息
         * 3. 驱动是否会配置masks寄存器，不配置寄存器无法产生中断
         */
        command_head_t cmd;
        cmd.code = CODE_INTERRUPT;
        cmd.addr = MBOX_RX_CFG_DATA;
        cmd.len = 16;

        load(MBOX_RX_CFG_DATA, 8, (uint8_t *)&txcfg);
        load(MBOX_RX_CFG_DATA, 8, (uint8_t *)&txdat);
        memcpy(cmd.data, (uint8_t *)&txcfg, 8);
        memcpy(&(cmd.data[8]), (uint8_t *)&txdat, 8);
        
        pcie->send((const uint8_t *)&cmd, PCIE_COMMAND_SEND_SIZE(cmd));
    }
    if (dir) {
        printf("pcie_mbox_t::irq_generate \r\n");
    }
}