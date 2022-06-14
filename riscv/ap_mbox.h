#ifndef __AP_MBOX_H__
#define __AP_MBOX_H__

#include <cstdlib>
#include <vector>
#include "devices.h"
#include "processor.h"
#include "pcie_driver.h"
#include "mailbox.h"

#define AP_MBOX_LOC_BASE        MBOX_PCIE_REG_BASE
#define AP_MBOX_SIZE            0x1000

/**
 * pcie_driver成员还未赋值
 * 可以考虑ap npc pcie mbox代码部分重用 
*/
class ap_mbox_device_t : public abstract_device_t {
 public:
  ap_mbox_device_t(simif_t* sim, sys_irq_t *sys_irq, pcie_driver_t * pcie, bool pcie_enabled);
  bool load(reg_t addr, size_t len, uint8_t* bytes);
  bool store(reg_t addr, size_t len, const uint8_t* bytes);
  void reset();
  size_t size() { return AP_MBOX_SIZE*4; }
  ~ap_mbox_device_t();

 private:
  uint32_t cmd_count;
  queue<uint32_t> cmd_value;
  uint32_t cmdext_count;
  queue<uint32_t> cmdext_value;
  uint8_t reg_base[AP_MBOX_SIZE];
  simif_t* sim = nullptr;
  sys_irq_t *sys_irq = nullptr;
  pcie_driver_t *pcie_driver = nullptr;
  bool pcie_enabled = false;
};

#endif  /* __AP_MBOX_H__ */