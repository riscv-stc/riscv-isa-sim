// See LICENSE for license details.
#ifndef _RISCV_PCIE_DRIVER_H
#define _RISCV_PCIE_DRIVER_H

#include <cstdlib>
#include <vector>
#include <condition_variable>
#include <mutex>
#include <thread>
#include "devices.h"

class simif_t;
class bankif_t;

#include <linux/netlink.h>
#include <linux/socket.h>

enum command_code {
    CODE_READ = 0,
    CODE_WRITE = 1,
    CODE_INTERRUPT = 2,
    CODE_STATUS = 3,
};

enum NL_STATUS {
  STATUS_UNINIT,
  STATUS_INIT,
  STATUS_OK,
  STATUS_EXIT,
};

#define COMMAND_DATA_SIZE           16
struct command_head_t {
    unsigned short code;
    unsigned short len;
    unsigned long addr;
    unsigned char data[COMMAND_DATA_SIZE];
};

/* spike和qemu通过消息队列通信 */
#define SPIKE_QEMU_MSG_PATHNAME     "/proc/stc/stc_cluster_0"
#define SPIKE_QEMU_MSG_S2Q_PROJ     's'
#define SPIKE_QEMU_MSG_Q2S_PROJ     'q'

/* spike to qemu msg type */
#define SQ_MTYPE_REQ_BASE       0x1000
#define SQ_MTYPE_RES_BASE       0x2000
/* cmd: enum command_code */
#define SQ_MTYPE_REQ(cmd)       (SQ_MTYPE_REQ_BASE + cmd)
#define SQ_MTYPE_RES(cmd)       (SQ_MTYPE_RES_BASE + cmd)
struct sq_msg_t {
    long mtype;
    struct command_head_t cmd_data;
};

#define COMMAND_HEAD_SIZE (sizeof(command_head_t) - COMMAND_DATA_SIZE)
#define PCIE_COMMAND_SEND_SIZE(cmd)  (sizeof(cmd)-sizeof(cmd.data)+cmd.len)

class pcie_driver_t {
 public:
  pcie_driver_t(simif_t* sim, bankif_t *bank, uint32_t bank_id, bool pcie_enabled, size_t board_id, size_t chip_id);
  ~pcie_driver_t();

  int send(const uint8_t* data, size_t len);
  int get_sync_state();
  
  int sqmsg_spike_send(long recv_type, const struct command_head_t *cmd_data);
  int sqmsg_spike_recv(long recv_type, struct command_head_t *cmd_data);
 
 private:
  std::unique_ptr<std::thread> mDriverThread;

  struct sockaddr_nl mSrcAddr;
  struct sockaddr_nl mDestAddr;
  struct nlmsghdr *mSendBuffer;
  struct nlmsghdr *mRecvBuffer;
  simif_t* mPSim;
  bankif_t *mBank;

  int mSockFd;
  int mStatus;
  uint32_t mBankId;
  int mDev;
  bool pcie_enabled;
  size_t board_id;
  size_t chip_id;
  
  reg_t mTxCfgAddr;
  reg_t mTxCmd;
  reg_t mTxExtCmd;
  int initialize();
  int read(reg_t addr, size_t length);
  int recv();
  bool load_data(reg_t addr, size_t len, uint8_t* bytes);
  bool store_data(reg_t addr, size_t len, const uint8_t* bytes);
  bool lock_channel(void);
  int update_status(NL_STATUS status);
  void task_doing();
  std::mutex pcie_mutex;

  int sq_s2q_msqid = -1;        /* spike send to qemu */
  int sq_q2s_msqid = -1;        /* qemu send to spike */
  std::unique_ptr<std::thread> sqmsg_spike_recv_thread_p;
  int qemu_mems_read(reg_t addr, size_t length);
  void sqmsg_req_recv_handle(void);
  int spike_qemu_msg_init(void);
  int spike_qemu_msg_destory(void);
};

#endif

