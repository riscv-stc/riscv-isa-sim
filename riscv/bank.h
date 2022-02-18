#ifndef __BANK_H__
#define __BANK_H__

#include <vector>
#include <mutex>
#include <condition_variable>
#include "decode.h"
#include <semaphore.h>
#include <pthread.h>

#include "devices.h"
#include "simif.h"
#include "bankif.h"
#include "hwsync.h"
#include "pcie_driver.h"
#include "processor.h"

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
#define MBOX_START           (0xc07f4000)   /* NPUV2 NP_MBOX_LOC 4KB */

//ddr high 1G address, just accessed by pcie and sysdma
//range is 0xc0800000 ~ 0xf8000000
/* NPUV2 SoC Region(non-cacheable) 0x00_C080_0000 */
#define GLB_UPPER_REGION_BASE       (0)
#define GLB_UPPER_REGION_SIZE      (0x100000000)
#define GLB_BOTTOM_REGION_SIZE (0xC0000000)
#define GLB_DIE0_UPPER_REGION_BANK0_START_ADDR (0x800000000)
#define GLB_DIE1_UPPER_REGION_BANK0_START_ADDR (0x8800000000)

class bank_t : public bankif_t {
private:
    int nprocs;     /* bank内核心数 */
    int bank_id;
    int die_id;
    bool is_finish;
    int pcie_enabled;
    std::vector<processor_t*> procs;
    pcie_driver_t *pcie_driver;

    bus_t bank_bus;
    std::vector<bus_t*> npc_bus;
public:
    bank_t(const char* isa, const char* priv, const char* varch, simif_t* sim,size_t ddr_size,
            hwsync_t *hwsync, FILE *log_file, bool pcie_enabled, size_t board_id, size_t chip_id, 
            int bank_nprocs, int bankid,const std::vector<int> hartids, bool halted);
    virtual ~bank_t();

    int get_bankid(void) {return bank_id;};
    int nprocs_in_bank(void) {return nprocs;};

    bool is_bottom_ddr(reg_t addr) const ;
    reg_t bottom_ddr_to_upper(reg_t addr) const;
    char* bank_addr_to_mem(reg_t addr);
    bool in_npc_mem(reg_t addr, local_device_type type);

    bool bank_mmio_load(reg_t addr, size_t len, uint8_t* bytes);
    bool bank_mmio_store(reg_t addr, size_t len, const uint8_t* bytes);

    bool npc_mmio_load(reg_t addr, size_t len, uint8_t* bytes, uint32_t idxinbank);
    bool npc_mmio_store(reg_t addr, size_t len, const uint8_t* bytes, uint32_t idxinbank);

    processor_t* get_core_by_idxinbank(int idx) { return procs.at(idx); }

    char* npc_addr_to_mem(reg_t addr, uint32_t idxinbank);

    void set_bank_finish(bool finish);
    bool is_bank_finish(void);
};

#endif // __BANK_H__
