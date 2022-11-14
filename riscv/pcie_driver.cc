#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <linux/netlink.h>
#include <linux/socket.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "processor.h"
#include "pcie_driver.h"
#include "simif.h"
#include "noc_addr.h"

/* maximum payload size */
#define MAX_PAYLOAD      (1280)
/* socket protacol type */
#define NL_PROTOCOL      (30)
/* socket port */
#define NL_PID           (100)
#define NL_START_PORT    (NL_PID)

/* netlink port, boarid bit15-bit12,chipid bit11-bit8 ,
 * NL_START_PORT + bankid bit7-bit0 
 */
#define NL_BOARD_BIT     (12)
#define NL_CHIP_BIT      (8)
#define NL_BOARD_ADDR(board_id) (board_id << NL_BOARD_BIT)
#define NL_CHIP_ADDR(chip_id)   (chip_id << NL_CHIP_BIT)

#define NL_NETLINK_START_PORT(board_id,chip_id) (NL_BOARD_ADDR(board_id)|NL_CHIP_ADDR(chip_id) + NL_START_PORT)
/* netlink groups */
#define NL_GROUPS        (0)

pcie_driver_t::pcie_driver_t(simif_t* sim, bool pcie_enabled,
        size_t board_id, size_t chip_id, const char *atuini) : mPSim(sim),
        pcie_enabled(pcie_enabled), board_id(board_id), chip_id(chip_id)
{
  mStatus = PCIE_UNINIT;

  if(!pcie_enabled)
  	return;

  /* pcie_dma */
  sem_init(&read_host_sem, 0, 0);

  if (initialize() != PCIE_OK) {
    std::cout << "PCIe driver init fail." << std::endl;
    return;
  }

  // To prepare create mssage head
  mSendBuffer = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
  if (!mSendBuffer) {
    std::cout << "driver init malloc send buffer error!" << std::endl;
    return;
  }

  memset(mSendBuffer, 0, NLMSG_SPACE(MAX_PAYLOAD));
  mSendBuffer->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
  mSendBuffer->nlmsg_pid = NL_NETLINK_START_PORT(board_id, chip_id);
  mSendBuffer->nlmsg_flags = NL_GROUPS;

  mRecvBuffer = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
  if (!mRecvBuffer) {
    std::cout << "driver init malloc recv buffer error!" << std::endl;
    return;
  }

  memset(mRecvBuffer, 0, sizeof(NLMSG_SPACE(MAX_PAYLOAD)));

  #if 0 /* 需要初始化完 pcie_mbox,sys_soc等, 再 update_status() */
  /* Recheck netlink status */
  if (NETLINK_FAULT == update_status(STATUS_OK))
    mStatus = ERROR_CONN;
  #endif

  pcie_ctl = new pcie_ctl_device_t(mPSim, this, atuini);
}

int pcie_driver_t::initialize()
{
  int rv = 0;

  if (!lock_channel()) {
    mStatus = ERROR_LOCK;
    return ERROR_LOCK;
  }

  // Create a socket
  mSockFd = socket(AF_NETLINK, SOCK_RAW, NL_PROTOCOL);
  if (mSockFd == NETLINK_FAULT) {
    mStatus = ERROR_SOCK;
    return ERROR_SOCK;
  }

  // To prepare binding
  memset(&mSrcAddr, 0, sizeof(mSrcAddr));
  mSrcAddr.nl_family = AF_NETLINK;
  mSrcAddr.nl_pid = NL_NETLINK_START_PORT(board_id, chip_id);
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
    return ERROR_BIND;
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

  return PCIE_OK;
}

bool pcie_driver_t::lock_channel(void)
{
  char magic_str[] = "lock";
  int magic_len = 4;
  char pathname[32];
  int rc;

  for (int i = 0 ; i < int(mPSim->nbanks()) ; i++) {
    memset(pathname, 0, sizeof(pathname));
    sprintf(pathname, "/proc/stc/stc_cluster_%d", i);
    if (access(pathname, F_OK)) {
      std::cout << "Cluster["
                << i
                << "] dev is not exist, please check driver."
                << std::endl;
      return false;
    }

    cluster_mdev[i] = open(pathname, O_RDWR);
    if (cluster_mdev[i] < 0)
      return false;

    rc = write(cluster_mdev[i], magic_str, magic_len);
    if (rc != magic_len)
      return false;
    usleep(10 * 1000);
  }

  return true;
}

/*
 * send data to kernel by netlink blocked, command head info must has in data.
 * data: command head info and src data that will send to
 * len:  length of commamd head and src data
 */
