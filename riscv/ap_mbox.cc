#include "ap_mbox.h"

ap_mbox_t::ap_mbox_t(simif_t *simif, apifc_t *apifc, int irq_num, sys_irq_t *sys_irq) :
        sim(simif), apifc(apifc), sys_irq(sys_irq), irq(irq_num), mbox_device_t(simif)
{
    reset();
}

ap_mbox_t::~ap_mbox_t()
{
    sim = nullptr;
    apifc = nullptr;
    sys_irq = nullptr;
    irq = 0;
}

void ap_mbox_t::reset(void)
{
    uint32_t val = 0;

    while (!rx_fifo.empty())
        rx_fifo.pop();

    memset(reg_base, 0, size());

    val = (1<<5) | (1<<6) | (0xf<<8) | (1<<13) | (1<<21);
    *(uint32_t*)(reg_base+MBOX_STATUS) = val;

    val = ~((uint32_t)0);
    *(uint32_t*)(reg_base+MBOX_INT_MASK) = val;
}

void ap_mbox_t::irq_generate(bool dir)
{
    if (apifc) {
        if ((N2AP_MBOX_IRQ!=irq) && (P2AP_MBOX_IRQ!=irq)) {
            return ;
        }
        if (false == dir) {     /* 取消中断 */
            apifc->generate_irq_to_a53(irq, dir);
        } else if ((N2AP_MBOX_IRQ==irq) && sys_irq->is_irq_ena_n2apmbox()) {
            apifc->generate_irq_to_a53(irq, dir);
        } else if ((P2AP_MBOX_IRQ==irq) && sys_irq->is_irq_ena_p2apmbox()) {
            apifc->generate_irq_to_a53(irq, dir);
        }
    }
}