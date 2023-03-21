#ifndef __AP_MBOX_H__
#define __AP_MBOX_H__

#include <cstdlib>
#include <vector>
#include "devices.h"
#include "processor.h"
#include "pcie_driver.h"
#include "mailbox.h"
#include "mbox_device.h"
#include "apifc.h"
#include "soc_apb.h"

class ap_mbox_t : public mbox_device_t {
 public:
  ap_mbox_t(simif_t *simif, apifc_t *apifc, int irq_num, sys_irq_t *sys_irq, mbox_identify_t type);
  ~ap_mbox_t();

  void reset(void);

private:
    simif_t *sim = nullptr;
    apifc_t *apifc = nullptr;
    sys_irq_t *sys_irq = nullptr;
    int irq = 0;    /* N2AP_MBOX_IRQ / P2AP_MBOX_IRQ*/

    void irq_generate(bool dir) override;
    void send_irq_to_sysirq(int irq, bool dir);
};

#endif  /* __AP_MBOX_H__ */