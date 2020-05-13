#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <linux/netlink.h>
#include <linux/socket.h>
#include <errno.h>
#include <unistd.h>

#include "processor.h"
#include "pcie_driver.h"

#define PCIE_OK          (0)
#define PCIE_UNINIT      (-1)
#define ERROR_SOCK       (-2)
#define ERROR_BIND       (-3)

#define MAX_PAYLOAD      (1280) // maximum payload size
#define NETLINK_TEST     (30)
#define NL_PID           (100)
#define NL_GROUPS        (0)
pcie_driver_t::pcie_driver_t(simif_t* sim, std::vector<processor_t*>& procs)
  : mPSim(sim), procs(procs)
{
  mStatus = PCIE_UNINIT;
  init();

  // To prepare create mssage head
  mSendBuffer = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
  if (!mSendBuffer) {
    std::cout << "driver init malloc send buffer error!" << std::endl;
    // return;
  }

  memset(mSendBuffer, 0, sizeof(mSendBuffer));
  mSendBuffer->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
  mSendBuffer->nlmsg_pid = NL_PID;
  mSendBuffer->nlmsg_flags = NL_GROUPS;

  mRecvBuffer = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
  if (!mRecvBuffer) {
    std::cout << "driver init malloc recv buffer error!" << std::endl;
    // return;
  }

  memset(mRecvBuffer, 0, sizeof(mRecvBuffer));
}

void pcie_driver_t::init()
{
  int rv = 0;

	// Create a socket
	mSockFd = socket(AF_NETLINK, SOCK_RAW, NETLINK_TEST);
	if (mSockFd == -1) {
    std::cout << "driver init error getting socket: " \
      << strerror(errno) << std::endl;
    mStatus = ERROR_SOCK;
    return;
	}

	// To prepare binding
	memset(&mSrcAddr, 0, sizeof(mSrcAddr));
	mSrcAddr.nl_family = AF_NETLINK;
	mSrcAddr.nl_pid = NL_PID;
	mSrcAddr.nl_groups = NL_GROUPS;

  // Bind src addr
	rv = bind(mSockFd, (struct sockaddr*)&mSrcAddr, sizeof(mSrcAddr));
	if (rv < 0) {
    std::cout << "driver init bind failed: " << strerror(errno) << std::endl;
    close(mSockFd);
    mSockFd = -1;
    mStatus = ERROR_BIND;
    return;
	}

  memset(&mDestAddr, 0, sizeof(mDestAddr));
  mDestAddr.nl_family = AF_NETLINK;
  mDestAddr.nl_pid = 0;
  mDestAddr.nl_groups = 0;

  mStatus = PCIE_OK;
  auto mainloop = std::bind(&pcie_driver_t::transfer_loop, this);
  auto thread = new std::thread(mainloop);
  thread->detach();
  mDriverThread.reset(thread);
}

int pcie_driver_t::send(const uint8_t* data, size_t len)
{
  int rv = 0;

  if (unlikely(PCIE_OK != mStatus))
    return mStatus;

  mSendBuffer->nlmsg_len = NLMSG_SPACE(len);
  if (data && len)
    memcpy(NLMSG_DATA(mSendBuffer), data, len);

  //send message
  rv = sendto(mSockFd, mSendBuffer, NLMSG_LENGTH(len), 0, \
      (struct sockaddr*)(&mDestAddr), sizeof(mDestAddr));
  if (-1 == rv)
    std::cout << "pcie send data error: " << strerror(errno) << std::endl;

  return rv;
}

#define COMMAND_HEAD_SIZE (sizeof(command_head_t) - 4)
int pcie_driver_t::read(reg_t addr, size_t length)
{
  int rv = 0;
  int result = 0;
  int size = 0;
  int offset = 0;
  int block_size = 1024;
  command_head_t *pCmd = NULL;

  pCmd = NLMSG_DATA(mSendBuffer);
  pCmd->code = CODE_READ;
  for (int i = 0; i < length; i+=block_size) {
    offset = addr + i;
    size = std::min(length - i, (size_t)block_size);
    pCmd->addr = offset;
    pCmd->len = size;
    load_data(offset, size, (uint8_t*)pCmd->data);

    //send message
    mSendBuffer->nlmsg_len = NLMSG_SPACE(size + COMMAND_HEAD_SIZE);
    rv = sendto(mSockFd, mSendBuffer, NLMSG_LENGTH(size + COMMAND_HEAD_SIZE), \
        0, (struct sockaddr*)(&mDestAddr), sizeof(mDestAddr));
    if (-1 == rv) {
      std::cout << "pcie send data error: " << strerror(errno) << std::endl;
      return rv;
    }

    result += rv;
  }

  return result;
}

