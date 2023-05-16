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
#include "hwsync.h"

sys_apb_decoder_t::sys_apb_decoder_t(simif_t* sim,uint64_t base, uint8_t *reg_ptr) 
    : base(base), reg_base(reg_ptr)
{
    int i = 0;
    this->sim = sim;
    uint32_t val32 = 0;
    uint32_t tmp = 0;

    /* CHIP_NAME_ADDR，表征仿真平台包括哪些资源 */
    val32 = 0;
    for (i = 0 ; i < int(sim->nbanks()) ; i++) {
        tmp = 0x03;     /* 支持该bank, 支持ddr */
        switch(sim->nprocs()/sim->nbanks()) {   /* 核心数 */
        case 2: tmp |= 1<<2; break;
        case 4: tmp |= 2<<2; break;
        case 8: tmp |= 3<<2; break;
        case 1:
        default:
            tmp |= 0<<2; break;
        }
        tmp = tmp << (i*4);
        val32 |= tmp;
    }
    store(DECODER_SOC_CHIP_NAME_ADDR, 4, (uint8_t *)&val32);

    val32 = 0x20200102;
    store(DECODER_SOC_CHIP_VERSION_ADDR, 4, (uint8_t *)&val32);

    val32 = 0x00;
    store(DECODER_SOC_DIE_SEL_ADDR, 4, (uint8_t *)&val32);
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

    switch(addr) {
    case DECODER_BANK_NPC_MCU_RESET_ADDR_SET_ADDR:
    case DECODER_BANK_NPC_MCU_RESET_ADDR_CLR_ADDR:
    case DECODER_SOC_CHIP_NAME_ADDR:
    case DECODER_SOC_CHIP_VERSION_ADDR:
    case DECODER_SOC_DIE_SEL_ADDR:
        memcpy(bytes, (char *)reg_base + addr, len);
        break;
    case DECODER_SAFE_RESET_REQ_SET_ADDR:
        memcpy(bytes, (char *)reg_base + addr, len);
        break;
    default:
        printf("sys_apb_decoder r 0x%x 0x%lx unsupport \r\n", *(uint32_t*)bytes, addr+base);
        throw trap_load_access_fault(false, addr, 0, 0);
        break;
    }

    return true;
}

bool sys_apb_decoder_t::store(reg_t addr, size_t len, const uint8_t* bytes)
{
    if ((nullptr==reg_base) || (nullptr==bytes) || (addr+len>=size())) {
        return false;
    }

    switch(addr) {
    case DECODER_BANK_NPC_MCU_RESET_ADDR_SET_ADDR:
        this->reset_req(this->sim, bytes);
        break;
    case DECODER_BANK_NPC_MCU_RESET_ADDR_CLR_ADDR:
        this->reset_clr(this->sim, bytes);
        break;
    case DECODER_SAFE_RESET_REQ_SET_ADDR:
        this->safereset_req(this->sim, bytes);
        memcpy((char *)reg_base + addr, bytes, len);
        break;
    case DECODER_SAFE_RESET_REQ_CLR_ADDR:
        this->safereset_clr(this->sim, bytes);
        memcpy((char *)reg_base + addr, bytes, len);
        break;
    case DECODER_SOC_CHIP_NAME_ADDR:
    case DECODER_SOC_CHIP_VERSION_ADDR:
    case DECODER_SOC_DIE_SEL_ADDR:
        memcpy((char *)reg_base + addr, bytes, len);
        break;
    default:
        printf("sys_apb_decoder w 0x%x 0x%lx unsupport \r\n", *(uint32_t*)bytes, addr+base);
        throw trap_store_access_fault(false, addr, 0, 0);
        break;
    }

    return true;
}

void sys_apb_decoder_t::npc_reset_req(simif_t *sim, int processor_id)
{
    sim->get_core_by_idxinsim(processor_id)->reset_req();
}

void sys_apb_decoder_t::npc_safereset_clr(simif_t *sim, int processor_id)
{
    sim->get_core_by_idxinsim(processor_id)->safereset_clr();
    sim->hart_core_reset(processor_id);
}

