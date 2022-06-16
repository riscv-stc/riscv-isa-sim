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

/**
 * pcie_driver成员还未赋值
 * 可以考虑ap npc pcie mbox代码部分重用 
*/
class ap_mbox_t : public mbox_device_t {
 public:
  ap_mbox_t(simif_t *simif, apifc_t *apifc);
  ~ap_mbox_t();

  void reset(void);

private:
    simif_t *sim = nullptr;
    apifc_t *apifc = nullptr;

    void irq_generate(bool dir) override;
};

#endif  /* __AP_MBOX_H__ */