int pcie_driver_t::recv()
{
  int rv = 0;

  if (unlikely(PCIE_OK != mStatus))
    return mStatus;

  //recv message
  rv = recvfrom(mSockFd, mRecvBuffer, NLMSG_LENGTH(MAX_PAYLOAD), 0, NULL, NULL);
  if (-1 == rv)
    std::cout << "pcie recv data error: " << strerror(errno) << std::endl;

  return rv;
}

#define LOCAL_DDR_SIZE 0xa00000
#define DDR_SIZE (0xc0000000)
bool pcie_driver_t::load_data(reg_t addr, size_t len, uint8_t* bytes)
{
	if (auto host_addr = mPSim->addr_to_mem(addr)) {
  	// low addr use local caches
  	// if (addr + len < LOCAL_DDR_SIZE)
  		memcpy(bytes, host_addr, len);
  	return true;
  } else if (!mPSim->mmio_load(addr, len, bytes)) {
    throw trap_load_access_fault(addr);
  }

  return true;
}

bool pcie_driver_t::store_data(reg_t addr, size_t len, const uint8_t* bytes)
{
	if (auto host_addr = mPSim->addr_to_mem(addr)) {
  	// low addr use local caches
  	// if (addr + len < LOCAL_DDR_SIZE)
  		memcpy(host_addr, bytes, len);
  } else if (!mPSim->mmio_store(addr, len, bytes)) {
    throw trap_store_access_fault(addr);
  }

  return true;
}

#define INTERRUPT_ADDR (0xc60a100c)
#define INTERRUPT_EXT_ADDR (0xc60a1010)
#define MBOX_MRXCMD_ADDR (0xc07f400c)
#define MBOX_MRXCMDEXT_ADDR (0xc07f4010)
void pcie_driver_t::transfer_loop()
{
  command_head_t *pCmd = NULL;

  std::cout << "driver transfer loop start." << std::endl;
  while (1) {
    if (-1 != recv()) {
      pCmd = NLMSG_DATA(mRecvBuffer);
      std::cout << "recv cmd:" << pCmd->code << " addr:" \
        << pCmd->addr << " len:" << pCmd->len << std::endl;
      switch (pCmd->code) {
      	case CODE_READ:
      	  read(pCmd->addr, pCmd->len);
      	  break;
      	case CODE_WRITE:
          if (INTERRUPT_ADDR == pCmd->addr) {
            unsigned int value = *(unsigned int *)pCmd->data;
            store_data(MBOX_MRXCMD_ADDR, pCmd->len, pCmd->data);
            std::cout << "interrupt has occur " << value << std::endl;
          } else if (INTERRUPT_EXT_ADDR == pCmd->addr) {
            unsigned int value = *(unsigned int *)pCmd->data;
            store_data(MBOX_MRXCMDEXT_ADDR, pCmd->len, pCmd->data);
            std::cout << "interrupt has occur " << value << std::endl;
          } else {
            store_data(pCmd->addr, pCmd->len, (const uint8_t*)pCmd->data);
          }
      	  break;
        case CODE_INTERRUPT:
          procs[0]->state.mip = procs[0]->state.mip | (1 << IRQ_M_EXT);
          break;
      	default:
      	  std::cout << "unknow cmd." << std::endl;
    	}
    }
  }

  std::cout << "driver transfer loop end." << std::endl;
}

pcie_driver_t::~pcie_driver_t()
{
  if (mSockFd >= 0) {
	  close(mSockFd);
    mSockFd = -1;
  }

  if (mSendBuffer) {
    free(mSendBuffer);
    mSendBuffer = NULL;
  }

  if (mRecvBuffer) {
    free(mRecvBuffer);
    mRecvBuffer = NULL;
  }
}