int pcie_driver_t::send(const uint8_t* data, size_t len)
{
  int rv = 0;

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
int pcie_driver_t::update_status(NL_STATUS status)
{
   command_head_t cmd;
   int rv;

   cmd.code = CODE_STATUS;
   /* address is bankid */
   cmd.addr = 0;
   cmd.len = 4;
   *(uint32_t *)cmd.data = (uint32_t)status;
   rv = send((const uint8_t *)&cmd, PCIE_COMMAND_SEND_SIZE(cmd));
   std::cout << "tell peer status:" << status << std::endl;
   return rv;
}

/* get npc data for kernel at address in npc view */
int pcie_driver_t::read(reg_t addr, size_t length)
{
  int rv = 0;
  int count = 0;
  int size = 0;
  reg_t offset = 0;
  int block_size = 1024;
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

    count += rv;
  }

  return count;
}

int pcie_driver_t::pcie_bar_read(int barid, reg_t addr, size_t length)
{
  uint64_t axi_addr = 0;
  uint64_t paddr = 0;
  
  axi_addr = pcie_ctl->bar_axi_addr(barid, addr);

  if (2 == barid) {   /* pf bar2 不经过ATU */
    paddr = axi_addr;
  } else {    /* pf bar0 bar4 经过ATU */
    atu_t *at = pcie_ctl->get_atu(0);
    if (at && at->is_ipa_enabled()) {
      paddr = at->translate(axi_addr, length);
    }
  }

  return read(paddr, length);
}

bool pcie_driver_t::pcie_bar_store_data(int barid, reg_t addr, size_t len, const uint8_t* bytes)
{
  uint64_t axi_addr = 0;
  uint64_t paddr = 0;
  
  axi_addr = pcie_ctl->bar_axi_addr(barid, addr);

  paddr = axi_addr;
  if (2 != barid) {      /* pf bar2 不经过ATU */
    atu_t *at = pcie_ctl->get_atu(0);
    if (at && at->is_ipa_enabled()) {
      paddr = at->translate(axi_addr, len);
    }
  }

  return store_data(paddr, len, bytes);
}

/* recv data from PCIe port, recv data length
   no bigger than 1024 byte once. */
