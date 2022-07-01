#include <iostream>
#include "devices.h"
#include "sysdma.h"
#include "mmu.h"
#include "simif.h"
#include "encoding.h"
#include "unistd.h"
#include "arith.h"

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

#define SYS_DMAX_C0_SMMU_OFFSET            0x2000
#define SYS_DMAX_C1_SMMU_OFFSET            0x3000
#define SYS_DMAX_C0_ATU_OFFSET             0x4000
#define SYS_DMAX_C1_ATU_OFFSET             0x5000

#define SYS_DMAX_SMMU_SIZE          0x1000
#define SYS_DMAX_C1_ATU_SIZE        0x1000
#define SYS_DMAX_C1_ATU_END         (SYS_DMAX_C1_ATU_OFFSET+SYS_DMAX_C1_ATU_SIZE)
#define SYS_DMAX_DMMY_OFFSET        0x8000
#define SYS_DMAX_DMMY_SIZE          0x2000
#define SYS_DMAX_DMMY_END           (SYS_DMAX_DMMY_OFFSET+SYS_DMAX_DMMY_SIZE)

/* smmu register */
#define SMMU_SATP_REG               0x000
#define SMMU_TLB_FL_REG             0x100
#define SMMU_PAGE_TABLE_FAULT_REG   0x200

static reg_t sysdma_base[] = {
	SYSDMA0_BASE,
	SYSDMA1_BASE,
	SYSDMA2_BASE,
	SYSDMA3_BASE,
	SYSDMA4_BASE,
	SYSDMA5_BASE,
	SYSDMA6_BASE,
	SYSDMA7_BASE,
};

/**
 * @brief constructor
 */
sysdma_device_t::sysdma_device_t(int dma_idx, simif_t *sim, bankif_t *bank, const char *atuini)
    : sim(sim),bank(bank) {
  // dma feature
  dma_enabled_ = false;
  dma_idx_ = dma_idx;
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

  smmu[0] = new smmu_t(sim, bank, (uint8_t *)sys_dma_reg + SYS_DMAX_C0_SMMU_OFFSET);
  smmu[1] = new smmu_t(sim, bank, (uint8_t *)sys_dma_reg + SYS_DMAX_C1_SMMU_OFFSET);

  atu[0] = new atu_t(atuini, bank->get_bankid()*2 + dma_idx_, 0, (uint8_t *)sys_dma_reg + SYS_DMAX_C0_ATU_OFFSET);
  atu[1] = new atu_t(atuini, bank->get_bankid()*2 + dma_idx_, 1, (uint8_t *)sys_dma_reg + SYS_DMAX_C1_ATU_OFFSET);

  atu[0]->reset();
  atu[1]->reset();
}

/**
 * @brief destructor
 */
sysdma_device_t::~sysdma_device_t()
{
    delete smmu[0];
    delete smmu[1];

    delete atu[0];
    delete atu[1];
}

/**
 * @brief dma core function to transfer data between LLB and DDR
 */
