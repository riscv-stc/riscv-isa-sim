#include <iostream>
#include "../Transport/AbstractProxy.h"
#include "devices.h"
#include "mmu.h"
#include "processor.h"
#include "simif.h"

// offset of dma registers
#define DMA_CTLR_OFFSET 0X000
#define DMA_CENB_OFFSET 0X004
#define DMA_CDIS_OFFSET 0X008
#define DMA_CPRI_OFFSET 0X00C
#define DMA_CSTAT_OFFSET 0X010
#define DMA_CISR_OFFSET 0X014
#define DMA_CIER_OFFSET 0X018
#define DMA_CABR_OFFSET 0X01C
#define DMA_BUF_OFFSET 0X020
#define DMA_C0_CTLR_OFFSET 0X100
#define DMA_C0_DSAR_OFFSET 0X104
#define DMA_C0_BKMR0_OFFSET 0X10C
#define DMA_C0_BKMR1_OFFSET 0X110
#define DMA_C0_LLPR_OFFSET 0X114
#define DMA_C1_CTLR_OFFSET 0X200
#define DMA_C1_DSAR_OFFSET 0X204
#define DMA_C1_BKMR0_OFFSET 0X20C
#define DMA_C1_BKMR1_OFFSET 0X210
#define DMA_C1_LLPR_OFFSET 0X214

// base of DDR address
static uint64_t dma_ddr_base[] = {
  0x800000000, // DMA0
  0x800000000, // DMA1
  0x900000000, // DMA2
  0x900000000, // DMA3
  0xA00000000, // DMA4
  0xA00000000, // DMA5
  0xB00000000, // DMA6
  0xB00000000, // DMA7
};

/**
 * @brief constructor
 */
sysdma_device_t::sysdma_device_t(int dma_idx, std::vector<processor_t*>& procs)
    : procs_(procs) {
  // create thread for each dma channel
  for (int ch = 0; ch < DMA_MAX_CHANNEL_NUMER; ch++) {
    auto func = std::bind(&sysdma_device_t::dma_core, this, ch);
    this->dma_ch_thread_[ch] = std::thread(func);
    this->dma_ch_thread_[ch].detach();
  }

  // dma feature
  dma_enabled_ = false;
  for (int i = 0; i < DMA_MAX_CHANNEL_NUMER; i++) {
    dma_channel_[i].enabled = false;
    dma_channel_[i].desc_mode_enabled = false;
    dma_channel_[i].llp = 0;
    if(dma_idx >= sizeof(dma_ddr_base)/sizeof(dma_ddr_base[0]))
        throw dma_idx;
    dma_channel_[i].ddr_base = dma_ddr_base[dma_idx];
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
  Transport::AbstractProxy* proxy = nullptr;

  while (1) {
    std::unique_lock<std::mutex> lock(thread_lock_[ch]);
    thread_cond_[ch].wait(lock);

    if (proxy == nullptr) {
      proxy = procs_[0]->get_proxy();
    }
    if (proxy == nullptr) {
      std::cout << "sysdma: fail to get proxy" << std::endl;
      return;
    }

    while (dma_channel_[ch].llp) {
      if(dma_channel_[ch].llp < SYSDMA0_BASE + DMA_BUF_OFFSET ||
              dma_channel_[ch].llp >= SYSDMA0_BASE + DMA_BUF_OFFSET+DMA_BUF_SIZE)
          throw std::runtime_error("sysdma:wrong llp");

      // FIXME, need to check MCU/TCP
      procs_[0]->get_sim()->set_aunit(MCU, 0);
      struct dma_desc_t* desc = (struct dma_desc_t*)&dma_buf_[dma_channel_[ch].llp - SYSDMA0_BASE - DMA_BUF_OFFSET];

      // std::cout << "desc:" << hex << desc << "desc next:" << desc->llpr
      //           << "sysdma: ctlr:" << desc->ctlr.full << std::endl;

      if (desc->ctlr.bits.desc_en == 0) {
        std::cout << "sysdma: desc is disabled" << std::endl;
        dma_channel_[ch].llp = desc->llpr;
        continue;
      }
      direction_t direction;

#define LLB_BUFFER_START 0xf8000000
#define LLB_BUFFER_END 0xf8800000
      // only do simple address check
      if (desc->ddar >= LLB_BUFFER_START && desc->ddar <= LLB_BUFFER_END)
        direction = SYSDMA_DDR2LLB;
      else if (desc->dsar >= LLB_BUFFER_START && desc->dsar <= LLB_BUFFER_END)
        direction = SYSDMA_LLB2DDR;
      else
        std::cout << "sysdma: wrong direction" << std::endl;

      switch (direction) {
        // dma transfer from ddr to llb
        case SYSDMA_DDR2LLB: {
          auto dst = desc->ddar - LLB_BUFFER_START;
          auto src = desc->dsar;
          unsigned int col = desc->bkmr1.bits.width_high<<16 | desc->bkmr0.bits.width;
          unsigned int row = desc->bkmr0.bits.height;
          unsigned int stride = desc->bkmr1.bits.stride;

          // std::cout << "dst:" << hex << (unsigned int)dst << "src:" << hex <<
          // (unsigned int)desc->dsar << "col:" << col/2 << "row:" << row <<
          // "stride:" << stride << std::endl;
          for (int times = 0; times < 5; times++) {
            if (proxy->dmaXfer(src, dst, Transport::AbstractProxy::DDR2LLB,
                               col / 2,  // FIXME: sew
                               row, stride))
              break;
          }
          break;
        }

        // dma transfer from llb to ddr
        case SYSDMA_LLB2DDR: {
          auto dst = desc->ddar;
          auto src = desc->dsar - LLB_BUFFER_START;
          unsigned int col = desc->bkmr1.bits.width_high<<16 | desc->bkmr0.bits.width;
          unsigned int row = desc->bkmr0.bits.height;
          unsigned int stride = desc->bkmr1.bits.stride;

          for (int times = 0; times < 5; times++) {
            if (proxy->dmaXfer(dst, src, Transport::AbstractProxy::LLB2DDR,
                               col / 2, row, stride))
              break;
          }

          break;
        }
      }

      dma_channel_[ch].llp = desc->llpr;
    }

    proxy->dmaPoll();
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
      if (dma_channel_[0].xfer_complete) ret |= 0x1;
      if (dma_channel_[1].xfer_complete) ret |= 0x4;
      *((uint32_t*)bytes) = ret;
      return true;
    }

    // get status of dma channel busy
    case DMA_CSTAT_OFFSET: {
      uint32_t ret = 0;
      if (dma_channel_[0].busy) ret |= 0x1;
      if (dma_channel_[1].busy) ret |= 0x2;
      *((uint32_t*)bytes) = ret;
      return true;
    }

    default:
      if (DMA_BUF_OFFSET<=addr<=(DMA_BUF_OFFSET+DMA_BUF_SIZE-len)){
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
      if (DMA_BUF_OFFSET<=addr<=(DMA_BUF_OFFSET+DMA_BUF_SIZE-len)){
          *((uint32_t *)&dma_buf_[addr-DMA_BUF_OFFSET]) = val;
      } else {
          std::cout << "sysdma: unsupported store register offset: " << addr
                << std::endl;
      }
      break;
  }

  return true;
}
