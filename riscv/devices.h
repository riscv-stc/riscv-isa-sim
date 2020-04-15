#ifndef _RISCV_DEVICES_H
#define _RISCV_DEVICES_H

#include "decode.h"
#include <cstdlib>
#include <string>
#include <map>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

class pcie_driver_t;
class processor_t;

class abstract_device_t {
 public:
  virtual bool load(reg_t addr, size_t len, uint8_t* bytes) = 0;
  virtual bool store(reg_t addr, size_t len, const uint8_t* bytes) = 0;
  virtual ~abstract_device_t() {}
};

class bus_t : public abstract_device_t {
 public:
  bool load(reg_t addr, size_t len, uint8_t* bytes);
  bool store(reg_t addr, size_t len, const uint8_t* bytes);
  void add_device(reg_t addr, abstract_device_t* dev);

  std::pair<reg_t, abstract_device_t*> find_device(reg_t addr);

 private:
  std::map<reg_t, abstract_device_t*> devices;
};

class rom_device_t : public abstract_device_t {
 public:
  rom_device_t(std::vector<char> data);
  bool load(reg_t addr, size_t len, uint8_t* bytes);
  bool store(reg_t addr, size_t len, const uint8_t* bytes);
  const std::vector<char>& contents() { return data; }
 private:
  std::vector<char> data;
};

class uart_device_t : public abstract_device_t {
 public:
  uart_device_t();
  bool load(reg_t addr, size_t len, uint8_t* bytes);
  bool store(reg_t addr, size_t len, const uint8_t* bytes);
  // const std::vector<char>& contents() { return data; }
  ~uart_device_t();
 private:
  std::vector<char> data;
};

class mbox_device_t : public abstract_device_t {
 public:
  mbox_device_t(pcie_driver_t * pcie, std::vector<processor_t*>& p);
  bool load(reg_t addr, size_t len, uint8_t* bytes);
  bool store(reg_t addr, size_t len, const uint8_t* bytes);
  // const std::vector<char>& contents() { return data; }
  ~mbox_device_t();
 private:
  uint32_t cmd_count;
  queue<uint32_t> cmd_value;
  uint32_t cmdext_count;
  queue<uint32_t> cmdext_value;
  std::vector<processor_t*>& procs;
  uint8_t data[4096];
  pcie_driver_t *pcie_driver;
};

/**
 * @brief The sysdma_device_t class
 */
class sysdma_device_t : public abstract_device_t {
 public:
  sysdma_device_t(int dma_idx, std::vector<processor_t*>&);
  ~sysdma_device_t();

  bool load(reg_t addr, size_t len, uint8_t* bytes);
  bool store(reg_t addr, size_t len, const uint8_t* bytes);

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
  std::vector<processor_t*>& procs_;

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
  // size of dma buffer
  #define DMA_BUF_SIZE 0x1000
  char dma_buf_[DMA_BUF_SIZE];
  typedef enum {
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

class mem_t : public abstract_device_t {
 public:
  mem_t(size_t size) : len(size) {
    if (!size)
      throw std::runtime_error("zero bytes of target memory requested");
    data = (char*)calloc(1, size);
    if (!data)
      throw std::runtime_error("couldn't allocate " + std::to_string(size) + " bytes of target memory");
  }
  mem_t(const mem_t& that) = delete;
  ~mem_t() { free(data); }

  bool load(reg_t addr, size_t len, uint8_t* bytes) { return false; }
  bool store(reg_t addr, size_t len, const uint8_t* bytes) { return false; }
  char* contents() { return data; }
  size_t size() { return len; }

 private:
  char* data;
  size_t len;
};

class clint_t : public abstract_device_t {
 public:
  clint_t(std::vector<processor_t*>&);
  bool load(reg_t addr, size_t len, uint8_t* bytes);
  bool store(reg_t addr, size_t len, const uint8_t* bytes);
  size_t size() { return CLINT_SIZE; }
  void increment(reg_t inc);
 private:
  typedef uint64_t mtime_t;
  typedef uint64_t mtimecmp_t;
  typedef uint32_t msip_t;
  std::vector<processor_t*>& procs;
  mtime_t mtime;
  std::vector<mtimecmp_t> mtimecmp;
};

#endif