void sysdma_device_t::dma_core(int ch) {
  while (1) {
    std::unique_lock<std::mutex> lock(thread_lock_[ch], std::defer_lock);
    while (!dma_channel_[ch].enabled)
      thread_cond_[ch].wait(lock);

    while (dma_channel_[ch].llp) {
      if(dma_channel_[ch].llp < sysdma_base[dma_idx_] + DMA_BUF_OFFSET ||
              dma_channel_[ch].llp >= sysdma_base[dma_idx_] + DMA_BUF_OFFSET+DMA_BUF_SIZE)
          throw std::runtime_error("sysdma:wrong llp");

      struct dma_desc_t* desc = (struct dma_desc_t*)&sys_dma_reg[dma_channel_[ch].llp - sysdma_base[dma_idx_]];

      // std::cout << "desc:" << hex << desc << "desc next:" << desc->llpr
      //           << "sysdma: ctlr:" << desc->ctlr.full << std::endl;

      if (desc->ctlr.bits.desc_en == 0) {
        std::cout << "sysdma: desc is disabled" << std::endl;
        dma_channel_[ch].llp = desc->llpr;
        continue;
      }

      // for linear mode, row is 1, col is xfer len, stride is 0
      // xfer_len 0 for 4M Bytes, width 0 for 4M Bytes, bkmr0.bits.height 0 for 65536 rows
      unsigned int col = 0, width = 0;

      if (desc->ctlr.bits.blk_en) {
        width = desc->bkmr1.bits.width_high<<16 | desc->bkmr0.bits.width;
        col = width ? width : 0x400000;
      } else {
        col = desc->ctlr.bits.xfer_len ? desc->ctlr.bits.xfer_len : 0x400000;
      }

      unsigned int row =  desc->ctlr.bits.blk_en? (desc->bkmr0.bits.height ? desc->bkmr0.bits.height : 65536) : 1;
      unsigned int stride =  desc->ctlr.bits.blk_en? desc->bkmr1.bits.stride : 0;

      if(stride && stride < col)
        throw std::runtime_error("stride is smaller than col");

      char *dst = nullptr;
      char *src = nullptr;

      if (!((dst=bank->bank_addr_to_mem(desc->ddar)) || (dst=sim->addr_to_mem(desc->ddar)))) {
        throw std::runtime_error("dma_core() addr error");
      }
      if (!((src=bank->bank_addr_to_mem(desc->dsar)) || (src=sim->addr_to_mem(desc->dsar)))) {
        throw std::runtime_error("dma_core() addr error");
      }

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
  if (((SYS_DMAX_DMMY_END)<addr) || ((SYS_DMAX_C1_ATU_END<addr) && (SYS_DMAX_DMMY_OFFSET>addr))) {
    std::cout << "sysdma: unsupported load register offset: " << addr
            << std::endl;
    return false;
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

    case DMA_CABR_OFFSET:
      *((uint32_t*)bytes) = (dma_channel_[SYSDMA_CHAN0].ddr_base[DDR_DIR_SRC] >> 32) & 0xff;
      *((uint32_t*)bytes) |= (dma_channel_[SYSDMA_CHAN0].ddr_base[DDR_DIR_DST] >> 24) & 0xff00;
      *((uint32_t*)bytes) |= (dma_channel_[SYSDMA_CHAN1].ddr_base[DDR_DIR_SRC] >> 16) & 0xff0000;
      *((uint32_t*)bytes) |= (dma_channel_[SYSDMA_CHAN1].ddr_base[DDR_DIR_DST] >> 8) & 0xff000000;

      break;

    default:
      if (addr >=DMA_BUF_OFFSET && addr<=(DMA_BUF_OFFSET+DMA_BUF_SIZE-len)){
          memcpy(bytes, (uint8_t*)sys_dma_reg + addr, len);
      }else {
          memcpy(bytes, (uint8_t*)sys_dma_reg + addr, len);
      }
      break;
  }
  return true;
}

/**
 * @brief device store func
 */
bool sysdma_device_t::store(reg_t addr, size_t len, const uint8_t* bytes) {
  if (((SYS_DMAX_DMMY_END)<addr) || ((SYS_DMAX_C1_ATU_END<addr) && (SYS_DMAX_DMMY_OFFSET>addr))) {
    std::cout << "sysdma: unsupported store register offset: " << addr
            << std::endl;
    return false;
  }

  uint32_t val = *((uint32_t*)bytes);

  // std::cout << "reg offset:" << hex << addr << "  val:" << val << std::endl;

  // index of dma channel
  int ch = 0;
  
  if (SYS_DMAX_C0_SMMU_OFFSET > addr) {         /* SYS_DMAX + SYS_DMAX_DESC */
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
        dma_channel_[SYSDMA_CHAN0].ddr_base[DDR_DIR_SRC] = (uint64_t)(val&0xff) << 32;
        dma_channel_[SYSDMA_CHAN0].ddr_base[DDR_DIR_DST] = (uint64_t)(val&0xff00) << 24;
        dma_channel_[SYSDMA_CHAN1].ddr_base[DDR_DIR_SRC] = (uint64_t)(val&0xff0000) << 16;
        dma_channel_[SYSDMA_CHAN1].ddr_base[DDR_DIR_DST] = (uint64_t)(val&0xff000000) << 8;
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
            memcpy((uint8_t*)sys_dma_reg + addr, bytes, len);
        } else {
            memcpy((uint8_t*)sys_dma_reg + addr, bytes, len);
        }
        break;
    }
  } else if (((SYS_DMAX_C0_SMMU_OFFSET<=addr) && (SYS_DMAX_C1_SMMU_OFFSET>addr+len)) ||
        ((SYS_DMAX_C1_SMMU_OFFSET<=addr) && (SYS_DMAX_C0_ATU_OFFSET>addr+len))) {   /* SMMU */
    ;
  } else if (((SYS_DMAX_C0_ATU_OFFSET<=addr) && (SYS_DMAX_C1_ATU_OFFSET>addr+len)) ||
        ((SYS_DMAX_C1_ATU_OFFSET<=addr) && (SYS_DMAX_C1_ATU_END>addr+len))) {       /* ATU */
    ;
  } else {
    std::cout << "sysdma: unsupport store address 0x" << hex << addr << std::endl;
  }
  return true;
}

