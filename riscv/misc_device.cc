#include <fesvr/htif.h>

#include "devices.h"
#include "processor.h"
#include "soc_apb.h"

#define UART_BASE  0x100
#define EXIT_BASE  0x500
#define DUMP_BASE  0x600

#define DUMP_START_OFFSET   0x0
#define DUMP_ADDR_OFFSET    0x8
#define DUMP_LEN_OFFSET     0x10

#define NCP_INST_STATUS_OFFSET  0x7c

#define MCU_IRQ_STATUS_MASK     0x1ff7
#define MCU_IRQ_ENABLE_MASK     MCU_IRQ_STATUS_MASK
#define MCU_IRQ_CLEAR_MASK      0x1ffe

misc_device_t::misc_device_t(processor_t* proc,  simif_t *sim)
  : buf_len(0x4000), proc(proc), sim(sim), dump_count(0)
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
    switch(addr)
    {
        case NCP_INST_STATUS_OFFSET:
        {
            uint32_t hw_status = proc->get_hwsync_status();
            uint32_t pld_status = proc->get_pld_status();
            uint32_t index = proc->get_bank_id() * 8 + proc->get_idxinbank();
            uint32_t temp1 = (~hw_status) & (1 << index);
            uint32_t temp2 = (~pld_status) & (1 << index);
            uint32_t res = (temp1<<6) + (temp1<<7) + (temp2<<8) + (temp2<<9);
            memcpy(bytes, &res, len);
            break;
        }
        default:
            memcpy(bytes, (uint8_t *)reg_base + addr, len);
    }

    return true;
}

