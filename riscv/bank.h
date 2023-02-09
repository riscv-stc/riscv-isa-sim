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
#include "processor.h"
#include "sysdma.h"

class bank_t : public bankif_t {
private:
    int nprocs;     /* bank内核心数 */
    int bank_id;
    std::vector<processor_t*> procs;
    bus_t bank_bus;
    sysdma_device_t *sysdma[2] = {nullptr, nullptr};    /* 每个bank包含2个sysdma控制器 */

public:
    bank_t(const char* isa, const char* priv, const char* varch, simif_t* sim,size_t ddr_size,
            hwsync_t *hwsync, FILE *log_file, size_t board_id, size_t chip_id, 
            int bank_nprocs, int bankid,const std::vector<int> hartids, bool halted,
            const char *atuini);
    virtual ~bank_t();

    int get_bankid(void) {return bank_id;};
    int nprocs_in_bank(void) {return nprocs;};

    bool is_bottom_ddr(reg_t addr) const ;
    reg_t bottom_ddr_to_upper(reg_t addr) const;
    char* bank_addr_to_mem(reg_t addr);

    bool in_bank_mmio(reg_t addr);
    bool bank_mmio_load(reg_t addr, size_t len, uint8_t* bytes);
    bool bank_mmio_store(reg_t addr, size_t len, const uint8_t* bytes);

    bool npc_mmio_load(reg_t addr, size_t len, uint8_t* bytes, uint32_t idxinbank);
    bool npc_mmio_store(reg_t addr, size_t len, const uint8_t* bytes, uint32_t idxinbank);

    processor_t* get_core_by_idxinbank(int idx) { return procs.at(idx); }

    char* npc_addr_to_mem(reg_t addr, uint32_t idxinbank);
    char *dmae_vm_addr_to_mem(reg_t paddr, reg_t len, reg_t channel, processor_t* proc);
    void dmae_vm_mov(uint64_t src_addr, uint64_t dst_addr, int ele_size,
                uint32_t shape_x, uint32_t shape_y, uint32_t shape_z,
                uint32_t stride_s_x, uint32_t stride_s_y,
                uint32_t stride_d_x, uint32_t stride_d_y,
                uint32_t channel, processor_t *l1_proc);
};

#endif // __BANK_H__