void sys_apb_decoder_t::safereset_req(simif_t *sim, const uint8_t *bytes)
{
    int proc_id = -1;

    for(int i = 0 ;i < 32;i ++)
    {
        if(getBitValue(*(uint32_t*) (bytes),i) == 1 && this->position == direction::WEST
        && (i/8 == 1 || i/8 == 3)) {
            proc_id = (i/16)*2*8+(i%8);
            /* 硬件pld清除位于bank_misc */
            sim->npc_pld_clr(proc_id);
            this->npc_reset_req(this->sim, proc_id);
        }
        if(getBitValue(*(uint32_t*) (bytes),i) == 1 && this->position == direction::EAST
        && (i/8 == 1 || i/8 == 3)) {
            proc_id = 8+(i/16)*2*8+(i%8);
            sim->npc_pld_clr(proc_id);
            this->npc_reset_req(this->sim, proc_id);
        }
    }

    if (-1 == proc_id) {
        return ;
    }

    int group_id = sim->get_groupID_from_coreID(proc_id);
    if (-1 == group_id) {
        return ;
    }

    /* 清除npc所在sync grp的状态 */
    int data = (1<<31) + (1<<group_id);
    sim->mmio_store(HWSYNC_START + HS_SW_SYNC_REQ_CLR_OFFSET, 4, (uint8_t*)&data);
}

void sys_apb_decoder_t::safereset_clr(simif_t *sim, const uint8_t *bytes)
{
    int proc_id = 0;

    for(int i = 0 ;i < 32;i ++)
    {
        if(getBitValue(*(uint32_t*) (bytes),i) == 1 && this->position == direction::WEST
        && (i/8 == 1 || i/8 == 3))  {

            proc_id = (i/16)*2*8+(i%8);
            this->npc_safereset_clr(this->sim, proc_id);
        }
        if(getBitValue(*(uint32_t*) (bytes),i) == 1 && this->position == direction::EAST
        && (i/8 == 1 || i/8 == 3)) {

            proc_id = 8+(i/16)*2*8+(i%8);
            this->npc_safereset_clr(this->sim,proc_id);
        }
    }
}

void sys_apb_decoder_t::reset_req(simif_t *sim, const uint8_t *flag)
{
    if(this->position == direction::WEST)
    {
        uint8_t bank0_flag = *(uint8_t*) (flag);
        uint8_t bank2_flag = *(uint8_t*) (flag+1);
        for(int i = 0; i < 8; i++)
        {
            bool perform = getBitValue(bank0_flag, i);
            if(perform)
                npc_reset_req(sim,i);
        }
        for(int i = 0; i < 8; i++)
        {
            bool perform = getBitValue(bank2_flag, i);
            if(perform)
                npc_reset_req(sim,2 * 8 + i);
        }
    }
    else
    {
        uint8_t bank1_flag = *(uint8_t*) (flag);
        uint8_t bank3_flag = *(uint8_t*) (flag+1);
        for(int i = 0; i < 8; i++)
        {
            bool perform = getBitValue(bank1_flag, i);
            if(perform)
                npc_reset_req(sim,8 + i);
        }
        for(int i = 0; i < 8; i++)
        {
            bool perform = getBitValue(bank3_flag, i);
            if(perform)
                npc_reset_req(sim,3 * 8 + i);
        }
    }
}

void sys_apb_decoder_t::reset_clr(simif_t *sim,const uint8_t *flag)
{
    if(this->position == direction::WEST)
    {
        uint8_t bank0_flag = *(uint8_t*) (flag);
        uint8_t bank2_flag = *(uint8_t*) (flag+1);
        for(int i = 0; i < 8; i++)
        {
            bool perform = getBitValue(bank0_flag, i);
            if(perform)
                npc_safereset_clr(sim,i);
        }
        for(int i = 0; i < 8; i++)
        {
            bool perform = getBitValue(bank2_flag, i);
            if(perform)
                npc_safereset_clr(sim,2 * 8 + i);
        }
    }
    else
    {
        uint8_t bank1_flag = *(uint8_t*) (flag);
        uint8_t bank3_flag = *(uint8_t*) (flag+1);
        for(int i = 0; i < 8; i++)
        {
            bool perform = getBitValue(bank1_flag, i);
            if(perform)
                npc_safereset_clr(sim,8 + i);
        }
        for(int i = 0; i < 8; i++)
        {
            bool perform = getBitValue(bank3_flag, i);
            if(perform)
                npc_safereset_clr(sim,3 * 8 + i);
        }
    }
}

