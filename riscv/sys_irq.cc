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

sys_irq_t::sys_irq_t(simif_t *sim, apifc_t *apifc) : sim(sim), apifc(apifc)
{
    uint32_t val32 = 0;
    memset(reg_base, 0, sizeof(reg_base));

    val32 = 0xffffffff;
    store(CPU_IRQ_MASK_ADDR0, 4, (uint8_t*)(&val32));
    store(CPU_IRQ_MASK_ADDR1, 4, (uint8_t*)(&val32));
}

sys_irq_t::~sys_irq_t()
{
    ;
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
        printf("store addr sys_irq:0x%x no support \r\n",addr);
        throw trap_store_access_fault(false, addr, 0, 0);
        return false;
    }
    return true;
}

/**
 * 功能: 向 qemu a53发送一个中断
 * 参数: @irq: 中断号 0-1023
 * 参数: @dir: 1产生，0清除
 * 返回: 成功返回0
 */
int sys_irq_t::generate_irq_to_a53(int irq, int dir)
{
    int ret = 0;
    struct command_head_t cmd_data = {};

    /* data填中断号 */
    cmd_data.code = CODE_INTERRUPT;
    cmd_data.addr = 0;
    cmd_data.len = 4;
    if (dir) {
        *((int *)(cmd_data.data)) = cmd_data_irq(irq);
    } else {
        *((int *)(cmd_data.data)) = cmd_data_clear_irq(irq);
    }

    ret = apifc->sqmsg_spike_send(SQ_MTYPE_REQ(CODE_INTERRUPT), &cmd_data);
    return (0 > ret) ? ret : 0;
}