int pcie_driver_t::recv()
{
  int rv;

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
#define IGNORE_BANKID(core_id) ((core_id) & CORE_ID_MASK)

/* get data from npc addr, data will fill to buffer bytes. */
bool pcie_driver_t::load_data(reg_t addr, size_t len, uint8_t* bytes)
{
    reg_t paddr = 0;
    int procid = 0;
    int idxinbank  = 0;
    char *host_addr = nullptr;

    procid = which_npc(addr, &paddr);
    if (0 <= procid) {
        int idxinsim =mPSim->coreid_to_idxinsim(procid);
        int bankid = mPSim->get_bankid(idxinsim);
        idxinbank = mPSim->get_idxinbank(idxinsim);
        host_addr=mPSim->npc_addr_to_mem(paddr, bankid, idxinbank);

        if (host_addr) {
            memcpy(bytes, host_addr, len);
            return true;
        } else {
            if (mPSim->npc_mmio_load(paddr, len, bytes, bankid, idxinbank)) {
                return true;
            } else {
                std::cout << "PCIe driver load addr: 0x"
                    << hex
                    << addr
                    << " access fault."
                    << std::endl;
                throw trap_load_access_fault(false, addr, 0, 0);
            }
        }
    }

    if (nullptr != (host_addr=mPSim->addr_to_mem(addr))) {
        memcpy(bytes, host_addr, len);
    } else if (!mPSim->mmio_load(addr, len, bytes)) {
        std::cout << "PCIe driver load addr: 0x"
            << hex
            << addr
            << " access fault."
            << std::endl;
        throw trap_load_access_fault(false, addr, 0, 0);
    }

    return true;
}

/* write bytes to npc addr. */
bool pcie_driver_t::store_data(reg_t addr, size_t len, const uint8_t* bytes)
{
    reg_t paddr = 0;
    int procid = 0;
    int idxinbank  = 0;
    char *host_addr = nullptr;

    procid = which_npc(addr, &paddr);
    if (0 <= procid) {
        int idxinsim =mPSim->coreid_to_idxinsim(procid);
        int bankid = mPSim->get_bankid(idxinsim);
        idxinbank = mPSim->get_idxinbank(idxinsim);
        host_addr=mPSim->npc_addr_to_mem(paddr, bankid, idxinbank);

        if (host_addr) {
            memcpy(host_addr, bytes, len);
            return true;
        } else {
            if (mPSim->npc_mmio_store(paddr, len, bytes, bankid, idxinbank)) {
                return true;
            } else {
                std::cout << "PCIe driver store addr: 0x"
                    << hex
                    << addr
                    << " access fault."
                    << std::endl;
                throw trap_store_access_fault(false, addr, 0, 0);
            }
        }
    }

    if (nullptr != (host_addr=mPSim->addr_to_mem(addr))) {
        memcpy(host_addr, bytes, len);
    } else if (!mPSim->mmio_store(addr, len, bytes)) {
        std::cout << "PCIe driver store addr: 0x"
            << hex
            << addr
            << " access fault."
            << std::endl;
        throw trap_store_access_fault(false, addr, 0, 0);
    }
    
    return true;
}

/* core reset addr, just only write,
 * not realy phy_addr, just valid for PCIe dummy driver. */
#define PCIE_CORE_RESET_ADDR    (0xc07f3500)

void pcie_driver_t::task_doing()
{
  command_head_t *pCmd = NULL;
  uint32_t value;

  /* check PCIe init status. */
  if (unlikely(PCIE_OK != mStatus))
    return;

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
          read(pCmd->addr, pCmd->len);
          usleep(1);
          break;
        case CODE_WRITE:
          switch (pCmd->addr) {
            case PCIE_CORE_RESET_ADDR:
              value = *(uint32_t *)pCmd->data;
              /* each bank in spike core_id is start at 0. */
              mPSim->hart_reset(value);
              break;

            default:
              store_data(pCmd->addr, pCmd->len, (const uint8_t*)pCmd->data);
          }
          break;
        case CODE_PF_BAR0_READ:
        case CODE_PF_BAR2_READ:
        case CODE_PF_BAR4_READ:
          pcie_bar_read(CMD_CODE_TO_BARID(pCmd->code), pCmd->addr, pCmd->len);
          break;
        case CODE_PF_BAR0_WRITE:
        case CODE_PF_BAR2_WRITE:
        case CODE_PF_BAR4_WRITE:
          pcie_bar_store_data(CMD_CODE_TO_BARID(pCmd->code), pCmd->addr, pCmd->len, (const uint8_t*)pCmd->data);
          break;
        case CODE_READ_HOST:    /* CODE_READ_HOST resp */
          read_host_notify_ok(pCmd);
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
  update_status(STATUS_EXIT);
  if (mSockFd >= 0) {
    close(mSockFd);
    mSockFd = -1;
  }

  for (int i = 0 ; i < int(mPSim->nbanks()) ; i++) {
    if (0 <= cluster_mdev[i])
      close(cluster_mdev[i]);
  }

  if (mSendBuffer) {
    free(mSendBuffer);
    mSendBuffer = NULL;
  }

  if (mRecvBuffer) {
    free(mRecvBuffer);
    mRecvBuffer = NULL;
  }

  if (pcie_ctl) {
      delete pcie_ctl;
      pcie_ctl = nullptr;
  }
}

/* 成功返回0 */
int pcie_driver_t::read_host_ready_to(struct command_head_t *cmd)
{
  int i = 0;
  int sem_val = 0;

  if (!read_host_mutex.try_lock()) {
    return -1;
  }

  read_host_cmd = cmd;

  sem_getvalue(&read_host_sem, &sem_val);
  if (0 < sem_val) {
    for (i = 0 ; i < sem_val ; i++) {
      sem_trywait(&read_host_sem);
    }
  }

  return 0;
}

int pcie_driver_t::read_host_wait(void)
{
  sem_wait(&read_host_sem);
  read_host_mutex.unlock();

  return 0;
}

void pcie_driver_t::read_host_notify_ok(struct command_head_t *cmd)
{
  memcpy(read_host_cmd, cmd, sizeof(*cmd));
  sem_post(&read_host_sem);
}

pcie_ctl_device_t::pcie_ctl_device_t(simif_t *_sim, pcie_driver_t *_pcie, const char *atuini) : sim(_sim), pcie(_pcie)
{
  reg_base = (uint8_t *)malloc(len);

  pcie_atu[0] = new atu_t(atuini, 0, (uint8_t *)reg_base + PCIE_IOV_ATU0_OFFSET);
  pcie_atu[1] = new atu_t(atuini, 1, (uint8_t *)reg_base + PCIE_IOV_ATU1_OFFSET);

  pcie_atu[0]->reset();
  pcie_atu[1]->reset();

  pcie_mbox = new pcie_mbox_t(sim, pcie);
  pcie_dma = new pcie_dma_dev_t((uint8_t *)reg_base + PCIE_DMA_OFFSET, pcie_atu[0], pcie_atu[1], sim, pcie);
  axi_master_comm = new axi_master_common_t((uint8_t *)reg_base + AXI_MASTER_COMM_OFFSET);
}

pcie_ctl_device_t::~pcie_ctl_device_t(void)
{
  delete pcie_dma;
  pcie_dma = nullptr;

  delete axi_master_comm;
  axi_master_comm = nullptr;

  delete pcie_atu[0];
  pcie_atu[0] = nullptr;

  delete pcie_atu[1];
  pcie_atu[1] = nullptr;

  delete pcie_mbox;
  pcie_mbox = nullptr;

  delete reg_base;
  reg_base = nullptr;
}

bool pcie_ctl_device_t::load(reg_t addr, size_t len, uint8_t* bytes)
{
  if (unlikely(!bytes || ((size()<addr+len)) || ((4!=len)))) {
    std::cout << "pcie_ctl_cfg: unsupported load register offset: " << hex << addr
        << " len: " << hex << len << std::endl;
    return false;
  }

  if (IS_IN_REGION(addr, PCIE_DMA_OFFSET, PCIE_DMA_SIZE)) {   /* pcie_dma */
    pcie_dma->load(addr-PCIE_DMA_OFFSET, len, bytes);
  } 
  else if (IS_IN_REGION(addr, AXI_MASTER_COMM_OFFSET, AXI_MASTER_COMM_SIZE)) {  /* axi_master_common */
    axi_master_comm->load(addr-AXI_MASTER_COMM_OFFSET, len, bytes);
  }
  else if (IS_IN_REGION(addr, PCIE_MBOX_PF_OFFSET, PCIE_MBOX_PF_SIZE)) {    /* pcie_dma */
    pcie_dma->load(addr-PCIE_MBOX_PF_OFFSET, len, bytes);
  }
  else if (IS_IN_REGION(addr, PCIE_IOV_ATU0_OFFSET, PCIE_IOV_ATU_SIZE)) {   /* pcie_atu */
    pcie_atu[0]->load(addr-PCIE_IOV_ATU0_OFFSET, len, bytes);
  }
  else if (IS_IN_REGION(addr, PCIE_IOV_ATU1_OFFSET, PCIE_IOV_ATU_SIZE)) {   /* pcie_desc_atu */
    pcie_atu[1]->load(addr-PCIE_IOV_ATU1_OFFSET, len, bytes);
  }
  else {
    memcpy(bytes, (uint8_t*)reg_base+addr, len);
  }

  return true;
}

bool pcie_ctl_device_t::store(reg_t addr, size_t len, const uint8_t* bytes)
{
  if (unlikely(!bytes || ((size()<addr+len)) || ((4!=len)))) {
    std::cout << "pcie_ctl_cfg: unsupported store register offset: " << hex << addr
        << " len: " << hex << len << std::endl;
    return false;
  }

  if (IS_IN_REGION(addr, PCIE_DMA_OFFSET, PCIE_DMA_SIZE)) {   /* pcie_dma */
    pcie_dma->store(addr-PCIE_DMA_OFFSET, len, bytes);
  }
  else if (IS_IN_REGION(addr, AXI_MASTER_COMM_OFFSET, AXI_MASTER_COMM_SIZE)) {  /* axi_master_common */
    axi_master_comm->store(addr-AXI_MASTER_COMM_OFFSET, len, bytes);
  }
  else if (IS_IN_REGION(addr, PCIE_MBOX_PF_OFFSET, PCIE_MBOX_PF_SIZE)) {    /* pcie_dma */
    pcie_dma->store(addr-PCIE_MBOX_PF_OFFSET, len, bytes);
  }
  else if (IS_IN_REGION(addr, PCIE_IOV_ATU0_OFFSET, PCIE_IOV_ATU_SIZE)) {
    pcie_atu[0]->store(addr-PCIE_IOV_ATU0_OFFSET, len, bytes);
  }
  else if (IS_IN_REGION(addr, PCIE_IOV_ATU1_OFFSET, PCIE_IOV_ATU_SIZE)) {
    pcie_atu[1]->store(addr-PCIE_IOV_ATU1_OFFSET, len, bytes);
  }
  else {
    memcpy((uint8_t*)reg_base+addr, bytes, len);
  }

  return true;
}

/* pcie滑窗地址， 转换到 axi(noc)地址 */
uint64_t pcie_ctl_device_t::bar_axi_addr(int barid, uint32_t bar_offset)
{
  return axi_master_comm->bar_axi_addr(barid, bar_offset);
}

pcie_dma_dev_t::pcie_dma_dev_t(uint8_t *pcie_dma_regs, atu_t *_pcie_atu,
    atu_t *_pcie_desc_atu,simif_t *_sim, pcie_driver_t *_pcie) : reg_base(pcie_dma_regs),
    pcie_atu(_pcie_atu), pcie_desc_atu(_pcie_desc_atu), sim(_sim), pcie(_pcie)
{
  int i = 0;

  if (NULL == reg_base) {
    throw std::invalid_argument("error! pcie_dma reg_base is nullptr");
  }

  for (i = 0 ; i < PCIE_DMA_CH_TOTAL ; i++) {
    PCIEDMA_CTL(reg_base, i) |= (1<<PCIEDMA_CTL_READY);
  }
}

pcie_dma_dev_t::~pcie_dma_dev_t(void)
{
  reg_base = nullptr;
}

bool pcie_dma_dev_t::load(reg_t addr, size_t len, uint8_t* bytes)
{
  int ch = 0;
  if (unlikely(!bytes || ((size()<addr+len)) || ((4!=len)))) {
    std::cout << "pcie_dma_dev_t: unsupported load register offset: " << hex << addr
        << " len: " << hex << len << std::endl;
    return false;
  }

  ch = addr / 0x20;
  if (PCIE_DMA_CH_TOTAL <= ch) {
    std::cout << "pcie_dma_dev_t: unsupported store register offset: " << hex << addr
        << " len: " << hex << len << std::endl;
    return false;
  }
  switch(addr%0x20) {
  case PCIEDMA_VERSION:
  case PCIEDMA_FEATURES:
  case PCIEDMA_DESC_L:
  case PCIEDMA_DESC_U:
  case PCIEDMA_ATTIBUTES:
  case PCIEDMA_CONTROL:
    memcpy(bytes, (uint8_t*)reg_base+addr, len);
    break;
  default:
    std::cout << "pcie_dma_dev_t: unsupported load register offset: " << hex << addr
        << " len: " << hex << len << std::endl;
    return false;
  }

  return true;
}

bool pcie_dma_dev_t::store(reg_t addr, size_t len, const uint8_t* bytes)
{
  int ch = 0;
  uint32_t val32 = 0;

  if (unlikely(!bytes || ((size()<addr+len)) || ((4!=len)))) {
    std::cout << "pcie_dma_dev_t: unsupported store register offset: " << hex << addr
        << " len: " << hex << len << std::endl;
    return false;
  }

  ch = addr / 0x20;
  if (PCIE_DMA_CH_TOTAL <= ch) {
    std::cout << "pcie_dma_dev_t: unsupported store register offset: " << hex << addr
        << " len: " << hex << len << std::endl;
    return false;
  }
  switch(addr%0x20) {
  /*
  case PCIEDMA_VERSION:
  case PCIEDMA_FEATURES:
  */
  case PCIEDMA_DESC_L:
  case PCIEDMA_DESC_U:
  case PCIEDMA_ATTIBUTES:
    memcpy((uint8_t*)reg_base+addr, bytes, len);
    break;
  case PCIEDMA_CONTROL:
    {
      bool clear_irq = false;
      val32 = *(uint32_t*)bytes;
      if (val32 & (1<<PCIEDMA_CTL_DONE_STAT)) {   /* done_status */
        val32 &= (~(1<<PCIEDMA_CTL_DONE_STAT));
        clear_irq = true;
      }
      if (val32 & (1<<PCIEDMA_CTL_ERR_STAT)) {    /* error_status */
        val32 &= (~(1<<PCIEDMA_CTL_ERR_STAT));
        clear_irq = true;
      }

      PCIEDMA_CTL(reg_base, ch) &= 0x81F00000;    /* mask */
      PCIEDMA_CTL(reg_base, ch) |= val32;

      if (clear_irq) {
        sim->mmio_load(SYSIRQ_BASE+SYSIRQ_INGRESS_IRQ_STS_ADDR2, 4, (uint8_t*)&val32);
        val32 &= (~(1<<STS_ADDR2_PCIE_DMA_BIT_CH(ch)));
        sim->mmio_store(SYSIRQ_BASE+SYSIRQ_INGRESS_IRQ_STS_ADDR2, 4, (uint8_t*)&val32);
      }

      if (PCIEDMA_CTL(reg_base, ch) & (1<<PCIEDMA_CTL_GO)) {    /* go */
        pcie_dma_go(ch);
      }
    }
    break;
  default:
    std::cout << "pcie_dma_dev_t: unsupported store register offset: " << hex << addr
        << " len: " << hex << len << std::endl;
    return false;
  }

  return true;
}

/* 写DDR, 成功返回0 */
int pcie_dma_dev_t::write_soc(uint64_t addr, uint8_t *data, int len)
{
  uint8_t *maddr = nullptr;

  maddr = (uint8_t*)sim->addr_to_mem((reg_t)addr);
  if (nullptr == maddr) {
    return -1;
  }
  memcpy(maddr, data, len);

  return 0;
}

/* 读DDR, 成功返回0 */
int pcie_dma_dev_t::read_soc(uint64_t addr, uint8_t *data, int len)
{
  uint8_t *maddr = nullptr;

  maddr = (uint8_t*)sim->addr_to_mem((reg_t)addr);
  if (nullptr == maddr) {
    return -1;
  }
  memcpy(data, maddr, len);

  return 0;
}

/* 通过netlink通信写drv内存, 成功返回0 */
int pcie_dma_dev_t::write_host(uint64_t addr, uint8_t *data, int len)
{
  int ret = 0;
  command_head_t cmd = {};

  cmd.code = CODE_WRITE_HOST;
  cmd.addr = addr;
  cmd.len = len;
  memcpy(cmd.data, data, len);
  ret = pcie->send((const uint8_t *)&cmd, PCIE_COMMAND_SEND_SIZE(cmd));
  if (0 < ret)
    return 0;
  else 
    return ret;
}

/* 通过netlink通信读drv内存, 成功返回0 */
int pcie_dma_dev_t::read_host(uint64_t addr, uint8_t *data, int len)
{
  int ret = 0;
  command_head_t cmd = {};

  cmd.code = CODE_READ_HOST;
  cmd.addr = addr;
  cmd.len = len;

  pcie->read_host_ready_to(&cmd);
  ret = pcie->send((const uint8_t *)&cmd, PCIE_COMMAND_SEND_SIZE(cmd));
  if (0 < ret) {
    pcie->read_host_wait();
    if ((addr==cmd.addr) && (len==cmd.len) && (CODE_READ_HOST==cmd.code)) {
      memcpy(data, cmd.data, len);
      ret = 0;
    } else {
      ret = -2;
    }
  }

  return ret;
}

/* 成功返回0 */
int pcie_dma_dev_t::pcie_dma_xfer(uint64_t soc, uint64_t pcie, int len, int ob_not_ib)
{
  int i = 0;
  int ret = 0;
  int is_finish = 0;
  int len_once = 0;
  uint64_t soc_addr = 0;
  uint64_t pcie_addr = 0;
  uint8_t *buf = nullptr;

  printf("%s soc %lx pcie %lx len %d dir %d \n", __FUNCTION__, soc, pcie, len, ob_not_ib);

  buf = (uint8_t*)malloc(XFER_LEN_ONCE_MAX);
  if (nullptr == buf) {
    return -1;
  }

  for (i = 0 ; i < len ; i += XFER_LEN_ONCE_MAX) {
    soc_addr = soc + i;
    pcie_addr = pcie + i;
    len_once = (i+XFER_LEN_ONCE_MAX > len) ? len-i : XFER_LEN_ONCE_MAX;
    memset(buf, 0, XFER_LEN_ONCE_MAX);

    ret = -1;
    if (XFER_DIR_H2D == ob_not_ib) {
        ret = read_host(pcie_addr, buf, len_once);
        if (0 == ret) {
          ret = write_soc(soc_addr, buf, len_once);
        }
    } else if (XFER_DIR_D2H == ob_not_ib) {
        ret = read_soc(soc_addr, buf, len_once);
        if (0 == ret) {
          ret = write_host(pcie_addr, buf, len_once);
        }
    }

    if (0 != ret) {
      goto copy_failed;
    }
  }
  is_finish = 1;

copy_failed:
  free(buf);
  if (1 != is_finish)
    return -2;
  return 0;
}

/* 解析寄存器和desc, 执行数据搬运 */
void pcie_dma_dev_t::pcie_dma_go(int ch)
{
  int i = 0;
  int ret = 0;
  int desc_len = 0;
  int ob_not_ib = 0;
  uint32_t val32 = 0;
  uint64_t desc_addr_reg = 0;
  uint8_t *desc_addr = nullptr;

  int xfer_len = 0;
  uint64_t soc_addr = 0;
  uint64_t pcie_addr = 0;

  /* control.go = 0 */
  PCIEDMA_CTL(reg_base, ch) &= (~(1<<PCIEDMA_CTL_GO));

  /* control.ready = 0 */
  PCIEDMA_CTL(reg_base, ch) &= (~(1<<PCIEDMA_CTL_READY));

  /* control.error_status = 0 */
  PCIEDMA_CTL(reg_base, ch) &= (~(1<<PCIEDMA_CTL_ERR_STAT));

  /* control.done_status = 0 */
  PCIEDMA_CTL(reg_base, ch) &= (~(1<<PCIEDMA_CTL_DONE_STAT));

  /* control.status_code = Running  */
  // PCIEDMA_CTL(reg_base, ch) &= (~(0x1f<<PCIEDMA_CTL_STATUS));
  // PCIEDMA_CTL(reg_base, ch) |= (2<<PCIEDMA_CTL_STATUS);

  desc_addr_reg = PCIEDMA_DESC(reg_base, ch);
  desc_len = PCIEDMA_CTL(reg_base, ch) & 0x1fff;
  ob_not_ib = (PCIEDMA_CTL(reg_base, ch) >> PCIEDMA_CTL_OB_NOT_IB) & 0x01;

  if (pcie_desc_atu && pcie_desc_atu->is_ipa_enabled()) {
    desc_addr_reg = pcie_desc_atu->translate(desc_addr_reg, 1);
  }
  
  printf("%s ch %d desc_len %d start ... \n", __FUNCTION__, ch, desc_len);

  if ( 0 == desc_len) {
    std::cout << "pcie_dma: control.length " << desc_len << " is error"  << std::endl;
    return;
  }

  for (i = 0 ; i < desc_len ; i++,desc_addr_reg+=PDD_ONE_DESC_LEN) {
    desc_addr = (uint8_t*)sim->addr_to_mem((reg_t)desc_addr_reg);
    if (nullptr == desc_addr) {
      std::cout << "pcie_dma_dev_t: desc addr %lx error " << hex << desc_addr_reg << std::endl;
      throw std::runtime_error("");
    }

    xfer_len = PDD_CTL_LEN(desc_addr) & 0xffffff;
    soc_addr = PDD_SOC_ADDR(desc_addr);
    pcie_addr = PDD_PCIE_ADDR(desc_addr);

    if (pcie_atu && pcie_atu->is_ipa_enabled()) {
      soc_addr = pcie_atu->translate(soc_addr, 1);
    }
    ret = pcie_dma_xfer(soc_addr, pcie_addr, xfer_len, ob_not_ib);
    if (0 != ret) {
      break;
    }
  }

  /* control.ready = 1 */
  PCIEDMA_CTL(reg_base, ch) |= (1<<PCIEDMA_CTL_READY);

  // /* status code idle */
  // PCIEDMA_CTL(reg_base, ch) &= (~(0x1f<<PCIEDMA_CTL_STATUS));

  if (0 == ret) {
    PCIEDMA_CTL(reg_base, ch) |= (1<<PCIEDMA_CTL_DONE_STAT);    /* control.done_status = 1 */
    printf("%s ch %d desc_len %d done \n", __FUNCTION__, ch, desc_len);
  } else {
    PCIEDMA_CTL(reg_base, ch) |= (1<<PCIEDMA_CTL_ERR_STAT);     /* control.error_status = 1 */
    printf("%s ch %d desc_len %d ret %d error \n", __FUNCTION__, ch, desc_len, ret);
  }

  /* raise irq */
  if (PCIEDMA_CTL(reg_base, ch) & (1<<PCIEDMA_CTL_DONE_IRQ_ENA) ||
      (PCIEDMA_CTL(reg_base, ch) & (1<<PCIEDMA_CTL_ERR_IRQ_ENA))) {
    printf("pcie_dma raise interrupt ch %d \n", ch);
    sim->mmio_load(SYSIRQ_BASE+SYSIRQ_INGRESS_IRQ_STS_ADDR2, 4, (uint8_t*)&val32);
    val32 |= (1<<STS_ADDR2_PCIE_DMA_BIT_CH(ch));
    sim->mmio_store(SYSIRQ_BASE+SYSIRQ_INGRESS_IRQ_STS_ADDR2, 4, (uint8_t*)&val32);
  }
}

axi_master_common_t::axi_master_common_t(uint8_t *regs) : reg_base(regs)
{
  int i = 0;

  if (NULL == reg_base) {
    throw std::invalid_argument("error! pcie_dma reg_base is nullptr");
  }
  memset(reg_base, 0, size());
}

axi_master_common_t::~axi_master_common_t(void)
{
  reg_base = nullptr;
}

bool axi_master_common_t::load(reg_t addr, size_t len, uint8_t* bytes)
{
  int ch = 0;
  if (unlikely(!bytes || ((size()<addr+len)))) {
    std::cout << "axi_master_common_t: unsupported load register offset: " << hex << addr
        << " len: " << hex << len << std::endl;
    return false;
  }

  memcpy(bytes, (uint8_t*)reg_base+addr, len);

  return true;
}

bool axi_master_common_t::store(reg_t addr, size_t len, const uint8_t* bytes)
{
  int ch = 0;
  uint32_t val32 = 0;

  if (unlikely(!bytes || ((size()<addr+len)))) {
    std::cout << "axi_master_common_t: unsupported store register offset: " << hex << addr
        << " len: " << hex << len << std::endl;
    return false;
  }

  memcpy((uint8_t*)reg_base+addr, bytes, len);

  return true;
}

/**
 * pcie 访存经过 bar窗口， 返回该窗口配置的基地只
 * pf: barid 0/4
 */
uint64_t axi_master_common_t::bar_axi_base_addr(int barid)
{
  uint32_t addr0 = 0;
  uint32_t addr1 = 0;

  if (4 < barid) {
    std::cout << "bar_axi_base_addr: unsupported barid :" << barid << std::endl;
    throw std::invalid_argument("");
  }

  addr0 = ((uint32_t*)reg_base)[barid*2];
  addr1 = ((uint32_t*)reg_base)[barid*2+1];

  return ((uint64_t)addr1 << 32) | addr0;
}

/* pcie bar2地址映射， 参考 芯片架构图 Memory Mapping from the view of PCIe Host */
uint64_t axi_master_common_t::pf_bar2_axi_addr(uint32_t bar_offset)
{
  uint64_t trans_addr = 0xc4000000 + bar_offset;

  struct axi_map_t {
    uint64_t trans_addr;
    uint64_t soc_addr;
    uint32_t size;
  };

  const struct axi_map_t pf_bar2_remapper[] = {
    {0xc4000000, SOC_APB_BASE,        0x80000},
    {0xc4080000, HWSYNC_START,        0x80000},
    {0xc410c000, P2AP_MBOX_LOC_BASE,  0x1000},
    {0xc410d000, N2AP_MBOX_LOC_BASE,  0x1000},
    {0xc4800000, SYSDMA0_BASE,        0x10000},
    {0xc4810000, SYSDMA1_BASE,        0x10000},
    {0xc4820000, SYSDMA2_BASE,        0x10000},
    {0xc4830000, SYSDMA3_BASE,        0x10000},
    {0xc4840000, SYSDMA4_BASE,        0x10000},
    {0xc4850000, SYSDMA5_BASE,        0x10000},
    {0xc4860000, SYSDMA6_BASE,        0x10000},
    {0xc4870000, SYSDMA7_BASE,        0x10000},
    {0xc4880000, SRAM_START,          0x80000},
    {0xc4900000, PCIE_CTL_CFG_BASE,   0x80000},
    {0xc4d00000, NOC_NPC0_BASE+NPC_SYS_OFFET,   0x10000},
    {0xc4d10000, NOC_NPC1_BASE+NPC_SYS_OFFET,   0x10000},
    {0xc4d20000, NOC_NPC2_BASE+NPC_SYS_OFFET,   0x10000},
    {0xc4d30000, NOC_NPC3_BASE+NPC_SYS_OFFET,   0x10000},
    {0xc4d40000, NOC_NPC4_BASE+NPC_SYS_OFFET,   0x10000},
    {0xc4d50000, NOC_NPC5_BASE+NPC_SYS_OFFET,   0x10000},
    {0xc4d60000, NOC_NPC6_BASE+NPC_SYS_OFFET,   0x10000},
    {0xc4d70000, NOC_NPC7_BASE+NPC_SYS_OFFET,   0x10000},
    {0xc4d80000, NOC_NPC8_BASE+NPC_SYS_OFFET,   0x10000},
    {0xc4d90000, NOC_NPC9_BASE+NPC_SYS_OFFET,   0x10000},
    {0xc4da0000, NOC_NPC10_BASE+NPC_SYS_OFFET,   0x10000},
    {0xc4db0000, NOC_NPC11_BASE+NPC_SYS_OFFET,   0x10000},
    {0xc4dc0000, NOC_NPC12_BASE+NPC_SYS_OFFET,   0x10000},
    {0xc4dd0000, NOC_NPC13_BASE+NPC_SYS_OFFET,   0x10000},
    {0xc4de0000, NOC_NPC14_BASE+NPC_SYS_OFFET,   0x10000},
    {0xc4df0000, NOC_NPC15_BASE+NPC_SYS_OFFET,   0x10000},
    {0xc4e00000, NOC_NPC16_BASE+NPC_SYS_OFFET,   0x10000},
    {0xc4e10000, NOC_NPC17_BASE+NPC_SYS_OFFET,   0x10000},
    {0xc4e20000, NOC_NPC18_BASE+NPC_SYS_OFFET,   0x10000},
    {0xc4e30000, NOC_NPC19_BASE+NPC_SYS_OFFET,   0x10000},
    {0xc4e40000, NOC_NPC20_BASE+NPC_SYS_OFFET,   0x10000},
    {0xc4e50000, NOC_NPC21_BASE+NPC_SYS_OFFET,   0x10000},
    {0xc4e60000, NOC_NPC22_BASE+NPC_SYS_OFFET,   0x10000},
    {0xc4e70000, NOC_NPC23_BASE+NPC_SYS_OFFET,   0x10000},
    {0xc4e80000, NOC_NPC24_BASE+NPC_SYS_OFFET,   0x10000},
    {0xc4e90000, NOC_NPC25_BASE+NPC_SYS_OFFET,   0x10000},
    {0xc4ea0000, NOC_NPC26_BASE+NPC_SYS_OFFET,   0x10000},
    {0xc4eb0000, NOC_NPC27_BASE+NPC_SYS_OFFET,   0x10000},
    {0xc4ec0000, NOC_NPC28_BASE+NPC_SYS_OFFET,   0x10000},
    {0xc4ed0000, NOC_NPC29_BASE+NPC_SYS_OFFET,   0x10000},
    {0xc4ee0000, NOC_NPC30_BASE+NPC_SYS_OFFET,   0x10000},
    {0xc4ef0000, NOC_NPC31_BASE+NPC_SYS_OFFET,   0x10000}
  };

  if (0xc5000000 <= trans_addr) {
    std::cout << "pf_bar2_axi_addr: unsupported translated address " << hex << trans_addr << std::endl;
    throw std::invalid_argument("");
  }

  for (int i = 0 ; i < int(sizeof(pf_bar2_remapper)/sizeof(pf_bar2_remapper[0])) ; i++) {
    if (pf_bar2_remapper[i].trans_addr<=trans_addr && 
        pf_bar2_remapper[i].trans_addr+pf_bar2_remapper[i].size>trans_addr) {
      return  trans_addr - pf_bar2_remapper[i].trans_addr + pf_bar2_remapper[i].soc_addr;
    }
  }

  std::cout << "pf_bar2_axi_addr: unsupported translated address " << hex << trans_addr << std::endl;
  throw std::invalid_argument("");

  return 0;
}

/* pcie滑窗地址， 转换到 axi(noc)地址 */
uint64_t axi_master_common_t::bar_axi_addr(int barid, uint32_t bar_offset)
{
  if (2 == barid) {
    return pf_bar2_axi_addr(bar_offset);
  } else {
    return bar_axi_base_addr(barid) + bar_offset;
  }
}