sys_irq_t::sys_irq_t(simif_t *sim, apifc_t *apifc, uint8_t *reg_ptr) :
        sim(sim), apifc(apifc), reg_base(reg_ptr)
{
    uint32_t val32 = 0;
    
    val32 = 0xffffffff;
    store(SYSIRQ_CPU_IRQ_MASK_ADDR0, 4, (uint8_t*)(&val32));
    store(SYSIRQ_CPU_IRQ_MASK_ADDR1, 4, (uint8_t*)(&val32));
    store(SYSIRQ_CPU_IRQ_MASK_ADDR2, 4, (uint8_t*)(&val32));
    store(SYSIRQ_PCIE_IRQ_MASK_ADDR0, 4, (uint8_t*)(&val32));
}

sys_irq_t::~sys_irq_t()
{
    reg_base = nullptr;
}

bool sys_irq_t::load(reg_t addr, size_t len, uint8_t* bytes)
{
    if ((nullptr==reg_base) || (nullptr==bytes) || (addr+len>=size())) {
        return false;
    }

    memcpy(bytes, (char *)reg_base + addr, len);
    return true;
}

bool sys_irq_t::store(reg_t addr, size_t len, const uint8_t* bytes)
{
    int i = 0;
    uint32_t val32 = 0;
    uint32_t mask = 0;
    uint32_t sts_old = 0;
    uint32_t sts_new = 0;
    int irq = 0;

    if ((nullptr==reg_base) || (nullptr==bytes) || (addr+len>=size())) {
        return false;
    }
    
    switch(addr) {
    /* npcx-->A53中断mask, 0b1屏蔽, 0b0不屏蔽 */
    case SYSIRQ_CPU_IRQ_MASK_ADDR0:     /* 0x00 */
    /* A53中断mask, [29]:p2ap_mbox， [30]:n2ap_mbox， 0b0中断可以发送到A53 */
    case SYSIRQ_CPU_IRQ_MASK_ADDR1:     /* 0x04 */
    case SYSIRQ_CPU_IRQ_MASK_ADDR2:     /* 0x08 */
    case SYSIRQ_CPU_IRQ_MASK_ADDR3:     /* 0x0c 没有添加功能, 只支持读写 */
    case SYSIRQ_PCIE_IRQ_MASK_ADDR0:
        memcpy((char *)reg_base + addr, bytes, len);
        break;
    case SYSIRQ_BANK_SW_IRQ_IN_SET_ADDR:   /* 0b1 产生中断 */
        memcpy((char *)reg_base + addr, bytes, len);
        val32 = *(uint32_t *)(reg_base+addr);
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
        *(uint32_t *)(reg_base+addr) = 0;
        break;
    case SYSIRQ_BANK_NPC_SW_IRQ_LATCH_CLR_ADDR:     /* 写1清除STS对应位 */
        val32 = *(uint32_t *)bytes;
        sts_old = *(uint32_t *)(reg_base+SYSIRQ_TO_CPU_NPC_SW_IRQ_OUT_STS_ADDR);
        sts_new = sts_old & (~val32);
        *(uint32_t *)(reg_base+SYSIRQ_TO_CPU_NPC_SW_IRQ_OUT_STS_ADDR) = sts_new;
        if ((sts_new ^ sts_old) & 0x000000ff) {
            irq = A53_NPC_CLUSTER0_IRQ;
        } else if ((sts_new ^ sts_old) & 0x0000ff00) {
            irq = A53_NPC_CLUSTER1_IRQ;
        } else if ((sts_new ^ sts_old) & 0x00ff0000) {
            irq = A53_NPC_CLUSTER2_IRQ;
        } else if ((sts_new ^ sts_old) & 0xff000000) {
            irq = A53_NPC_CLUSTER3_IRQ;
        }
    #if 0   /* spike发清中断太慢了, 在qemu里发起清中断不必等spike发消息 */
        if (0 != irq) {
            apifc->generate_irq_to_a53(irq, false);
        }
    #endif

        sts_old = *(uint32_t *)(reg_base+SYSIRQ_TO_PCIE_NPC_SW_IRQ_OUT_STS_ADDR);
        sts_new = sts_old & (~val32);
        *(uint32_t *)(reg_base+SYSIRQ_TO_PCIE_NPC_SW_IRQ_OUT_STS_ADDR) = sts_new;
        /* 清除pcie中断 */
        break;
    case SYSIRQ_TO_CPU_NPC_SW_IRQ_OUT_STS_ADDR:
    /**
     * 已弃用， 在专用接口产生中断
     * 两个STS寄存器对用户是RO的， 此处由misc发起写操作
     * 对应mask为0， 修改STS同时发送中断
     * 对应mask为1， 不会修改STS
     */
        irq = 0;
        mask = *(uint32_t *)(reg_base+SYSIRQ_CPU_IRQ_MASK_ADDR0);
        sts_old = *(uint32_t *)(reg_base+addr);
        sts_new = *(uint32_t *)bytes;
        sts_new = sts_old | (sts_new & (~mask));
        if ((sts_new ^ sts_old) & 0x000000ff) {
            irq = A53_NPC_CLUSTER0_IRQ;
        } else if ((sts_new ^ sts_old) & 0x0000ff00) {
            irq = A53_NPC_CLUSTER1_IRQ;
        } else if ((sts_new ^ sts_old) & 0x00ff0000) {
            irq = A53_NPC_CLUSTER2_IRQ;
        } else if ((sts_new ^ sts_old) & 0xff000000) {
            irq = A53_NPC_CLUSTER3_IRQ;
        }
        if (0 != irq) {
            memcpy(reg_base+addr, (uint8_t*)&sts_new, 4);
            apifc->generate_irq_to_a53(irq, true);
        }
        break;
    case SYSIRQ_TO_PCIE_NPC_SW_IRQ_OUT_STS_ADDR:
        irq = 0;
        mask = *(uint32_t *)(reg_base+SYSIRQ_PCIE_IRQ_MASK_ADDR0);
        sts_old = *(uint32_t *)(reg_base+addr);
        sts_new = *(uint32_t *)bytes;
        sts_new = sts_old | (sts_new & (~mask));
        if (sts_new ^ sts_old) {
            memcpy(reg_base+addr, (uint8_t*)&sts_new, 4);
            /* 向pcie发送中断 */
        }
        break;
    case SYSIRQ_INGRESS_IRQ_STS_ADDR2:
        irq = 0;
        mask = *(uint32_t *)(reg_base+SYSIRQ_CPU_IRQ_MASK_ADDR2);
        sts_old = *(uint32_t *)(reg_base+addr);
        sts_new = *(uint32_t *)bytes;
        // sts_new = sts_old | (sts_new & (~mask));
        sts_new = (sts_new & (~mask));
        memcpy(reg_base+addr, (uint8_t*)&sts_new, 4);
        for (i = 0 ; i < PCIE_DMA_CH_TOTAL ; i++) {
            if ((sts_new ^ sts_old) & (1<<STS_ADDR2_PCIE_DMA_BIT_CH(i))) {
                irq = A53_PCIE_DMA_IRQ_CH(i);
                if (sts_new & (1<<STS_ADDR2_PCIE_DMA_BIT_CH(i))) {
                    apifc->generate_irq_to_a53(irq, true);  
                } else {
                    /* pcie_dma 清中断在qemu实现 */
                    /* apifc->generate_irq_to_a53(irq, false); */
                    ;
                }
            }
        }
        break;
    /* die1的功能逻辑后续按需补充 */
    case SYSIRQ_TO_CPU_REMOTE_DIR_SW_IRQ_LATCH_CLR_ADDR:
    case SYSIRQ_TO_CPU_REMOTE_DIE_NPC_IRQ_MASK_ADDR:
    case SYSIRQ_TO_CPU_REMOTE_DIE_NPC_IRQ_STS_ADDR:
        memcpy((char *)reg_base + addr, bytes, len);
        break;
    default:
        printf("sys_irq store addr :0x%lx len %ld no support \r\n",addr, len);
        throw trap_store_access_fault(false, addr, 0, 0);
        return false;
    }
    return true;
}

