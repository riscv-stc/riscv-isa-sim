#include "ap_mbox.h"

ap_mbox_t::ap_mbox_t(simif_t *simif, apifc_t *apifc) :
        sim(simif), apifc(apifc), mbox_device_t(simif)
{
    reset();
}

ap_mbox_t::~ap_mbox_t()
{

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

    printf("ap_mbox_t::reset \r\n");
}

void ap_mbox_t::irq_generate(bool dir)
{
    if (apifc) {
        apifc->generate_irq_to_a53(N2AP_MBOX_IRQ, dir);
    }
    if (dir) {
        printf("ap_mbox_t::irq_generate \r\n");
    }
}