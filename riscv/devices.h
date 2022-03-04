#ifndef _RISCV_DEVICES_H
#define _RISCV_DEVICES_H

#include "decode.h"
#include "mmio_plugin.h"
#include <cstdlib>
#include <string>
#include <map>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <condition_variable>
#include <stdexcept>

#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include "bankif.h"
#include "iniparser.h"

#define HWSYNC_START          (0xd0080000)

/* NPUV2 The lower 3GB region (0x00_0000_0000 ~ 0x00_BFFF_FFFF) is the remapping target region of DDR space */
#define ddr_mem_start        (0x00000000)

/* NPUV2 L1 Buffer (1024KB+288KB) */
#define l1_buffer_start      (0xc0000000)
#define l1_buffer_size       (0x00148000)

/* NPUV2 Intermediate Buffer(256KB) */
#define im_buffer_start      (0xc0400000)
#define im_buffer_size       (0x00040000)

/* NPUV2 Index(.sp) RAM (80KB) */
#define sp_buffer_start      (0xc0500000)
#define sp_buffer_size       (0x00014000)

#define MISC_START           (0xc07f3000)   /* NPUV2 NP_MISC 4KB */
#define MISC_SIZE            (0x1000)
#define MCU_IRQ_STATUS_OFFSET   (0x9a8)
#define MCU_IRQ_ENABLE_OFFSET   (0x9ac)
#define MCU_IRQ_CLEAR_OFFSET    (0x9b0)
#define MCU_IRQ_STATUS_BIT_DMA0_SMMU0   (4)
#define MCU_IRQ_STATUS_BIT_DMA1_SMMU0   (5)
#define MCU_IRQ_STATUS_BIT_DMA2_SMMU0   (6)
#define MCU_IRQ_STATUS_BIT_DMA3_SMMU0   (7)
#define MCU_IRQ_STATUS_BIT_DMA0_ATU0    (8)
#define MCU_IRQ_STATUS_BIT_DMA1_ATU0    (9)
#define MCU_IRQ_STATUS_BIT_DMA2_ATU0    (10)
#define MCU_IRQ_STATUS_BIT_DMA3_ATU0    (11)

#define MBOX_START           (0xc07f4000)   /* NPUV2 NP_MBOX_LOC 4KB */

#define NP_IOV_ATU_START    (0xc07f7000)    /* ipa address trans unit */
#define NP_IOV_ATU_SIZE     (0x1000)

#define NPC_LOCAL_ADDR_START (l1_buffer_start)
#define NPC_LOCAL_REGIN_SIZE (0x800000)

//ddr high 1G address, just accessed by pcie and sysdma
//range is 0xc0800000 ~ 0xf8000000
/* NPUV2 SoC Region(non-cacheable) 0x00_C080_0000 */
#define GLB_UPPER_REGION_BASE       (0)
#define GLB_UPPER_REGION_SIZE      (0x100000000)
#define GLB_BOTTOM_REGION_SIZE (0xC0000000)
#define GLB_DIE0_UPPER_REGION_BANK0_START_ADDR (0x800000000)
#define GLB_DIE1_UPPER_REGION_BANK0_START_ADDR (0x8800000000)

#define IS_NPC_LOCAL_REGION(addr)   (((addr>=NPC_LOCAL_ADDR_START) && \
        (addr<NPC_LOCAL_ADDR_START+NPC_LOCAL_REGIN_SIZE)) ? true : false)

class pcie_driver_t;
class processor_t;
class simif_t;

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

class misc_device_t : public abstract_device_t {
 public:
  misc_device_t(processor_t* proc);
  bool load(reg_t addr, size_t len, uint8_t* bytes);
  bool store(reg_t addr, size_t len, const uint8_t* bytes);

  size_t size() { return buf_len; }
  ~misc_device_t();
 private:
  std::vector<char> buf;        /* 调试用，uart存字符串 */
  size_t buf_len;
  uint8_t *reg_base = nullptr;  /* 寄存器地址空间 */
  processor_t* proc;

