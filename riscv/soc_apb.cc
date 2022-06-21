#include <iostream>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "simif.h"
#include "apifc.h"
#include "devices.h"
#include "processor.h"
#include "pcie_driver.h"
#include "soc_apb.h"

sys_apb_decoder_t::sys_apb_decoder_t(uint64_t base, uint8_t *reg_ptr) 
    : base(base), reg_base(reg_ptr)
{
    uint32_t val32 = 0;

    val32 = 0x20200102;
    store(SOC_CHIP_VERSION_ADDR, 4, (uint8_t *)&val32);

    val32 = 0x00;
    store(SOC_DIE_SEL_ADDR, 4, (uint8_t *)&val32);
}

sys_apb_decoder_t::~sys_apb_decoder_t()
{
    reg_base = nullptr;
}

bool sys_apb_decoder_t::load(reg_t addr, size_t len, uint8_t* bytes)
{
    if ((nullptr==reg_base) || (nullptr==bytes) || (addr+len>=size())) {
        return false;
    }

    memcpy(bytes, (char *)reg_base + addr, len);
    printf("sys_apb_decoder r 0x%x 0x%lx \r\n", *(uint32_t*)bytes, addr+base);

    return true;
}

bool sys_apb_decoder_t::store(reg_t addr, size_t len, const uint8_t* bytes)
{
    int i = 0;
    uint32_t val32 = 0;

    if ((nullptr==reg_base) || (nullptr==bytes) || (addr+len>=size())) {
        return false;
    }

    memcpy((char *)reg_base + addr, bytes, len);
    printf("sys_apb_decoder w 0x%x 0x%lx \r\n", *(uint32_t*)bytes, addr+base);

    return true;
}

sys_irq_t::sys_irq_t(simif_t *sim, apifc_t *apifc, uint8_t *reg_ptr) :
        sim(sim), apifc(apifc), reg_base(reg_ptr)
{
    uint32_t val32 = 0;
    
    val32 = 0xffffffff;
    store(CPU_IRQ_MASK_ADDR0, 4, (uint8_t*)(&val32));
    store(CPU_IRQ_MASK_ADDR1, 4, (uint8_t*)(&val32));
}

sys_irq_t::~sys_irq_t()
{
    reg_base = nullptr;
}

bool sys_irq_t::load(reg_t addr, size_t len, uint8_t* bytes)
{
    if ((nullptr==reg_base) || (nullptr==bytes)) {
        return false;
    }
    if (/* (addr<0) ||  */(addr+len>=size())) {
        return false;
    }
    memcpy(bytes, (char *)reg_base + addr, len);
    return true;
}

bool sys_irq_t::store(reg_t addr, size_t len, const uint8_t* bytes)
{
    int i = 0;
    uint32_t val32 = 0;

    if ((nullptr==reg_base) || (nullptr==bytes)) {
        return false;
    }
    if (/* (addr<0) ||  */(addr+len>=size())) {
        return false;
    }
    memcpy((char *)reg_base + addr, bytes, len);

    switch(addr) {
    case CPU_IRQ_MASK_ADDR0:        /* 0b1 屏蔽中断, 0b0不屏蔽 */
    case CPU_IRQ_MASK_ADDR1:
        break;
    case BANK_SW_IRQ_IN_SET_ADDR:   /* 0b1 产生中断 */
        val32 = *(uint32_t *)(reg_base+BANK_SW_IRQ_IN_SET_ADDR);
        for (i = 0 ; i < 32 ; i++) {
            if (val32 & (1<<i)) {
                processor_t *proc = nullptr;
                proc = sim->get_core_by_idxinsim(sim->coreid_to_idxinsim(i));
                if (proc) {
                    proc->misc_dev->set_mcu_irq_status(MCU_IRQ_STATUS_BIT_NPC_IN_IRQ, 1);
                } else {
                    printf("sysirq proc %d not found \r\n",i);
                }
            }
        }
        *(uint32_t *)(reg_base+BANK_SW_IRQ_IN_SET_ADDR) = 0;
        break;
    case TO_CPU_NPC_SW_IRQ_OUT_STS_ADDR:    /* RO */
    default:
        printf("store addr sys_irq:0x%lx no support \r\n",addr);
        throw trap_store_access_fault(false, addr, 0, 0);
        return false;
    }
    return true;
}

soc_apb_t::soc_apb_t(simif_t *sim, apifc_t *apifc) : sim(sim), apifc(apifc)
{
    memset(reg_base, 0, sizeof(reg_base));

    /* 0xd3e10000 */
    sys_irq = new sys_irq_t(sim, apifc, &(reg_base[SYSIRQ_BASE-SOC_APB_BASE]));

    /* 0xd3e00000 */
    sys_apb_decoder_west = new sys_apb_decoder_t(SYS_APB_DECODER_WEST_BASE,
                &(reg_base[SYS_APB_DECODER_WEST_BASE-SOC_APB_BASE]));
    /* 0xd3e60000 */
    sys_apb_decoder_east = new sys_apb_decoder_t(SYS_APB_DECODER_EAST_BASE,
                &(reg_base[SYS_APB_DECODER_EAST_BASE-SOC_APB_BASE]));
}

soc_apb_t::~soc_apb_t()
{
    free(sys_irq);
    free(sys_apb_decoder_west);
    free(sys_apb_decoder_east);
    sys_irq = nullptr;
    sys_apb_decoder_west = nullptr;
    sys_apb_decoder_east = nullptr;
}

bool soc_apb_t::load(reg_t addr, size_t len, uint8_t* bytes)
{
    if ((nullptr==reg_base) || (nullptr==bytes) || (addr+len>=size())) {
        return false;
    }

    switch((addr+SOC_APB_BASE)&0xfff80000) {
    case SYS_APB_DECODER_WEST_BASE:
        sys_apb_decoder_west->load(addr&(SYS_APB_DECODER_SIZE-1), len, bytes);
        break;
    case SYS_APB_DECODER_EAST_BASE:
        sys_apb_decoder_east->load(addr&(SYS_APB_DECODER_SIZE-1), len, bytes);
        break;
    case SYSIRQ_BASE:
        sys_irq->load(addr&(SYSIRQ_SIZE-1), len, bytes);
        break;
    default:
        memcpy(bytes, (char *)reg_base + addr, len);
        printf("soc_apb_t r 0x%x 0x%lx \r\n", *(uint32_t*)bytes, addr+SOC_APB_BASE);
    }
    
    return true;
}

bool soc_apb_t::store(reg_t addr, size_t len, const uint8_t* bytes)
{
    if ((nullptr==reg_base) || (nullptr==bytes) || (addr+len>=size())) {
        return false;
    }

    switch((addr+SOC_APB_BASE)&0xfff80000) {
    case SYS_APB_DECODER_WEST_BASE:
        sys_apb_decoder_west->store(addr&(SYS_APB_DECODER_SIZE-1), len, bytes);
        break;
    case SYS_APB_DECODER_EAST_BASE:
        sys_apb_decoder_east->store(addr&(SYS_APB_DECODER_SIZE-1), len, bytes);
        break;
    case SYSIRQ_BASE:
        sys_irq->store(addr&(SYSIRQ_SIZE-1), len, bytes);
        break;
    default:
        memcpy((char *)reg_base + addr, bytes, len);
        printf("soc_apb_t w 0x%x 0x%lx \r\n", *(uint32_t*)bytes, addr+SOC_APB_BASE);
    }

    return true;
}
