// See LICENSE for license details.
#ifndef _RISCV_PCIE_DRIVER_H
#define _RISCV_PCIE_DRIVER_H

#include <cstdlib>
#include <vector>
#include <condition_variable>
#include <mutex>
#include <thread>
#include "devices.h"
#include "mbox_device.h"
#include "atu.h"

class simif_t;
class bankif_t;

#include <linux/netlink.h>
#include <linux/socket.h>
#include <semaphore.h>

#define PCIE_OK          (0)
#define PCIE_UNINIT      (-1)
#define ERROR_SOCK       (-2)
#define ERROR_BIND       (-3)
#define ERROR_CONN       (-4)
#define ERROR_LOCK       (-5)

#define NETLINK_FAULT    (-1)

#define PF_BAR_CMD_CODE(barid, iswrite)   (2 * barid + iswrite + 0x80)
#define CMD_CODE_TO_BARID(cmd)            ((cmd & 0x0f) / 2)

enum command_code {
    CODE_READ = 0,
    CODE_WRITE = 1,
    CODE_INTERRUPT = 2,
    CODE_STATUS = 3,

    CODE_PF_BAR0_READ = PF_BAR_CMD_CODE(0, CODE_READ),    /* 0x80 */
    CODE_PF_BAR0_WRITE = PF_BAR_CMD_CODE(0, CODE_WRITE),  /* 0x81 */

    CODE_PF_BAR2_READ = PF_BAR_CMD_CODE(2, CODE_READ),    /* 0x84 */
    CODE_PF_BAR2_WRITE = PF_BAR_CMD_CODE(2, CODE_WRITE),  /* 0x85 */

    CODE_PF_BAR4_READ = PF_BAR_CMD_CODE(4, CODE_READ),    /* 0x88 */
    CODE_PF_BAR4_WRITE = PF_BAR_CMD_CODE(4, CODE_WRITE),  /* 0x89 */

    /* r/w host mems */
    CODE_READ_HOST      = 0x100,
    CODE_WRITE_HOST     = 0x101,
};

enum NL_STATUS {
  STATUS_UNINIT,
  STATUS_INIT,
  STATUS_OK,
  STATUS_EXIT,
};

#define COMMAND_DATA_SIZE_MAX           512
struct command_head_t {
    unsigned short code;
    unsigned short len;
    unsigned long addr;
    unsigned char data[COMMAND_DATA_SIZE_MAX];
};

#define COMMAND_HEAD_SIZE (sizeof(command_head_t) - COMMAND_DATA_SIZE_MAX)
#define PCIE_COMMAND_SEND_SIZE(cmd)  (sizeof(cmd)-sizeof(cmd.data)+cmd.len)

#define PCIE_REGION_SIZE      0x100000  /* 1MiB */    

#define PCIE_IOV_ATU0_OFFSET  0xC0000
#define PCIE_IOV_ATU1_OFFSET  0xC8000
#define PCIE_IOV_ATU_SIZE     0x8000

#define PCIE_AXI_SLAVE      0x00011000
#define PCIE_AXI_SLAVE_SIZE 0x1000

/* PCIE_DMA */
#define PCIE_DMA_OFFSET     0x00017000
#define PCIE_DMA_SIZE       0x1000
#define PCIE_DMA_CH_TOTAL   16

#define AXI_MASTER_COMM_OFFSET  0x10000
#define AXI_MASTER_COMM_SIZE    0x1000

#define PCIE_MBOX_PF_OFFSET     0xb0000
#define PCIE_MBOX_PF_SIZE       0x1000

#define AXI_ADDR0   (PCIE_AXI_SLAVE+0x120)
#define AXI_ADDR1   (PCIE_AXI_SLAVE+0x124)

#define IS_IN_REGION(addr, start, size) ((start<=addr && start+size>addr) ? true : false)

#define PCIEDMA_VERSION     0x00
#define PCIEDMA_FEATURES    0x04
#define PCIEDMA_DESC_L      0x10
#define PCIEDMA_DESC_U      0x14
#define PCIEDMA_ATTIBUTES   0x18
#define PCIEDMA_CONTROL     0x1c

#define PCIEDMA_CTL_READY         31
#define PCIEDMA_CTL_ERR_STAT      29
#define PCIEDMA_CTL_DONE_STAT     28
#define PCIEDMA_CTL_ERR_IRQ_ENA   27
#define PCIEDMA_CTL_DONE_IRQ_ENA  26
#define PCIEDMA_CTL_STATUS        20
#define PCIEDMA_CTL_OB_NOT_IB     17
#define PCIEDMA_CTL_GO            16
#define PCIEDMA_CTL_LENGTH        0

#define PCIEDMA_DESC(reg_base, ch)          (*(uint64_t*)((uint8_t*)(reg_base)+0x20*ch+PCIEDMA_DESC_L))
#define PCIEDMA_CTL(reg_base, ch)           (*(uint32_t*)((uint8_t*)(reg_base)+0x20*ch+PCIEDMA_CONTROL))

enum {
	XFER_DIR_H2D = 0,
	XFER_DIR_D2H = 1,	/* ob_not_ib */
};

/* PCIE_DMA_DESC */
#define PDD_SOC_ADDR_L_OFFSET        0x00
#define PDD_SOC_ADDR_U_OFFSET        0x04
#define PDD_SOC_ATTR_OFFSET          0x08
#define PDD_PCIE_ADDR_L_OFFSET       0x0c
#define PDD_PCIE_ADDR_H_OFFSET       0x10
#define PDD_PCIE_ATTR_OFFSET         0x14
#define PDD_PCIE_PASID_OFFSET        0x18
#define PDD_PCIE_TPH_OFFSET          0x1c
#define PDD_CTL_LEN_OFFSET           0x20
#define PDD_STATUS_OFFSET            0x24