bool sys_irq_t::is_irq_ena_p2apmbox(void)
{
    uint32_t val32 = 0;

    load(SYSIRQ_CPU_IRQ_MASK_ADDR1, 4, (uint8_t*)&val32);

    return ((val32>>29)&0x01) ? false : true; 
}

bool sys_irq_t::is_irq_ena_n2apmbox(void)
{
    uint32_t val32 = 0;

    load(SYSIRQ_CPU_IRQ_MASK_ADDR1, 4, (uint8_t*)&val32);

    return ((val32>>30)&0x01) ? false : true; 
}

void sys_irq_t::set_to_cpu_irq_out(int proc_id)
{
    int irq = 0;
    uint32_t sts = 0;
    uint32_t mask = 0;
    uint32_t val32 = 0;

    /* mask 0b0 不屏蔽, mask 0b1屏蔽 */
    mask = *(uint32_t *)(reg_base+SYSIRQ_CPU_IRQ_MASK_ADDR0);
    
    val32 = 1 << (proc_id & 0x1f);
    if (val32 & (~mask)) {
        if (val32 & 0x000000ff) {
            irq = A53_NPC_CLUSTER0_IRQ;
        } else if (val32 & 0x0000ff00) {
            irq = A53_NPC_CLUSTER1_IRQ;
        } else if (val32 & 0x00ff0000) {
            irq = A53_NPC_CLUSTER2_IRQ;
        } else if (val32 & 0xff000000) {
            irq = A53_NPC_CLUSTER3_IRQ;
        }

        *(uint32_t *)(reg_base+SYSIRQ_TO_CPU_NPC_SW_IRQ_OUT_STS_ADDR) |= val32;
        if (0 != irq) {
            apifc->generate_irq_to_a53(irq, true);
        }
    }
}