  uint32_t dump_addr;
  uint32_t dump_len;
  uint32_t dump_count;
};

class mbox_device_t : public abstract_device_t {
 public:
  mbox_device_t(pcie_driver_t * pcie, processor_t* p, bool pcie_enabled);
  bool load(reg_t addr, size_t len, uint8_t* bytes);
  bool store(reg_t addr, size_t len, const uint8_t* bytes);
  void reset();
  size_t size() { return 4096; }
  // const std::vector<char>& contents() { return data; }
  ~mbox_device_t();
 private:
  uint32_t cmd_count;
  std::queue<uint32_t> cmd_value;
  uint32_t cmdext_count;
  std::queue<uint32_t> cmdext_value;
  processor_t* p;
  uint8_t data[4096];
  bool pcie_enabled;
  pcie_driver_t *pcie_driver;
};

/**
 * @brief The sysdma_device_t class
 */
class sysdma_device_t : public abstract_device_t {
 public:
  sysdma_device_t(int dma_idx, simif_t *sim,bankif_t *bank);
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
  char dma_buf_[DMA_BUF_SIZE];
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

class share_mem_t : public abstract_device_t {
  private:
    int shm_id;
    size_t len;
    char *data_ptr;
    char *start_ptr;
    char *shm_name;

  public:
    share_mem_t(size_t size, char* name, size_t offset) : len(size) {
      char file_name[64];
      if (!size)
        throw std::runtime_error("zero bytes of target memory requested");

      shm_id = shm_open(name, O_CREAT | O_RDWR, 0666);
      if (shm_id == -1) {
        throw std::runtime_error("shmget failed");
      }

      sprintf(file_name, "/dev/shm/%s", name);
      chmod(file_name, 0666);
      ftruncate(shm_id, size);
      start_ptr = (char *)mmap(0, size, PROT_WRITE, MAP_SHARED, shm_id, 0);
      if (start_ptr == (void *)-1)
        throw std::runtime_error("shmat failed");
      data_ptr = start_ptr + offset;
      shm_name = name;
    }

    share_mem_t(const share_mem_t& that) = delete;
    ~share_mem_t() {
      char file_name[64];
      sprintf(file_name, "/dev/shm/%s", shm_name);
      chmod(file_name, 0666);

      munmap(start_ptr, len);
      shm_unlink(shm_name);
    }

    bool load(reg_t addr, size_t len, uint8_t* bytes) { return false; }
    bool store(reg_t addr, size_t len, const uint8_t* bytes) { return false; }
    char* contents() {return data_ptr;}
    size_t size() {return len;}
};

class clint_t : public abstract_device_t {
 public:
  clint_t(simif_t *sim, uint64_t freq_hz, bool real_time);
  bool load(reg_t addr, size_t len, uint8_t* bytes);
  bool store(reg_t addr, size_t len, const uint8_t* bytes);
  size_t size() { return CLINT_SIZE; }
  void increment(reg_t inc);
 private:
  typedef uint64_t mtime_t;
  typedef uint64_t mtimecmp_t;
  typedef uint32_t msip_t;
  //std::vector<processor_t*>& procs;
  simif_t *sim;
  uint64_t freq_hz;
  bool real_time;
  uint64_t real_time_ref_secs;
  uint64_t real_time_ref_usecs;
  mtime_t mtime;
  std::vector<mtimecmp_t> mtimecmp;
};

class mmio_plugin_device_t : public abstract_device_t {
 public:
  mmio_plugin_device_t(const std::string& name, const std::string& args);
  virtual ~mmio_plugin_device_t() override;

  virtual bool load(reg_t addr, size_t len, uint8_t* bytes) override;
  virtual bool store(reg_t addr, size_t len, const uint8_t* bytes) override;

 private:
  mmio_plugin_t plugin;
  void* user_data;
};

#endif