#define PDD_ONE_DESC_LEN                40
#define PDD_SOC_ADDR(desc_base)         (*(uint64_t*)((uint8_t*)desc_base+PDD_SOC_ADDR_L_OFFSET))
#define PDD_PCIE_ADDR(desc_base)        (*(uint64_t*)((uint8_t*)desc_base+PDD_PCIE_ADDR_L_OFFSET))
#define PDD_CTL_LEN(desc_base)          (*(uint64_t*)((uint8_t*)desc_base+PDD_CTL_LEN_OFFSET))

#define XFER_LEN_ONCE_MAX       COMMAND_DATA_SIZE_MAX

class pcie_ctl_device_t;

class pcie_driver_t {
 public:
  pcie_driver_t(simif_t* sim, bool pcie_enabled, size_t board_id, size_t chip_id, const char *atuini);
  ~pcie_driver_t();

  int send(const uint8_t* data, size_t len);

  void set_mStatus(int status) {mStatus = status;};
  int update_status(NL_STATUS status);

  /* pcie_dma */
  int read_host_ready_to(struct command_head_t *cmd);
  int read_host_wait(void);

  pcie_ctl_device_t *get_pcie_ctl(void) { return  pcie_ctl;};

 private:
  std::unique_ptr<std::thread> mDriverThread;

  struct sockaddr_nl mSrcAddr;
  struct sockaddr_nl mDestAddr;
  struct nlmsghdr *mSendBuffer;
  struct nlmsghdr *mRecvBuffer;
  simif_t* mPSim;

  int mSockFd;
  int mStatus;
  bool pcie_enabled;
  size_t board_id;
  size_t chip_id;  
  reg_t mTxCfgAddr;
  reg_t mTxCmd;
  reg_t mTxExtCmd;

  int cluster_mdev[4] = {-1, -1, -1, -1};
  int initialize();
  int recv();
  bool load_data(reg_t addr, size_t len, uint8_t* bytes);
  bool store_data(reg_t addr, size_t len, const uint8_t* bytes);

  bool pcie_bar_read(int barid, reg_t addr, size_t length, uint8_t *data);
  bool pcie_bar_store_data(int barid, reg_t addr, size_t len, const uint8_t* bytes);

  int send_nl_read_resp(uint64_t addr, uint8_t *data, int len, unsigned short resp_code);

  bool lock_channel(void);
  void task_doing();
  std::mutex pcie_mutex;

  /* pcie_dma */
  sem_t read_host_sem;
  std::mutex read_host_mutex;;
  struct command_head_t *read_host_cmd = nullptr;
  void read_host_notify_ok(struct command_head_t *cmd);

  pcie_ctl_device_t *pcie_ctl = nullptr;
};

class pcie_dma_dev_t : public abstract_device_t {
  public:
    pcie_dma_dev_t(uint8_t *pcie_dma_regs, atu_t *_pcie_atu,
        atu_t *_pcie_desc_atu, simif_t *_sim, pcie_driver_t *_pcie);
    ~pcie_dma_dev_t();

    size_t size(void) {return len;};
    bool load(reg_t addr, size_t len, uint8_t* bytes);
    bool store(reg_t addr, size_t len, const uint8_t* bytes);

  private:
    uint8_t *reg_base = nullptr;
    atu_t *pcie_atu;
    atu_t *pcie_desc_atu;
    simif_t *sim = nullptr;
    pcie_driver_t *pcie = nullptr;
    size_t len = 0x20*PCIE_DMA_CH_TOTAL;   /* PCIE_DMA_SIZE */

    void pcie_dma_go(int ch);
    int write_soc(uint64_t addr, uint8_t *data, int len);
    int read_soc(uint64_t addr, uint8_t *data, int len);
    int write_host(uint64_t addr, uint8_t *data, int len);
    int read_host(uint64_t addr, uint8_t *data, int len);
    int pcie_dma_xfer(uint64_t soc, uint64_t pcie, int len, int ob_not_ib);
};

class axi_master_common_t : public abstract_device_t {
  public:
    axi_master_common_t(uint8_t *regs);
    ~axi_master_common_t();

    size_t size(void) {return len;};
    bool load(reg_t addr, size_t len, uint8_t* bytes);
    bool store(reg_t addr, size_t len, const uint8_t* bytes);

    uint64_t bar_axi_addr(int barid, uint32_t bar_offset);
  private:
    uint8_t *reg_base = nullptr;
    size_t len = 0x28;   /* bar0 - bar4 */
    uint64_t bar_axi_base_addr(int barid);
    uint64_t pf_bar2_axi_addr(uint32_t bar_offset);
};

class pcie_ctl_device_t : public abstract_device_t {
 public:
  pcie_ctl_device_t(simif_t *_sim, pcie_driver_t *_pcie, const char *atuini);
  ~pcie_ctl_device_t();

  size_t size(void) {return len;};
  bool load(reg_t addr, size_t len, uint8_t* bytes);
  bool store(reg_t addr, size_t len, const uint8_t* bytes);

  /* pcie atu */
  atu_t *get_atu(int n) { return pcie_atu[n]; }
  uint64_t bar_axi_addr(int barid, uint32_t bar_offset);
 private:
  simif_t *sim = nullptr;
  pcie_driver_t *pcie = nullptr;
  size_t len = PCIE_CTL_CFG_SIZE;
  uint8_t *reg_base = nullptr;

  atu_t *pcie_atu[2];

  pcie_mbox_t *pcie_mbox = nullptr;
  pcie_dma_dev_t *pcie_dma = nullptr;
  axi_master_common_t *axi_master_comm = nullptr;
};

#endif