void sysdma_device_t::dmae_atu_trap(reg_t paddr, int channel, processor_t* proc)
{
    uint32_t mcu_irq_bit = 0;

    mcu_irq_bit = channel + MCU_IRQ_STATUS_BIT_DMA0_ATU0;
    if (MCU_IRQ_STATUS_BIT_DMA3_ATU0 < mcu_irq_bit) {
       return ;
    }

    proc->misc_dev->set_mcu_irq_status(mcu_irq_bit, true);
}

char* sysdma_device_t::dmae_addr_to_mem(reg_t paddr, reg_t len, reg_t channel, processor_t* proc)
{
    atu_t *at = atu[channel%2];
    smmu_t *sysdma_smmu = smmu[channel%2];

    char *host_addr = nullptr;
    int idxinbank = proc ? proc->get_idxinbank() : 0;

    paddr = sysdma_smmu->translate(paddr, len, proc);
    if(at && at->is_ipa_enabled()) {
        if (!at->pmp_ok(paddr, len)) {
            dmae_atu_trap(paddr, channel, proc);
            return nullptr;
        }
        paddr = at->translate(paddr, len);
        if (IPA_INVALID_ADDR == paddr) {
            dmae_atu_trap(paddr, channel, proc);
            return nullptr;
        }
    }

    if(IS_NPC_LOCAL_REGION(paddr)) {
        host_addr = bank->npc_addr_to_mem(paddr, idxinbank);
    } else {
        if (nullptr == (host_addr=bank->bank_addr_to_mem(paddr)))
            host_addr = sim->addr_to_mem(paddr);
    }

    return host_addr;
}

smmu_t::~smmu_t()
{
    ;
}

smmu_t::smmu_t(simif_t *sim, bankif_t *bank, uint8_t *sysdma_smmu_base) : sim(sim),
        bank(bank),reg_base(sysdma_smmu_base), mmu_t(sim, bank, nullptr, nullptr, nullptr)
{
    *(uint64_t*)((uint8_t*)reg_base+SMMU_TLB_FL_REG) = 1ull<<48;
}

reg_t smmu_t::translate(reg_t addr, reg_t len, processor_t* proc)
{
    access_type type = LOAD;
    uint32_t xlate_flags = 0;

    if (!proc)
        return addr;

    this->proc = proc;

    bool mxr = get_field(proc->state.mstatus, MSTATUS_MXR);
    bool virt = (proc) ? proc->state.v : false;
    reg_t mode = proc->state.prv;
    if (type != FETCH) {
        if (!proc->state.debug_mode && get_field(proc->state.mstatus, MSTATUS_MPRV)) {
            mode = get_field(proc->state.mstatus, MSTATUS_MPP);
        if (get_field(proc->state.mstatus, MSTATUS_MPV))
            virt = true;
        }
        if (!proc->state.debug_mode && (xlate_flags & RISCV_XLATE_VIRT)) {
            virt = true;
            mode = get_field(proc->state.hstatus, HSTATUS_SPVP);
            if (type == LOAD && (xlate_flags & RISCV_XLATE_VIRT_MXR)) {
                mxr = true;
            }
        }
    }

    reg_t satp = *(reg_t *)((uint8_t *)reg_base + SMMU_SATP_REG);
    reg_t paddr = walk(addr, type, mode, virt, mxr, satp) | (addr & (PGSIZE-1));
    if (!pma_ok(paddr, len, type,!!(xlate_flags&RISCV_XLATE_AMO_FLAG)))
        throw_access_exception(virt, addr, type);
    if (!pmp_ok(paddr, len, type, mode))
        throw_access_exception(virt, addr, type);
    return paddr;
}
