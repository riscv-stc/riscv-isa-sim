#ifndef __SOC_APB_H__
#define __SOC_APB_H__

#include <cstdlib>
#include <vector>
#include "devices.h"
#include "processor.h"
#include "pcie_driver.h"
#include "mailbox.h"
#include "mbox_device.h"
#include "apifc.h"

#define SOC_APB_BASE    0xd3e00000
#define SOC_APB_SIZE    0x80000

#define SYSIRQ_BASE     0xd3e10000
#define SYSIRQ_SIZE     0x10000

#define SYSIRQ_CPU_IRQ_MASK_ADDR0               0x00    /* 0b1 屏蔽中断, 0b0不屏蔽 */
#define SYSIRQ_CPU_IRQ_MASK_ADDR1               0x04
#define SYSIRQ_PCIE_IRQ_MASK_ADDR0              0x20
#define SYSIRQ_BANK_SW_IRQ_IN_SET_ADDR          0x54    /* 0b1 在NPC产生中断 */
#define SYSIRQ_BANK_NPC_SW_IRQ_LATCH_CLR_ADDR   0x58
#define SYSIRQ_TO_CPU_REMOTE_DIR_SW_IRQ_LATCH_CLR_ADDR      0x7c
#define SYSIRQ_TO_CPU_REMOTE_DIE_NPC_IRQ_MASK_ADDR  0x100
#define SYSIRQ_TO_CPU_REMOTE_DIE_NPC_IRQ_STS_ADDR   0x10c
#define SYSIRQ_TO_CPU_NPC_SW_IRQ_OUT_STS_ADDR       0x114
#define SYSIRQ_TO_PCIE_NPC_SW_IRQ_OUT_STS_ADDR      0x118

#define A53_NPC_CLUSTER0_IRQ         (32+52)
#define A53_NPC_CLUSTER1_IRQ         (32+53)
#define A53_NPC_CLUSTER2_IRQ         (32+54)
#define A53_NPC_CLUSTER3_IRQ         (32+55)

#define SYS_APB_DECODER_WEST_BASE   0xd3e00000
#define SYS_APB_DECODER_EAST_BASE   0xd3e60000
#define SYS_APB_DECODER_SIZE        0x10000

#define DECODER_BANK_NPC_MCU_RESET_ADDR_SET_ADDR     (0x100)
#define DECODER_BANK_NPC_MCU_RESET_ADDR_CLR_ADDR     (0x104)
#define DECODER_SOC_CHIP_VERSION_ADDR 	      (0x1004)
#define DECODER_SOC_DIE_SEL_ADDR              (0x19b4)

#define getBitValue(var, idx) ((var >> (idx)) & 1)
#define setBitValue(var, idx, value) (var) = (value > 0) ? ((var) | (1 << (idx))) : ((var) & ~(1 << (idx)))

class apifc_t;

class sys_apb_decoder_t : public abstract_device_t {
public:
    sys_apb_decoder_t(uint64_t base, uint8_t *reg_ptr);
    ~sys_apb_decoder_t();
    bool load(reg_t addr, size_t len, uint8_t* bytes);
    bool store(reg_t addr, size_t len, const uint8_t* bytes);
    size_t size() { return SYS_APB_DECODER_SIZE; }
    bool in_state_reset(size_t relative_bankid,size_t idxinbank);
    bool in_state_disarm_reset(size_t relative_bankid,size_t idxinbank);
    
private:
    uint64_t base = 0x00;
    uint8_t *reg_base = nullptr;
};


class sys_irq_t : public abstract_device_t {
public:
    sys_irq_t(simif_t *sim, apifc_t *apifc, uint8_t *reg_ptr);
    ~sys_irq_t();
    bool load(reg_t addr, size_t len, uint8_t* bytes);
    bool store(reg_t addr, size_t len, const uint8_t* bytes);
    size_t size() { return SYSIRQ_SIZE; }

    bool is_irq_ena_p2apmbox(void);
    bool is_irq_ena_n2apmbox(void);
private:
    simif_t *sim = nullptr;
    apifc_t *apifc = nullptr;
    uint8_t *reg_base = nullptr;
};

class soc_apb_t : public abstract_device_t {
public:
    soc_apb_t(simif_t *sim, apifc_t *apifc);
    ~soc_apb_t();
    bool load(reg_t addr, size_t len, uint8_t* bytes);
    bool store(reg_t addr, size_t len, const uint8_t* bytes);
    size_t size() { return sizeof(reg_base); }
    sys_apb_decoder_t * get_sys_apb_decoder_west(){return sys_apb_decoder_west;}
    sys_apb_decoder_t * get_sys_apb_decoder_east(){return sys_apb_decoder_east;}
    void disarm_sys_apb(processor_t* processor);
    sys_irq_t *sys_irq = nullptr;
private:
    simif_t *sim = nullptr;
    apifc_t *apifc = nullptr;
    sys_apb_decoder_t *sys_apb_decoder_west = nullptr;
    sys_apb_decoder_t *sys_apb_decoder_east = nullptr;

    uint8_t reg_base[SOC_APB_SIZE];
};

#endif  /* __SOC_APB_H__ */