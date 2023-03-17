#ifndef __MBOX_DEVICE_H__
#define __MBOX_DEVICE_H__

#include <cstdlib>
#include <vector>
#include "devices.h"
#include "mailbox.h"
#include "simif.h"

#define MBOX_RXFIFO_DEPTH       16  /* 16x64-bit */
class mbox_device_t : public abstract_device_t {
 public:
  mbox_device_t(simif_t *simif);
  ~mbox_device_t();

  bool load(reg_t addr, size_t len, uint8_t* bytes);
  bool store(reg_t addr, size_t len, const uint8_t* bytes);
  size_t size() { return sizeof(reg_base); }

  virtual void reset(void) = 0;

 protected:
  uint8_t reg_base[4096];
  std::queue<uint64_t> rx_fifo;   /* 16x64-bit */
  std::mutex store_mutex;
    
 private:
  simif_t *sim = nullptr;

  void irq_update(void);
  int send_msg(uint64_t txcfg, uint64_t txdat);

  /* 需要a53/npc/pcie_mbox 实现自己的generate_irq */
  virtual void irq_generate(bool dir);  /* true产生, false清除 */
};

class np_mbox_t : public mbox_device_t {
public:
    np_mbox_t(simif_t *simif, misc_device_t *misc_dev);
    ~np_mbox_t();

    void reset(void);

private:
    simif_t *sim = nullptr;
    misc_device_t *misc = nullptr;

    void irq_generate(bool dir) override;
};

class pcie_mbox_t : public mbox_device_t {
public:
    pcie_mbox_t(simif_t *simif, pcie_driver_t *pcie_driver);
    ~pcie_mbox_t();

    void reset(void);

private:
    simif_t *sim = nullptr;
    pcie_driver_t *pcie = nullptr;

    void irq_generate(bool dir) override;
};

#endif  /* __MBOX_DEVICE_H__ */