soc_apb_t::soc_apb_t(simif_t *sim, apifc_t *apifc) : sim(sim), apifc(apifc)
{
    memset(reg_base, 0, sizeof(reg_base));

    /* 0xd3e10000 */
    sys_irq = new sys_irq_t(sim, apifc, &(reg_base[SYSIRQ_BASE-SOC_APB_BASE]));

    /* 0xd3e00000 */
    sys_apb_decoder_west = new sys_apb_decoder_t(sim,SYS_APB_DECODER_WEST_BASE,
                &(reg_base[SYS_APB_DECODER_WEST_BASE-SOC_APB_BASE]));
    sys_apb_decoder_west->set_position(direction::WEST);
    /* 0xd3e60000 */
    sys_apb_decoder_east = new sys_apb_decoder_t(sim,SYS_APB_DECODER_EAST_BASE,
                &(reg_base[SYS_APB_DECODER_EAST_BASE-SOC_APB_BASE]));
    sys_apb_decoder_east->set_position(direction::EAST);
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

    switch((addr+SOC_APB_BASE)&0xffff0000) {
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

    switch((addr+SOC_APB_BASE)&0xffff0000) {
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
        printf("soc_apb_t unsupport addr w 0x%x 0x%lx \r\n", *(uint32_t*)bytes, addr+SOC_APB_BASE);
        throw trap_store_access_fault(false, addr, 0, 0);
    }

    return true;
}
