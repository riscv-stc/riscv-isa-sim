#include <iostream>
#include "devices.h"
#include "mmu.h"
#include "processor.h"
#include "simif.h"
#include "encoding.h"
#include "unistd.h"

// offset of dma registers
#define DMA_CTLR_OFFSET 0X000
#define DMA_CENB_OFFSET 0X004
#define DMA_CDIS_OFFSET 0X008
#define DMA_CPRI_OFFSET 0X00C
#define DMA_CSTAT_OFFSET 0X010
#define DMA_CISR_OFFSET 0X014
#define DMA_CIER_OFFSET 0X018
#define DMA_CABR_OFFSET 0X01C
#define DMA_BUF_OFFSET 0X1000
#define DMA_C0_CTLR_OFFSET 0X100
#define DMA_C0_DSAR_OFFSET 0X104
#define DMA_C0_BKMR0_OFFSET 0X10C
#define DMA_C0_BKMR1_OFFSET 0X110
#define DMA_C0_LLPR_OFFSET 0X114
#define DMA_C0_PCNT_OFFSET 0X118
#define DMA_C1_CTLR_OFFSET 0X200
#define DMA_C1_DSAR_OFFSET 0X204
#define DMA_C1_BKMR0_OFFSET 0X20C
#define DMA_C1_BKMR1_OFFSET 0X210
#define DMA_C1_LLPR_OFFSET 0X214
#define DMA_C1_PCNT_OFFSET 0X218

/**
 * @brief constructor
 */
sysdma_device_t::sysdma_device_t(int dma_idx, std::vector<processor_t*>& procs)
    : procs_(procs) {
  // dma feature
  dma_enabled_ = false;
  for (int i = 0; i < DMA_MAX_CHANNEL_NUMER; i++) {
    dma_channel_[i].enabled = false;
    dma_channel_[i].desc_mode_enabled = false;
    dma_channel_[i].llp = 0;
    dma_channel_[i].ddr_base[DDR_DIR_SRC] = 0;
    dma_channel_[i].ddr_base[DDR_DIR_DST] = 0;
  }

  // create thread for each dma channel
  for (int ch = 0; ch < DMA_MAX_CHANNEL_NUMER; ch++) {
    dma_ch_thread_[ch] = std::thread([this, ch] { dma_core(ch); });
    dma_ch_thread_[ch].detach();
  }
}

/**
 * @brief destructor
 */
sysdma_device_t::~sysdma_device_t() {}

/**
 * @brief dma core function to transfer data between LLB and DDR
 */
void sysdma_device_t::dma_core(int ch) {
  while (1) {
    std::unique_lock<std::mutex> lock(thread_lock_[ch], std::defer_lock);
    while (!dma_channel_[ch].enabled)
      thread_cond_[ch].wait(lock);

    while (dma_channel_[ch].llp) {
      if(dma_channel_[ch].llp < SYSDMA0_BASE + DMA_BUF_OFFSET ||
              dma_channel_[ch].llp >= SYSDMA0_BASE + DMA_BUF_OFFSET+DMA_BUF_SIZE)
          throw std::runtime_error("sysdma:wrong llp");

      struct dma_desc_t* desc = (struct dma_desc_t*)&dma_buf_[dma_channel_[ch].llp - SYSDMA0_BASE - DMA_BUF_OFFSET];

      // std::cout << "desc:" << hex << desc << "desc next:" << desc->llpr
      //           << "sysdma: ctlr:" << desc->ctlr.full << std::endl;

      if (desc->ctlr.bits.desc_en == 0) {
        std::cout << "sysdma: desc is disabled" << std::endl;
        dma_channel_[ch].llp = desc->llpr;
        continue;
      }

      // for linear mode, row is 1, col is xfer len, stride is 0
      unsigned int col = desc->ctlr.bits.blk_en?
                  desc->bkmr1.bits.width_high<<16 | desc->bkmr0.bits.width : desc->ctlr.bits.xfer_len;
      unsigned int row =  desc->ctlr.bits.blk_en? desc->bkmr0.bits.height : 1;
      unsigned int stride =  desc->ctlr.bits.blk_en? desc->bkmr1.bits.stride : 0;

      if(stride && stride < col)
        throw std::runtime_error("stride is smaller than col");

      simif_t *sim = procs_[0]->get_sim();
      char *dst = sim->addr_to_mem(desc->ddar);
      char *src = sim->addr_to_mem(desc->dsar);

      if (stride == 0) {
        memcpy(dst, src, col * row);
      } else {
        for (int i = 0; i < row; i++) {
          memcpy(dst + i * col, src + i * stride, col);
        }
      }

      dma_channel_[ch].llp = desc->llpr;
    }

    // Q&A: one complete status include both channels?
    dma_channel_[ch].xfer_complete = true;
    // clear context
    dma_channel_[ch].enabled = false;
    dma_channel_[ch].busy = false;
  }
}

