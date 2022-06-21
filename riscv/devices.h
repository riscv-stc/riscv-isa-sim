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

#define HWSYNC_START          (0xd3e80000)

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

/* 64bit RO 指令计数寄存器 */
#define NCP_VME_INST_CNT        (0x000)     
#define NCP_MME_INST_CNT        (0x008)
#define NCP_RVV_INST_CNT        (0x010)     /* v2没有rvv，这个保留暂不生效 */
#define NCP_SYN_INST_CNT        (0x018)
#define NCP_SYN_DONE_INST_CNT   (0x020)
#define MTE_PLD_INST_CNT        (0x028)
#define MTE_PLD_DONE_INST_CNT   (0x030)
#define MTE_ICMOV_INST_CNT      (0x038)
#define MTE_MOV_INST_CNT        (0x040)
#define DMA_INST_CNT            (0x048)

#define MCU_PERF_CNT_CTRL       (0x080)

/* 32bit 中断寄存器 */
#define NP_IRQ_OUT_CTRL         (0x9a0)
#define MCU_IRQ_STATUS_OFFSET   (0x9a8)
#define MCU_IRQ_ENABLE_OFFSET   (0x9ac)
#define MCU_IRQ_CLEAR_OFFSET    (0x9b0)
#define MCU_IRQ_STATUS_BIT_NPC_MBOX_IRQ (0)
#define MCU_IRQ_STATUS_BIT_NPC_IN_IRQ   (1)
#define MCU_IRQ_STATUS_BIT_DMA0_SMMU0   (4)
#define MCU_IRQ_STATUS_BIT_DMA1_SMMU0   (5)
#define MCU_IRQ_STATUS_BIT_DMA2_SMMU0   (6)
#define MCU_IRQ_STATUS_BIT_DMA3_SMMU0   (7)
#define MCU_IRQ_STATUS_BIT_DMA0_ATU0    (8)
#define MCU_IRQ_STATUS_BIT_DMA1_ATU0    (9)
#define MCU_IRQ_STATUS_BIT_DMA2_ATU0    (10)
#define MCU_IRQ_STATUS_BIT_DMA3_ATU0    (11)

#define IS_STC_NPUV2_INST(bits) (((insn_bits_t)(bits)&0x7f) == 0x7b)
#define IS_PLD_INST(bits)       (((insn_bits_t)(bits)&MASK_PLD) == MATCH_PLD)
#define IS_ICMOV_M_INST(bits)   (((insn_bits_t)(bits)&MASK_ICMOV_M) == MATCH_ICMOV_M)

#define IS_MTE_MOV_INST(bits)    ((((insn_bits_t)(bits)&MASK_MOV_LLB_L1)==MATCH_MOV_LLB_L1) \
            || (((insn_bits_t)(bits)&MASK_MOV_L1_LLB)==MATCH_MOV_L1_LLB))

#define IS_SYNC_INST(bits)      (IS_STC_NPUV2_INST(bits) && \
            ((bits>>26) == (MATCH_SYNC>>26)))

#define IS_MME_INST(bits)       (IS_STC_NPUV2_INST(bits) && \
            ((bits>>26) >= (MATCH_METR_M>>26)) && ((bits>>26) <= (MATCH_MEACC_M>>26)))

#define IS_DMAE_INST(bits)      (((bits&MASK_MOV_L1_GLB) >= MATCH_MOV_L1_GLB) && \
            ((bits&MASK_MOV_LLB_LLB) <= MATCH_MOV_LLB_LLB))


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
  size_t size(void) {return data.size();};
  bool load(reg_t addr, size_t len, uint8_t* bytes);
  bool store(reg_t addr, size_t len, const uint8_t* bytes);
  const std::vector<char>& contents() { return data; }
 private:
  std::vector<char> data;
};

class misc_device_t : public abstract_device_t {
 public:
  misc_device_t(pcie_driver_t * pcie, processor_t* proc);
  bool load(reg_t addr, size_t len, uint8_t* bytes);
  bool store(reg_t addr, size_t len, const uint8_t* bytes);

  size_t size() { return buf_len; }
  ~misc_device_t();

  void inst_cnt(insn_bits_t bits);      /* 更新指令计数器 */
  void inst_done_cnt(insn_bits_t bits); /* sync done, pld done */
  void inst_cnt_start(void)     {inst_start = true;};
  void inst_cnt_stop(void)      {inst_start = false;};
  bool is_inst_start(void)      {return inst_start;};
  void inst_cnt_clear(void);

  void set_mcu_irq_status(int mcu_irq_status_bit, bool val);

 private:
  std::vector<char> buf;        /* 调试用，uart存字符串 */
  size_t buf_len;
  uint8_t *reg_base = nullptr;  /* 寄存器地址空间 */
  processor_t* proc;
  bool inst_start = false;

  uint32_t dump_addr;
  uint32_t dump_len;
  uint32_t dump_count;

  pcie_driver_t *pcie_driver = nullptr;

  /* 只读寄存器的写操作不放在store中 */
  bool ro_register_write(reg_t addr, uint32_t val);
  bool ro_register_write(reg_t addr, uint64_t val);
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

#define SYSIRQ_BASE     0xd3e10000
#define SYSIRQ_SIZE     0x10000

/* Mask to CPU Register */
#define CPU_IRQ_MASK_ADDR0          0x00        /* 0b1 屏蔽中断, 0b0不屏蔽 */
#define CPU_IRQ_MASK_ADDR1          0x04

/* BANK SW IRQ Register */
#define BANK_SW_IRQ_IN_SET_ADDR     0x54        /* 0b1 在NPC产生中断 */
#define BANK_NPC_SW_IRQ_LATCH_CLR_ADDR      0x58

/* IRQ STATUS REGISTER */
#define TO_CPU_NPC_SW_IRQ_OUT_STS_ADDR      0x114   /* RO */

class apifc_t;

class sys_irq_t : public abstract_device_t {
 public:
  sys_irq_t(simif_t *sim, apifc_t *apifc);
  ~sys_irq_t();
  bool load(reg_t addr, size_t len, uint8_t* bytes);
  bool store(reg_t addr, size_t len, const uint8_t* bytes);
  size_t size() { return SYSIRQ_SIZE; }

 private:
  simif_t *sim = nullptr;
  apifc_t *apifc = nullptr;

  uint8_t reg_base[SYSIRQ_SIZE];
};

#endif
