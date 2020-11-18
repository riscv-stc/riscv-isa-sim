// See LICENSE for license details.
#ifndef _RISCV_PCIE_DRIVER_H
#define _RISCV_PCIE_DRIVER_H

#include <cstdlib>
#include <vector>
#include <condition_variable>
#include <mutex>
#include <thread>
#include "devices.h"

class processor_t;
class simif_t;

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

struct command_head_t {
    unsigned short code;
    unsigned short len;
    unsigned int addr;
    unsigned char data[4];
};

class pcie_driver_t {
 public:
  pcie_driver_t(simif_t* sim, std::vector<processor_t*>& procs, uint32_t bank_id);
  ~pcie_driver_t();

  int32_t send(const uint8_t* data, size_t len);
  int32_t get_sync_state();
 private:
  std::vector<processor_t*>& procs;
  std::unique_ptr<std::thread> mDriverThread;

  struct sockaddr_nl mSrcAddr;
  struct sockaddr_nl mDestAddr;
  struct nlmsghdr *mSendBuffer;
  struct nlmsghdr *mRecvBuffer;
  simif_t* mPSim;

  int mSockFd;
  int mStatus;
  uint32_t mBankId;

  reg_t mTxCfgAddr;
  reg_t mTxCmd;
  reg_t mTxExtCmd;
  void init();
  int32_t read(reg_t addr, size_t length);
  int32_t recv();
  bool load_data(reg_t addr, size_t len, uint8_t* bytes);
  bool store_data(reg_t addr, size_t len, const uint8_t* bytes);
  int32_t tell_peer_status(NL_STATUS status);
  void task_doing();
  std::mutex pcie_mutex;
};

#endif

