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
#define ERROR_CONN       (-4)
#define ERROR_BIND       (-3)

#define NETLINK_FAULT    (-1)

/* maximum payload size */
#define MAX_PAYLOAD      (1280)
/* socket protacol type */
#define NL_PROTOCOL      (30)
/* socket port */
#define NL_PID           (100)
#define NL_START_PORT    (NL_PID)
/* netlink groups */
#define NL_GROUPS        (0)

#define DDR_SIZE       (0xc0000000)
#define NOC_NPC0_BASE  (0xc2000000)
#define NOC_NPC1_BASE  (0xc2800000)
#define NOC_NPC2_BASE  (0xca000000)
#define NOC_NPC3_BASE  (0xca800000)
#define NOC_NPC4_BASE  (0xcb000000)
#define NOC_NPC5_BASE  (0xd2000000)
#define NOC_NPC6_BASE  (0xd2800000)
#define NOC_NPC7_BASE  (0xd3000000)
#define NOC_NPC8_BASE  (0xc4000000)
#define NOC_NPC9_BASE  (0xc4800000)
#define NOC_NPC10_BASE (0xcc000000)
#define NOC_NPC11_BASE (0xcc800000)
#define NOC_NPC12_BASE (0xcd000000)
#define NOC_NPC13_BASE (0xd4000000)
#define NOC_NPC14_BASE (0xd4800000)
#define NOC_NPC15_BASE (0xd5000000)

#define NOC_NPC16_BASE (0xda000000)
#define NOC_NPC17_BASE (0xda800000)
#define NOC_NPC18_BASE (0xdb000000)
#define NOC_NPC19_BASE (0xe2000000)
#define NOC_NPC20_BASE (0xe2800000)
#define NOC_NPC21_BASE (0xe3000000)
#define NOC_NPC22_BASE (0xea000000)
#define NOC_NPC23_BASE (0xea800000)
#define NOC_NPC24_BASE (0xdc000000)
#define NOC_NPC25_BASE (0xdc800000)
#define NOC_NPC26_BASE (0xdd000000)
#define NOC_NPC27_BASE (0xe4000000)
#define NOC_NPC28_BASE (0xe4800000)
#define NOC_NPC29_BASE (0xe5000000)
#define NOC_NPC30_BASE (0xec000000)
#define NOC_NPC31_BASE (0xec800000)

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
        NOC_NPC15_BASE,
        NOC_NPC16_BASE,
        NOC_NPC17_BASE,
        NOC_NPC18_BASE,
        NOC_NPC19_BASE,
        NOC_NPC20_BASE,
        NOC_NPC21_BASE,
        NOC_NPC22_BASE,
        NOC_NPC23_BASE,
        NOC_NPC24_BASE,
        NOC_NPC25_BASE,
        NOC_NPC26_BASE,
        NOC_NPC27_BASE,
        NOC_NPC28_BASE,
        NOC_NPC29_BASE,
        NOC_NPC30_BASE,
        NOC_NPC31_BASE,
};

std::function<int32_t(NL_STATUS)> pcie_driver_exit;
pcie_driver_t::pcie_driver_t(simif_t* sim, std::vector<processor_t*>& procs, uint32_t bank_id)
  : procs(procs), mPSim(sim), mBankId(bank_id)
{
  mStatus = PCIE_UNINIT;
  init();

  // To prepare create mssage head
  mSendBuffer = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
  if (!mSendBuffer) {
    std::cout << "driver init malloc send buffer error!" << std::endl;
    return;
  }

  memset(mSendBuffer, 0, NLMSG_SPACE(MAX_PAYLOAD));
  mSendBuffer->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
  mSendBuffer->nlmsg_pid = NL_START_PORT + mBankId;
  mSendBuffer->nlmsg_flags = NL_GROUPS;

  mRecvBuffer = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
  if (!mRecvBuffer) {
    std::cout << "driver init malloc recv buffer error!" << std::endl;
    return;
  }

  memset(mRecvBuffer, 0, sizeof(NLMSG_SPACE(MAX_PAYLOAD)));
  /* recheck netlink status */
  if (NETLINK_FAULT == tell_peer_status(STATUS_OK))
    mStatus = ERROR_CONN;
}

