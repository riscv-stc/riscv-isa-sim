#ifndef __SYSDMA_H__
#define __SYSDMA_H__

#include "device.h"
#include "atu.h"
#include "mmu.h"

class smmu_t : public mmu_t
{
public:
    smmu_t(simif_t *sim, bankif_t *bank, uint8_t *sysdma_smmu_base);
    ~smmu_t();
    reg_t translate(reg_t addr, reg_t len, processor_t* proc);

private:
    simif_t *sim;
    bankif_t *bank;
    uint8_t *reg_base;
    reg_t satp = ~(reg_t)(0);
};

/**
 * @brief The sysdma_device_t class
 */
class sysdma_device_t : public abstract_device_t {
 public:
  sysdma_device_t(int dma_idx, simif_t *sim, bankif_t *bank, const char *atuini);
  ~sysdma_device_t();

  bool load(reg_t addr, size_t len, uint8_t* bytes);
  bool store(reg_t addr, size_t len, const uint8_t* bytes);
  size_t size(void) {return sizeof(sys_dma_reg);};

  char *dmae_addr_to_mem(reg_t paddr, reg_t len, reg_t channel, processor_t* proc);

  // dma descriptor
  struct dma_desc_t {
    union {
      struct {
        uint32_t src_data_type : 4;
        uint32_t reserved3 : 4;
        uint32_t dst_data_type : 4;
        uint32_t reserved2 : 10;
        uint32_t burst_len : 4;   /* Maximum AXI burst length, init 0xf */
        uint32_t burst_size : 3;  /* AXI burst size, init 0x7 */
        uint32_t reserved1 : 1;
        uint32_t remote_desc_en : 1;    /* Remote Descriptor Fetch Enable */
        uint32_t desc_en : 1;     /* descriptor mode enabled */
      } bits;
      uint32_t full;
    } ctlr;

    uint32_t dsar;  // source address
    uint32_t ddar;  // destination address

    union {
      struct {
        uint32_t width : 22;
        uint32_t reserved : 10;
      } bits;
      uint32_t full;
    } bkmr0;

    union {
      struct {
        uint32_t high : 16;
        uint32_t depth : 16;
      } bits;
      uint32_t full;
    } bkmr1;

    union {
      struct {
        uint32_t stride_s1 : 24;    /* src stride width */
        uint32_t reserved : 8;
      } bits;
      uint32_t full;
    } bkmr2;

    union {
      uint32_t stride_s2;           /* src stride high */
      uint32_t full;
    } bkmr3;

    union {
      struct {
        uint32_t stride_d1 : 24;    /* dst stride witth */
        uint32_t reserved : 8;
      } bits;
      uint32_t full;
    } bkmr4;

    union {
      uint32_t stride_d2;           /* dst stride hight */
      uint32_t full;
    } bkmr5;
    
    uint32_t llpr;  // next llp addr
  };

 private:
 
  void dma_core(int ch);
  simif_t *sim;
  bankif_t *bank;
  smmu_t *smmu[2];
  atu_t *atu[2];

  void dmae_atu_trap(reg_t paddr, int channel, processor_t* proc);

  // dma direction
  enum direction_t {
    SYSDMA_CHAN0   = 0,
    SYSDMA_CHAN1   = 1,
  };

  // dma thread
  #define DMA_MAX_CHANNEL_NUMER 2
  std::thread dma_ch_thread_[DMA_MAX_CHANNEL_NUMER];
  std::mutex thread_lock_[DMA_MAX_CHANNEL_NUMER];
  std::condition_variable thread_cond_[DMA_MAX_CHANNEL_NUMER];

  // dma feature
  bool dma_enabled_;
  int dma_idx_;
  // size of dma buffer
  #define DMA_BUF_SIZE 0x1000

  char sys_dma_reg[DMA_REGION_SIZE];
  enum {
    DDR_DIR_SRC = 0,
    DDR_DIR_DST,
    DDR_DIR_EA,   /*  It takes effect only if DMA_C0_CTLR.REMOTE_DESC_EN=1 */
    DDR_DIR_MAX,
  };

  struct {
    bool enabled;
    bool desc_mode_enabled;
    unsigned long llp;
    bool xfer_complete = false;
    bool busy = false;
    uint64_t ddr_base[DDR_DIR_MAX];
  } dma_channel_[DMA_MAX_CHANNEL_NUMER];
};

#endif