/**
 * @brief device load func
 */
bool sysdma_device_t::load(reg_t addr, size_t len, uint8_t* bytes) {
  if (len != 4) {
    std::cout << "sysdma: load wrong len:" << len << std::endl;
  }

  switch (addr) {
    // get status of dma transfer
    case DMA_CISR_OFFSET: {
      uint32_t ret = 0;
      std::unique_lock<std::mutex> lock0(thread_lock_[0], std::defer_lock);
      if (lock0.try_lock()) {
        if (dma_channel_[0].xfer_complete) {
          ret |= 0x1;
        }
        lock0.unlock();
      }
      std::unique_lock<std::mutex> lock1(thread_lock_[1], std::defer_lock);
      if (lock1.try_lock()) {
        if (dma_channel_[1].xfer_complete) {
          ret |= 0x4;
        }
        lock1.unlock();
      }
      *((uint32_t*)bytes) = ret;
      return true;
    }

    // get status of dma channel busy
    case DMA_CSTAT_OFFSET: {
      uint32_t ret = 0;
      std::unique_lock<std::mutex> lock0(thread_lock_[0], std::defer_lock);
      if (lock0.try_lock()) {
        if (dma_channel_[0].busy) {
          ret |= 0x1;
        }
        lock0.unlock();
      } else {
        ret |= 0x1;
      }
      std::unique_lock<std::mutex> lock1(thread_lock_[1], std::defer_lock);
      if (lock1.try_lock()) {
        if (dma_channel_[1].busy) {
          ret |= 0x2;
        }
        lock1.unlock();
      } else {
        ret |= 0x2;
      }
      *((uint32_t*)bytes) = ret;
      return true;
    }

    //DMA performance counter, fake return 0
    case DMA_C0_PCNT_OFFSET:
    case DMA_C1_PCNT_OFFSET:
      *((uint32_t*)bytes) = 0;
      return true;

    default:
      if (addr >=DMA_BUF_OFFSET && addr<=(DMA_BUF_OFFSET+DMA_BUF_SIZE-len)){
          *((uint32_t*)bytes) = *((uint32_t *)&dma_buf_[addr-DMA_BUF_OFFSET]);
      }else {
          std::cout << "sysdma: unsupported load address " << addr << std::endl;
      }
      break;
  }
  return true;
}

/**
 * @brief device store func
 */