void pcie_driver_t::init()
{
  int32_t rv = 0;

  // Create a socket
  mSockFd = socket(AF_NETLINK, SOCK_RAW, NL_PROTOCOL);
  if (mSockFd == NETLINK_FAULT) {
    mStatus = ERROR_SOCK;
    return;
  }

  // To prepare binding
  memset(&mSrcAddr, 0, sizeof(mSrcAddr));
  mSrcAddr.nl_family = AF_NETLINK;
  mSrcAddr.nl_pid = NL_START_PORT + mBankId;
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

  /* netlink not case nl_pid in user space. */
  mDestAddr.nl_pid = 0;
  mDestAddr.nl_groups = 0;

  /* just a status for temporary */
  mStatus = PCIE_OK;
  auto mainloop = std::bind(&pcie_driver_t::task_doing, this);
  auto thread = new std::thread(mainloop);
  thread->detach();
  mDriverThread.reset(thread);
  pcie_driver_exit = std::bind(&pcie_driver_t::tell_peer_status,
                               this,
                               std::placeholders::_1);
}

/*
 * send data to kernel by netlink blocked, command head info must has in data.
 * data: command head info and src data that will send to
 * len:  length of commamd head and src data
 */
int32_t pcie_driver_t::send(const uint8_t* data, size_t len)
{
  int32_t rv = 0;

  if (unlikely(PCIE_OK != mStatus))
    return mStatus;

  pcie_mutex.lock();
  mSendBuffer->nlmsg_len = NLMSG_SPACE(len);
  if (data && len)
    memcpy(NLMSG_DATA(mSendBuffer), data, len);

  // send message
  rv = sendto(mSockFd,
              mSendBuffer,
              NLMSG_LENGTH(len),
              0,
              (struct sockaddr*)(&mDestAddr),
              sizeof(mDestAddr));
  pcie_mutex.unlock();

  if (NETLINK_FAULT == rv)
    std::cout << "pcie send data error: "
        << strerror(errno)
        << std::endl;

  return rv;
}

/*
 * Tell the peer status.
 * status: the status of this bank netlink connect.
 */
int32_t pcie_driver_t::tell_peer_status(NL_STATUS status)
{
   command_head_t cmd;
   int32_t rv;

   cmd.code = CODE_STATUS;
   /* address is bankid */
   cmd.addr = mBankId;
   cmd.len = 4;
   *(uint32_t *)cmd.data = (uint32_t)status;
   rv = send((const uint8_t *)&cmd, sizeof(cmd));
   std::cout << "tell peer status:" << status << std::endl;
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

  pCmd = (command_head_t*)NLMSG_DATA(mSendBuffer);
  for (size_t i = 0; i < length; i+=block_size) {
    offset = addr + i;
    size = std::min(length - i, (size_t)block_size);

    pcie_mutex.lock();
    pCmd->addr = offset;
    pCmd->len = size;
    load_data(offset, size, (uint8_t*)pCmd->data);

    pCmd->code = CODE_READ;
    mSendBuffer->nlmsg_len = NLMSG_SPACE(size + COMMAND_HEAD_SIZE);
    rv = sendto(mSockFd,
    	        mSendBuffer,
    	        NLMSG_LENGTH(size + COMMAND_HEAD_SIZE),
                0,
                (struct sockaddr*)(&mDestAddr),
                sizeof(mDestAddr));
    pcie_mutex.unlock();

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
  // if (unlikely(PCIE_OK != mStatus))
  //   return mStatus;

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

#define CORE_NUM_OF_BANK (0x8)
#define CORE_ID_MASK     (0x7)
#define NPC_LOCAL_ADDR_START (0xc0000000)
#define NPC_LOCAL_REGIN_SIZE (0x800000)
#define IGNORE_BANKID(core_id) ((core_id) & CORE_ID_MASK)
#define NPC_MBOX_TOTAL \
	(sizeof(noc_npc_base) / \
	sizeof(noc_npc_base[0]))

/* current address is which npc */
#define IS_NPC(addr, id) \
	(((addr) >= noc_npc_base[id]) && \
	((addr) < noc_npc_base[id] + NPC_LOCAL_REGIN_SIZE))

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

  return IGNORE_BANKID(core_id);
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
      //throw trap_load_access_fault(addr);
    }
  }
  else
    if (!mPSim->mmio_load(addr, len, bytes)) {
      std::cout << "PCIe driver load addr: 0x"
      	  << hex
      	  << addr
      	  << " access fault."
      	  << std::endl;
      //throw trap_load_access_fault(addr);
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
      //throw trap_store_access_fault(addr);
    }
  } else if (!mPSim->mmio_store(addr, len, bytes)) {
    std::cout << "PCIe driver load addr: 0x"
    	<< hex
    	<< addr
    	<< " access fault."
    	<< std::endl;
    //throw trap_store_access_fault(addr);
  } else {
    std::cout << "load addr: 0x"
    	<< hex
    	<< addr
    	<< " access fault."
    	<< std::endl;
    //throw trap_store_access_fault(addr);
  }

  return true;
}

