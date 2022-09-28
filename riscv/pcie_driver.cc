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

pcie_driver_t::pcie_driver_t(simif_t* sim, bankif_t *bank, uint32_t bank_id, bool pcie_enabled, 
        size_t board_id, size_t chip_id) : mPSim(sim), mBank(bank), mBankId(bank_id),
        pcie_enabled(pcie_enabled), board_id(board_id), chip_id(chip_id)
{
  mStatus = PCIE_UNINIT;
  mDev = -1;

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
  //mSendBuffer->nlmsg_pid = NL_START_PORT + mBankId;
  mSendBuffer->nlmsg_pid = NL_NETLINK_START_PORT(board_id, chip_id) + mBankId;
  mSendBuffer->nlmsg_flags = NL_GROUPS;

  mRecvBuffer = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
  if (!mRecvBuffer) {
    std::cout << "driver init malloc recv buffer error!" << std::endl;
    return;
  }

  memset(mRecvBuffer, 0, sizeof(NLMSG_SPACE(MAX_PAYLOAD)));
  
  #if 0 /* 因为引入了pcie_mbox,sys_soc等, 此处需向后调整 */
  /* Recheck netlink status */
  if (NETLINK_FAULT == update_status(STATUS_OK))
    mStatus = ERROR_CONN;
  #endif
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
  //mSrcAddr.nl_pid = NL_START_PORT + mBankId;
  mSrcAddr.nl_pid = NL_NETLINK_START_PORT(board_id, chip_id) + mBankId;
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

  memset(pathname, 0, sizeof(pathname));
  sprintf(pathname, "/proc/stc/stc_cluster_%d", mBankId);
  if (access(pathname, F_OK)) {
    std::cout << "Cluster["
              << mBankId
              << "] dev is not exist, please check driver."
              << std::endl;
    return false;
  }

  mDev = open(pathname, O_RDWR);
  if (mDev < 0)
    return false;

  rc = write(mDev, magic_str, magic_len);
  if (rc != magic_len)
    return false;
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
   cmd.addr = mBankId;
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
    }

    if ((host_addr) || (host_addr=mBank->bank_addr_to_mem(addr)) ||
            (host_addr=mPSim->addr_to_mem(addr))) {
        memcpy(bytes, host_addr, len);
    } else if (!((mBank->npc_mmio_load(paddr, len, bytes,idxinbank)) ||
            (mBank->bank_mmio_load(addr, len, bytes)) ||
            (mPSim->mmio_load(addr, len, bytes)))) {
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
    }

    if ((host_addr) || (host_addr=mBank->bank_addr_to_mem(addr)) ||
            (host_addr=mPSim->addr_to_mem(addr))) {
        memcpy(host_addr, bytes, len);
    } else if (!((mBank->npc_mmio_store(paddr, len, bytes,idxinbank)) ||
            (mBank->bank_mmio_store(addr, len, bytes)) ||
            (mPSim->mmio_store(addr, len, bytes)))) {
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

/* sync state addr, just only read,
 * not realy phy_addr, just valid for PCIe dummy driver. */
#define PCIE_SYNC_STATE_ADDR    (0xc07f3504)
int pcie_driver_t::get_sync_state()
{
  int rv;
  uint32_t state = 0;
  command_head_t cmd;

  cmd.code = CODE_READ;
  cmd.addr = PCIE_SYNC_STATE_ADDR;
  cmd.len = 4;

  for (reg_t i = 0; i < mPSim->nprocs(); i++)
    if ((mPSim->get_core_by_idxinsim(i)->get_hwsync_status() & (1 << (mBankId * 8 + i))) == 0)
      state |= 0x1 << mPSim->get_core_by_idxinsim(i)->id;

  /* each bank core_id in spike is start at 0. */
  *(uint32_t *)cmd.data = state;
  rv = send((const uint8_t *)&cmd, PCIE_COMMAND_SEND_SIZE(cmd));
  return rv;
}

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
            case PCIE_CORE_RESET_ADDR:
              value = *(uint32_t *)pCmd->data;
              /* each bank in spike core_id is start at 0. */
              mPSim->hart_reset(value);
              break;

            default:
                    store_data(pCmd->addr, pCmd->len, (const uint8_t*)pCmd->data);
          }
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

  if (mDev != -1) {
    close(mDev);
    mDev = -1;
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

pcie_ctl_device_t::pcie_ctl_device_t(simif_t *_sim, pcie_driver_t *_pcie) : sim(_sim), pcie(_pcie)
{
  reg_base = (uint8_t *)malloc(len);

  pcie_dma = new pcie_dma_dev_t((uint8_t *)reg_base + PCIE_DMA_OFFSET, sim, pcie);
}

pcie_ctl_device_t::~pcie_ctl_device_t(void)
{
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

  /* pcie_dma */
  if (PCIE_DMA_OFFSET<=addr && (PCIE_DMA_OFFSET+PCIE_DMA_SIZE>addr)) {
    pcie_dma->load(addr-PCIE_DMA_OFFSET, len, bytes);
  } else {
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

  /* pcie_dma */
  if (PCIE_DMA_OFFSET<=addr && (PCIE_DMA_OFFSET+PCIE_DMA_SIZE>addr)) {
    pcie_dma->store(addr-PCIE_DMA_OFFSET, len, bytes);
  } else {
    memcpy((uint8_t*)reg_base+addr, bytes, len);
  }

  return true;
}

pcie_dma_dev_t::pcie_dma_dev_t(uint8_t *pcie_dma_regs, simif_t *_sim,
    pcie_driver_t *_pcie) : sim(_sim), pcie(_pcie), reg_base(pcie_dma_regs)
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

  printf("%s src %lx dst %lx len %d dir %d \n", __FUNCTION__, soc, pcie, len, ob_not_ib);

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
    printf("%s ch %d desc_len %d error \n", __FUNCTION__, ch, desc_len);
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