bool sysdma_device_t::store(reg_t addr, size_t len, const uint8_t* bytes) {
  if (len != 4) {
    std::cout << "sysdma: store wrong len:" << len << std::endl;
  }

  uint32_t val = *((uint32_t*)bytes);

  // std::cout << "reg offset:" << hex << addr << "  val:" << val << std::endl;

  // index of dma channel
  int ch = 0;

  // offset of register
  // currently, only handle base feature of registers
  switch (addr) {
    // dma controller enabled
    case DMA_CTLR_OFFSET:
      if (val & 0x1) dma_enabled_ = true;
      break;

    // dma channel enabled
    case DMA_CENB_OFFSET: {
      // start xfer in channnel 0
      if (val & 0x1) {
        ch = 0;
        std::unique_lock<std::mutex> lock(thread_lock_[ch], std::defer_lock);
        if (lock.try_lock()) {
          dma_channel_[ch].enabled = true;
          dma_channel_[ch].busy = true;
          thread_cond_[ch].notify_all();
        } else
          std::cout << "sysdma: enable chanel 0 in working state" << std::endl;
      }

      // start xfer in channel 1
      if (val & 0x2) {
        ch = 1;
        std::unique_lock<std::mutex> lock(thread_lock_[ch], std::defer_lock);
        if (lock.try_lock()) {
          dma_channel_[ch].enabled = true;
          dma_channel_[ch].busy = true;
          thread_cond_[ch].notify_all();
        } else
          std::cout << "sysdma: enable chanel 1 in working state" << std::endl;
      }
      break;
    }

    // DMA Channel Interrupt Enable Register
    case DMA_CIER_OFFSET:
      if (val != 0) {
        std::cout << "sysdma: can't handle interrupt " << std::endl;
      }
      break;

    // DMA Channel Interrupt Status Registe
    case DMA_CISR_OFFSET:
      // clear interrupt pending status
      if (val & 0x1) {
        ch = 0;
        dma_channel_[ch].xfer_complete = false;
      }

      if (val & 0x4) {
        ch = 1;
        dma_channel_[ch].xfer_complete = false;
      }
      break;

    case DMA_CABR_OFFSET:
      // don't set ddr base address, since local address is enough
      // dma_channel_[SYSDMA_CHAN0].ddr_base[DDR_DIR_SRC] = (uint64_t)(val&0xff) << 32;
      // dma_channel_[SYSDMA_CHAN0].ddr_base[DDR_DIR_DST] = (uint64_t)(val&0xff00) << 24;
      // dma_channel_[SYSDMA_CHAN1].ddr_base[DDR_DIR_SRC] = (uint64_t)(val&0xff0000) << 16;
      // dma_channel_[SYSDMA_CHAN1].ddr_base[DDR_DIR_DST] = (uint64_t)(val&0xff000000) << 8;
      break;

    // DMA Channel x Control Register
    case DMA_C1_CTLR_OFFSET:
      ch = 1;
    case DMA_C0_CTLR_OFFSET: {
      // Descriptor Mode Enable
      if (val == 0x80000000) {
        std::unique_lock<std::mutex> lock(thread_lock_[ch], std::defer_lock);
        if (lock.try_lock())
          dma_channel_[ch].desc_mode_enabled = true;
        else
          std::cout << "sysdma: fail to set descriptor mode, since channel"
                    << ch << " in working state." << std::endl;
      } else
        std::cout << "sysdma: only support descriptor mode" << std::endl;
      break;
    }

    // DDMA Channel x Link List Pointer Registers
    case DMA_C1_LLPR_OFFSET:
      ch = 1;
    case DMA_C0_LLPR_OFFSET: {
      // FIXME: check the data is not 0x0
      std::unique_lock<std::mutex> lock(thread_lock_[ch], std::defer_lock);
      if (lock.try_lock()) {
        dma_channel_[ch].llp = val;
        if (dma_channel_[ch].llp == 0)
          std::cout << "sysdma: llp is null" << std::endl;
      } else
        std::cout << "sysdma: fail to set llp, since channel" << ch
                  << " in working state." << std::endl;
      break;
    }

    default:
      if (addr >= DMA_BUF_OFFSET && addr<=(DMA_BUF_OFFSET+DMA_BUF_SIZE-len)){
          *((uint32_t *)&dma_buf_[addr-DMA_BUF_OFFSET]) = val;
      } else {
          std::cout << "sysdma: unsupported store register offset: " << addr
                << std::endl;
      }
      break;
  }

  return true;
}
