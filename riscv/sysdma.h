#ifndef __SYSDMA_H__
#define __SYSDMA_H__

#include "device.h"
#include "atu.h"

/**
 * @brief The sysdma_device_t class
 */
class sysdma_device_t : public abstract_device_t {
 public:
  sysdma_device_t(int dma_idx, simif_t *sim,bankif_t *bank);
  ~sysdma_device_t();

  bool load(reg_t addr, size_t len, uint8_t* bytes);
  bool store(reg_t addr, size_t len, const uint8_t* bytes);

  size_t size(void) {return sizeof(sys_dma_reg);};

  // dma descriptor
  struct dma_desc_t {
    union {
      struct {
        uint32_t xfer_len : 22;   // transfer length
        uint32_t burst_len : 4;   // burst length
        uint32_t burst_size : 3;  // burst size
        uint32_t blk_en : 1;      // block mode enabled
        uint32_t reserved : 1;
        uint32_t desc_en : 1;  // descriptor mode enabled
      } bits;
      uint32_t full;
    } ctlr;

    uint32_t dsar;  // source address
    uint32_t ddar;  // destination address

    union {
      struct {
        uint32_t width : 16;   // block Width low 16-bits in bytes
        uint32_t height : 16;  // block Height in rows
      } bits;
      uint32_t full;
    } bkmr0;

    union {
      struct {
        uint32_t stride : 24;  // block stride in bytes
        uint32_t width_high : 6; // block Width high 6 bits in bytes
        uint32_t width_high_reserved : 2;
      } bits;
      uint32_t full;
    } bkmr1;

    uint32_t llpr;  // next llp addr
  };

 private:

  void dma_core(int ch);
  simif_t *sim;
  bankif_t *bank;

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
  #define DMA_REGION_SIZE 0x10000       /* 64kB */

  char sys_dma_reg[DMA_REGION_SIZE];
  enum {
    DDR_DIR_SRC = 0,
    DDR_DIR_DST,
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
