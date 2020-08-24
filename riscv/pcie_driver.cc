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
#define NETLINK_FAULT    (-1)

/* maximum payload size */
#define MAX_PAYLOAD      (1280)
#define NETLINK_TEST     (30)
#define NL_PID           (100)
#define NL_GROUPS        (0)
pcie_driver_t::pcie_driver_t(simif_t* sim, std::vector<processor_t*>& procs)
  : procs(procs), mPSim(sim)
{
  mStatus = PCIE_UNINIT;
  init();

  // To prepare create mssage head
  mSendBuffer = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
  if (!mSendBuffer) {
    std::cout << "driver init malloc send buffer error!" << std::endl;
    // return;
  }

  memset(mSendBuffer, 0, NLMSG_SPACE(MAX_PAYLOAD));
  mSendBuffer->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
  mSendBuffer->nlmsg_pid = NL_PID;
  mSendBuffer->nlmsg_flags = NL_GROUPS;

  mRecvBuffer = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
  if (!mRecvBuffer) {
    std::cout << "driver init malloc recv buffer error!" << std::endl;
    // return;
  }

  memset(mRecvBuffer, 0, sizeof(NLMSG_SPACE(MAX_PAYLOAD)));
}

void pcie_driver_t::init()
{
  int32_t rv = 0;

  // Create a socket
  mSockFd = socket(AF_NETLINK, SOCK_RAW, NETLINK_TEST);
  if (mSockFd == NETLINK_FAULT) {
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
    std::cout << "driver init bind failed: "
    	<< strerror(errno)
    	<< std::endl;
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

/* send data to kernel by netlink, command head info must has in data. */
int32_t pcie_driver_t::send(const uint8_t* data, size_t len)
{
  int32_t rv = 0;

  if (unlikely(PCIE_OK != mStatus))
    return mStatus;

  mSendBuffer->nlmsg_len = NLMSG_SPACE(len);
  if (data && len)
    memcpy(NLMSG_DATA(mSendBuffer), data, len);

  //send message
  rv = sendto(mSockFd,
              mSendBuffer,
              NLMSG_LENGTH(len),
              0,
              (struct sockaddr*)(&mDestAddr),
              sizeof(mDestAddr));
  if (NETLINK_FAULT == rv)
    std::cout << "pcie send data error: "
        << strerror(errno)
        << std::endl;

  return rv;
}

/* get npc data for kernel at address in npc view */
#define COMMAND_HEAD_SIZE (sizeof(command_head_t) - 4)
int32_t pcie_driver_t::read(reg_t addr, size_t length)
{
  int32_t rv = 0;
  int32_t result = 0;
  int32_t size = 0;
  reg_t offset = 0;
  int32_t block_size = 1024;
  command_head_t *pCmd = NULL;

  pCmd = NLMSG_DATA(mSendBuffer);
  pCmd->code = CODE_READ;

  for (size_t i = 0; i < length; i+=block_size) {
    offset = addr + i;
    size = std::min(length - i, (size_t)block_size);
    pCmd->addr = offset;
    pCmd->len = size;
    load_data(offset, size, (uint8_t*)pCmd->data);

    //send message
    mSendBuffer->nlmsg_len = NLMSG_SPACE(size + COMMAND_HEAD_SIZE);
    rv = sendto(mSockFd,
    	        mSendBuffer,
    	        NLMSG_LENGTH(size + COMMAND_HEAD_SIZE),
                0,
                (struct sockaddr*)(&mDestAddr),
                sizeof(mDestAddr));
    if (NETLINK_FAULT == rv) {
      std::cout << "pcie send data error: "
      	  << strerror(errno)
      	  << std::endl;
      return rv;
    }

    result += rv;
  }

  return result;
}

/* recv data from PCIe port, recv data length
   no bigger than 1024 byte once. */
int32_t pcie_driver_t::recv()
{
  int32_t rv;

  /* check PCIe init status. */
  if (unlikely(PCIE_OK != mStatus))
    return mStatus;

  /* recv message and save to mRecvBuffer. */
  rv = recvfrom(mSockFd,
                mRecvBuffer,
                NLMSG_LENGTH(MAX_PAYLOAD),
                0,
                NULL,
                NULL);

  if (NETLINK_FAULT == rv)
    std::cout << "pcie recv data error: "
        << strerror(errno)
        << std::endl;

  return rv;
}

#define LOCAL_DDR_SIZE 0xa00000
#define DDR_SIZE (0xc0000000)
#define NOC_NPC0_BASE 0xc2000000
#define NOC_NPC1_BASE 0xc2800000
#define NOC_NPC2_BASE 0xca000000
#define NOC_NPC3_BASE 0xca800000
#define NOC_NPC4_BASE 0xcb000000
#define NOC_NPC5_BASE 0xd2000000
#define NOC_NPC6_BASE 0xd2800000
#define NOC_NPC7_BASE 0xd3000000
#define NOC_NPC8_BASE 0xc4000000
#define NOC_NPC9_BASE 0xc4800000
#define NOC_NPC10_BASE 0xcc000000
#define NOC_NPC11_BASE 0xcc800000
#define NOC_NPC12_BASE 0xcd000000
#define NOC_NPC13_BASE 0xd4000000
#define NOC_NPC14_BASE 0xd4800000
#define NOC_NPC15_BASE 0xd5000000

/* NPC local space in soc view. */
static const uint32_t noc_npc_base[] = {
        NOC_NPC0_BASE,
        NOC_NPC1_BASE,
        NOC_NPC2_BASE,
        NOC_NPC3_BASE,
        NOC_NPC4_BASE,
        NOC_NPC5_BASE,
        NOC_NPC6_BASE,
        NOC_NPC7_BASE,
        NOC_NPC8_BASE,
        NOC_NPC9_BASE,
        NOC_NPC10_BASE,
        NOC_NPC11_BASE,
        NOC_NPC12_BASE,
        NOC_NPC13_BASE,
        NOC_NPC14_BASE,
        NOC_NPC15_BASE
};

#define NPC_LOCAL_ADDR_START (0xc0000000)
#define NPC_MBOX_TOTAL \
	sizeof(noc_npc_base) / \
	sizeof(noc_npc_base[0])

/* current address is which npc */
#define IS_NPC(addr, id) \
	(((addr) >= noc_npc_base[id]) && \
	((addr) < noc_npc_base[id] + 0x800000))

/* change soc address to local address */
#define switch_soc_to_local(addr, id) \
	((addr) - noc_npc_base[id] + NPC_LOCAL_ADDR_START)

/* adjust which npc from addr. */
int32_t which_npc(reg_t addr, reg_t *paddr)
{
  int32_t core_id = -1;
  int32_t mbox_num;

  mbox_num = NPC_MBOX_TOTAL;
  for (int32_t i = 0; i < mbox_num; i++) {
    if (IS_NPC(addr, i)) {
      core_id = i;
      *paddr = switch_soc_to_local(addr, i);
      break;
    }
  }

  return core_id;
}

/* get data from npc addr, data will fill to buffer bytes. */
bool pcie_driver_t::load_data(reg_t addr, size_t len, uint8_t* bytes)
{
  reg_t paddr;
  int32_t core_id;

  if (auto host_addr = mPSim->addr_to_mem(addr))
    memcpy(bytes, host_addr, len);
  else if (-1 != (core_id = which_npc(addr, &paddr))) {
    if (auto host_addr = mPSim->local_addr_to_mem(paddr, core_id)) {
      memcpy(bytes, host_addr, len);
      return true;
    }

    if (!mPSim->local_mmio_load(paddr, len, bytes, core_id)) {
      std::cout << "load addr: 0x"
      	  << hex
      	  << addr
      	  << " access fault."
      	  << std::endl;
      throw trap_load_access_fault(addr);
    }
  }
  else
    if (!mPSim->mmio_load(addr, len, bytes)) {
      std::cout << "PCIe driver load addr: 0x"
      	  << hex
      	  << addr
      	  << " access fault."
      	  << std::endl;
      throw trap_load_access_fault(addr);
    }

  return true;
}

/* write bytes to npc addr. */
bool pcie_driver_t::store_data(reg_t addr, size_t len, const uint8_t* bytes)
{
  reg_t paddr;
  int32_t core_id;

  if (auto host_addr = mPSim->addr_to_mem(addr))
    memcpy(host_addr, bytes, len);
  else if (-1 != (core_id = which_npc(addr, &paddr))) {
    if (auto host_addr = mPSim->local_addr_to_mem(paddr, core_id)) {
      memcpy(host_addr, bytes, len);
      return true;
    }

    if (!mPSim->local_mmio_store(paddr, len, bytes, core_id)) {
      std::cout << "PCIe driver store addr: 0x"
      	  << hex
      	  << addr
      	  << " access fault."
      	  << std::endl;
      throw trap_store_access_fault(addr);
    }
  } else if (!mPSim->mmio_store(addr, len, bytes)) {
    std::cout << "PCIe driver load addr: 0x"
    	<< hex
    	<< addr
    	<< " access fault."
    	<< std::endl;
    throw trap_store_access_fault(addr);
  } else {
    std::cout << "load addr: 0x"
    	<< hex
    	<< addr
    	<< " access fault."
    	<< std::endl;
    throw trap_store_access_fault(addr);
  }

  return true;
}

/* PCIe mbox address, send to txcmd or exttxcmd data will write to cfg address. */
#define PCIE_MBOX_CFG_ADDR      (0xc60a1000)
#define PCIE_MBOX_TXCMD_ADDR    (0xc60a1004)
#define PCIE_MBOX_EXTTXCMD_ADDR (0xc60a1008)
void pcie_driver_t::transfer_loop()
{
  command_head_t *pCmd = NULL;

  std::cout << "driver transfer loop start." << std::endl;
  while (1) {
    if (NETLINK_FAULT != recv()) {
      pCmd = NLMSG_DATA(mRecvBuffer);
      std::cout << "recv cmd:"
          << pCmd->code
          << hex
          << " addr:0x"
          << pCmd->addr
          << " len:0x"
          << pCmd->len
          << std::endl;

      switch (pCmd->code) {
        case CODE_READ:
          read(pCmd->addr, pCmd->len);
          break;

        case CODE_WRITE:
	  /* PCIe mbox cfg addr. */
          if (PCIE_MBOX_CFG_ADDR == pCmd->addr) {
            unsigned int value = *(unsigned int *)pCmd->data;
            mTxCfgAddr = value;
            std::cout << "cfg tx dst addr " << value << std::endl;
          } else if ((PCIE_MBOX_TXCMD_ADDR == pCmd->addr) |
           (PCIE_MBOX_EXTTXCMD_ADDR == pCmd->addr)) {
           /* PCIe mbox txcmd or exttxcmd. */
            unsigned int value = *(unsigned int *)pCmd->data;
            if (PCIE_MBOX_TXCMD_ADDR == pCmd->addr)
              mTxCmd = value;
            else
              mTxExtCmd = value;

            store_data(mTxCfgAddr, pCmd->len, pCmd->data);
            std::cout << "pcie mbox send addr:0x"
	    	<< hex
	    	<< mTxCfgAddr
	    	<< " value:0x"
	    	<< value
	    	<< std::endl;
          } else
            store_data(pCmd->addr, pCmd->len, (const uint8_t*)pCmd->data);

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