/* PCIe mbox address, send to txcmd or exttxcmd
 * data will write to cfg address. */
#define PCIE_MBOX_CFG_ADDR      (0xc60a1000)
#define PCIE_MBOX_TXCMD_ADDR    (0xc60a1004)
#define PCIE_MBOX_EXTTXCMD_ADDR (0xc60a1008)

/* core reset addr, just only write,
 * not realy phy_addr, just valid for PCIe dummy driver. */
#define PCIE_CORE_RESET_ADDR    (0xc07f3500)

/* sync state addr, just only read,
 * not realy phy_addr, just valid for PCIe dummy driver. */
#define PCIE_SYNC_STATE_ADDR    (0xc07f3504)
int32_t pcie_driver_t::get_sync_state()
{
  int32_t rv;
  uint32_t state = 0;
  command_head_t cmd;

  cmd.code = CODE_READ;
  cmd.addr = PCIE_SYNC_STATE_ADDR;
  cmd.len = 4;


  for (reg_t i = 0; i < procs.size(); i++)
    if (procs[i]->async_state())
      state |= 0x1 << i;
  /* each bank core_id in spike is start at 0. */
  *(uint32_t *)cmd.data = state << (mBankId * CORE_NUM_OF_BANK);
  rv = send((const uint8_t *)&cmd, sizeof(cmd));
  return rv;
}

void pcie_driver_t::task_doing()
{
  command_head_t *pCmd = NULL;
  uint32_t value;

  /* check PCIe init status. */
  if (unlikely(PCIE_OK != mStatus))
    return ;

  std::cout << "driver transfer loop start." << std::endl;
  while (1) {
    if (NETLINK_FAULT != recv()) {
      pCmd = (command_head_t*)NLMSG_DATA(mRecvBuffer);
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
	  switch (pCmd->addr) {
	    case PCIE_SYNC_STATE_ADDR:
	      get_sync_state();
	      break;

	    default:
              read(pCmd->addr, pCmd->len);
	  }
	  usleep(1);
          break;

        case CODE_WRITE:
	  switch (pCmd->addr) {
	    /* PCIe mbox cfg addr. */
            case PCIE_MBOX_CFG_ADDR:
	      {
                value = *(uint32_t *)pCmd->data;
                mTxCfgAddr = value;
                std::cout << "cfg tx dst addr " << value << std::endl;
              }
	      break;

	    case PCIE_MBOX_TXCMD_ADDR:
            case PCIE_MBOX_EXTTXCMD_ADDR:
	      {
                /* PCIe mbox txcmd or exttxcmd. */
                value = *(uint32_t *)pCmd->data;
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
              }
	      break;

	    case PCIE_CORE_RESET_ADDR:
	      value = *(uint32_t *)pCmd->data;
	      /* each bank in spike core_id is start at 0. */
	      mPSim->hart_reset(value);
	      break;

	    default:
              store_data(pCmd->addr, pCmd->len, (const uint8_t*)pCmd->data);
	  }
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
  /* netlink exit, no care error. */
  tell_peer_status(STATUS_EXIT);
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