bool misc_device_t::store(reg_t addr, size_t len, const uint8_t* bytes)
{
    if (unlikely(!bytes || addr >= MISC_SIZE)) {
        return false;
    }

    switch(addr) {
    case UART_BASE:     // uart device
        {
        for (size_t index = 0; index < len; index++) {
        if (unlikely('\n' == *bytes)) {
            std::cout << "cpu" << proc->get_id() << ":\t";
            for (int index = 0; index < int(buf.size()); index++)
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
            char *str = nullptr;
            if ((str = proc->get_sim()->addr_to_mem(prefix_addr)) ||
                    (str = proc->get_bank()->bank_addr_to_mem(prefix_addr))) {
                prefix = str;
            }
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
    case NP_IRQ_OUT_CTRL:       /* 写1向 a53/pcie发中断 */
        {
        uint32_t val = *(uint32_t *)bytes;
        memcpy((uint8_t *)reg_base+addr, bytes, len);

        if (1 == (val&0x01)) {
            val = 0;
            sim->mmio_load(SYSIRQ_BASE+SYSIRQ_TO_CPU_NPC_SW_IRQ_OUT_STS_ADDR, 4, (uint8_t *)&val);
            val |= (1<<proc->get_id());
            sim->mmio_store(SYSIRQ_BASE+SYSIRQ_TO_CPU_NPC_SW_IRQ_OUT_STS_ADDR, 4, (uint8_t *)&val);

            val = 0;
            sim->mmio_load(SYSIRQ_BASE+SYSIRQ_TO_PCIE_NPC_SW_IRQ_OUT_STS_ADDR, 4, (uint8_t *)&val);
            val |= (1<<proc->get_id());
            sim->mmio_store(SYSIRQ_BASE+SYSIRQ_TO_PCIE_NPC_SW_IRQ_OUT_STS_ADDR, 4, (uint8_t *)&val);
        }
        }
        break;
    case MCU_IRQ_ENABLE_OFFSET:
        ro_register_write(addr, *(uint32_t*)bytes);
        break;
    /* MCU_IRQ_STATUS_ADDR 是RO的，这里开个例外，允许驱动通过netlink向 NPC_IN_IRQ 写1 */
    case MCU_IRQ_STATUS_OFFSET:
        {
        uint32_t val = *(uint32_t *)bytes;
        if ((4==len) && ((1<<MCU_IRQ_STATUS_BIT_NPC_IN_IRQ)==val)) {
            load(MCU_IRQ_STATUS_OFFSET, 4, (uint8_t *)&val);
            val |= (1<<MCU_IRQ_STATUS_BIT_NPC_IN_IRQ);
            ro_register_write(MCU_IRQ_STATUS_OFFSET, (uint32_t)val);
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
            proc->set_mip_bit(IRQ_M_EXT, 0);    /* MIP_MEIP */
        }
        memset((uint8_t *)reg_base+addr, 0, 4);
        }
        break;
    case MCU_PERF_CNT_CTRL:
        {
        uint32_t val32 = 0;
        memcpy((uint8_t *)reg_base+addr, bytes, len);
        val32 = *(uint32_t *)((uint8_t *)reg_base+addr);
        if ((val32>>2) & 0x01)
            inst_cnt_clear();
        if ((val32>>1) & 0x01)
            inst_cnt_stop();
        if ((val32>>0) & 0x01)
            inst_cnt_start();
        memset((uint8_t *)reg_base+addr, 0, 4);
        }
        break;
    /* RO */
    case NCP_VME_INST_CNT:
    case NCP_MME_INST_CNT:
    case NCP_RVV_INST_CNT:
    case NCP_SYN_INST_CNT:
    case NCP_SYN_DONE_INST_CNT:
    case MTE_PLD_INST_CNT:
    case MTE_PLD_DONE_INST_CNT:
    case MTE_ICMOV_INST_CNT:
    case MTE_MOV_INST_CNT:
    case DMA_INST_CNT:
        return false;
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

void misc_device_t::inst_cnt(insn_bits_t bits)
{
    uint64_t addr = 0;
    uint64_t val64 = 0;

    if (!IS_STC_NPUV2_INST(bits) || !is_inst_start())
        return;
    
    if (IS_PLD_INST(bits)) {
        addr = MTE_PLD_INST_CNT;
    } else if (IS_ICMOV_M_INST(bits)) {
        addr = MTE_ICMOV_INST_CNT;
    } else if (IS_MTE_MOV_INST(bits)) {
        addr = MTE_MOV_INST_CNT;
    } else if (IS_SYNC_INST(bits)) {
        addr = NCP_SYN_INST_CNT;
    } else if (IS_MME_INST(bits)) {
        addr = NCP_MME_INST_CNT;
    } else if (IS_DMAE_INST(bits)) {
        addr = DMA_INST_CNT;
    } else {        /* vme */
        addr = NCP_VME_INST_CNT;
    }
    load(addr, sizeof(val64), (uint8_t*)&val64);
    val64 += 1;
    ro_register_write(addr, val64);
}

void misc_device_t::inst_done_cnt(insn_bits_t bits)
{
    uint64_t addr = 0;
    uint64_t val64 = 0;

    if (!IS_STC_NPUV2_INST(bits) || !is_inst_start())
        return;
    
    if (IS_PLD_INST(bits)) {
        addr = MTE_PLD_DONE_INST_CNT;
    } else if (IS_SYNC_INST(bits)) {
        addr = NCP_SYN_DONE_INST_CNT;
    } else {
        return;
    }
    
    load(addr, sizeof(val64), (uint8_t*)&val64);
    val64 += 1;
    ro_register_write(addr, val64);
}

void misc_device_t::inst_cnt_clear(void)
{
    uint64_t val64 = 0;

    ro_register_write(NCP_VME_INST_CNT, val64);
    ro_register_write(NCP_MME_INST_CNT, val64);
    ro_register_write(NCP_SYN_INST_CNT, val64);
    ro_register_write(NCP_SYN_DONE_INST_CNT, val64);
    ro_register_write(MTE_PLD_INST_CNT, val64);
    ro_register_write(MTE_PLD_DONE_INST_CNT, val64);
    ro_register_write(MTE_ICMOV_INST_CNT, val64);
    ro_register_write(MTE_MOV_INST_CNT, val64);
    ro_register_write(DMA_INST_CNT, val64);
}

bool misc_device_t::ro_register_write(reg_t addr, uint32_t val)
{
    uint32_t reg_status = 0;
    uint32_t reg_enable = 0;

    if (addr > MISC_SIZE-sizeof(val))
        return false;
    
    switch(addr) {
    case MCU_IRQ_STATUS_OFFSET:
    case MCU_IRQ_ENABLE_OFFSET:
        *(uint32_t *)((uint8_t *)reg_base+addr) = val;
        *(uint32_t *)((uint8_t *)reg_base+addr) &= MCU_IRQ_ENABLE_MASK;
        reg_status = *(uint32_t *)((uint8_t *)reg_base+MCU_IRQ_STATUS_OFFSET);
        reg_enable = *(uint32_t *)((uint8_t *)reg_base+MCU_IRQ_ENABLE_OFFSET);
        if (reg_status & reg_enable) {
            proc->set_mip_bit(IRQ_M_EXT, 1);  /* MIP_MEIP */
        }
        /*
        else {
            proc->set_mip_bit(IRQ_M_EXT, 0);
        }
        */
        break;
    default:
        *(uint32_t *)((uint8_t *)reg_base+addr) = val;
        break;
    }
    
    return true;

}

bool misc_device_t::ro_register_write(reg_t addr, uint64_t val)
{
    if (addr > MISC_SIZE-sizeof(val))
        return false;
    
    switch(addr) {
    case NCP_VME_INST_CNT:
    case NCP_MME_INST_CNT:
    case NCP_RVV_INST_CNT:
    case NCP_SYN_INST_CNT:
    case NCP_SYN_DONE_INST_CNT:
    case MTE_PLD_INST_CNT:
    case MTE_PLD_DONE_INST_CNT:
    case MTE_ICMOV_INST_CNT:
    case MTE_MOV_INST_CNT:
    case DMA_INST_CNT:
        *(uint64_t *)((uint8_t *)reg_base+addr) = val;
        break;
    default:
        *(uint64_t *)((uint8_t *)reg_base+addr) = val;
        break;

    }
    
    return true;
}

/* write MCU_IRQ_STATUS_ADDR, 0b1 irq, 0b0 noirq */
void misc_device_t::set_mcu_irq_status(int mcu_irq_status_bit, bool val)
{
    uint32_t irq_status = 0;

    load(MCU_IRQ_STATUS_OFFSET, sizeof(irq_status), (uint8_t *)&irq_status);
    if (val) {
        irq_status |= (1<<mcu_irq_status_bit);
    } else {
        irq_status &= (~(1<<mcu_irq_status_bit));
    }
    irq_status &= MCU_IRQ_STATUS_MASK;
    ro_register_write(MCU_IRQ_STATUS_OFFSET, (uint32_t)irq_status);
}

bank_misc_dev_t::bank_misc_dev_t(simif_t *sim) : sim(sim)
{
}

bank_misc_dev_t::~bank_misc_dev_t()
{
}

bool bank_misc_dev_t::load(reg_t addr, size_t len, uint8_t* bytes)
{
    uint32_t val = 0;
    switch(addr) {
    case 0x310:
        val = 0xff;
        memcpy(bytes, &val, len);
        break;
    default:
        val = 0;
        memcpy(bytes, &val, len);
    }
    return true;
}

bool bank_misc_dev_t::store(reg_t addr, size_t len, const uint8_t* bytes)
{
    return true;
}