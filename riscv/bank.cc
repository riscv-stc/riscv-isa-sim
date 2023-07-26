#include <iostream>
#include <assert.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <signal.h>

#include "bank.h"

/**
 * 参数: @nprocs bank内processor数量 
 */
bank_t::bank_t(const char* isa, const char* priv, const char* varch, simif_t* sim,size_t ddr_size,
            hwsync_t *hwsync, FILE *log_file, size_t board_id, 
            size_t chip_id, int bank_nprocs,int bankid, 
            const std::vector<int> hartids, bool halted, const char *atuini, bool multiCoreThreadFlag) : nprocs(bank_nprocs),
            bank_id(bankid), procs(std::max(size_t(bank_nprocs),size_t(1)))
{
    /* DDR */
    if (ddr_size > 0) {
        bank_bus.add_device(GLB_DIE0_UPPER_REGION_BANK0_START_ADDR+bank_id*GLB_UPPER_REGION_SIZE, new mem_t(ddr_size));
    }

    /* 创建 processor */
    for (int i = 0; i < nprocs; i++) {
        int hart_id = hartids.empty() ? (i + bank_id * nprocs) : hartids[bank_id*nprocs+i];
        procs[i] = new processor_t(isa, priv, varch, sim, this, hwsync, i,
                    hart_id, bank_id, halted, atuini, log_file, multiCoreThreadFlag);
    }

    /* 添加 sysdma */
    switch (bankid) {
    case 0:
        bank_bus.add_device(SYSDMA0_BASE, sysdma[0] = new sysdma_device_t(0, sim, this, atuini, procs[0]));
        bank_bus.add_device(SYSDMA1_BASE, sysdma[1] = new sysdma_device_t(1, sim, this, atuini, procs[0]));
        break;
    case 1:
        bank_bus.add_device(SYSDMA2_BASE, sysdma[0] = new sysdma_device_t(2, sim, this, atuini, procs[0]));
        bank_bus.add_device(SYSDMA3_BASE, sysdma[1] = new sysdma_device_t(3, sim, this, atuini, procs[0]));
        break;
    case 2:
        bank_bus.add_device(SYSDMA4_BASE, sysdma[0] = new sysdma_device_t(4, sim, this, atuini, procs[0]));
        bank_bus.add_device(SYSDMA5_BASE, sysdma[1] = new sysdma_device_t(5, sim, this, atuini, procs[0]));
        break;
    case 3:
        bank_bus.add_device(SYSDMA6_BASE, sysdma[0] = new sysdma_device_t(6, sim, this, atuini, procs[0]));
        bank_bus.add_device(SYSDMA7_BASE, sysdma[1] = new sysdma_device_t(7, sim, this, atuini, procs[0]));
        break;
    default:
        throw std::runtime_error("unsupported core id");
    }

    return ;
}

bank_t::~bank_t() {
    for (int i = 0; i < nprocs; i++) {
        delete procs[i];
    }

    return ;
}

/* 取npc核内非 mem_t类型的资源，如 misc mbox */
bool bank_t::npc_mmio_load(reg_t addr, size_t len, uint8_t* bytes, uint32_t idxinbank)
{
  return get_core_by_idxinbank(idxinbank)->mmio_load(addr, len, bytes);
}

bool bank_t::npc_mmio_store(reg_t addr, size_t len, const uint8_t* bytes, uint32_t idxinbank)
{
  return get_core_by_idxinbank(idxinbank)->mmio_store(addr, len, bytes);
}

bool bank_t::in_bank_mmio(reg_t addr) {
    auto desc = bank_bus.find_device(addr);

    if (auto mem = dynamic_cast<sysdma_device_t *>(desc.second)) {
        if (addr - desc.first <= mem->size()) {
            return true;
        }
    }

    return false;
}

/* 取bank内非 mem_t类型的资源，如 hwsync */
bool bank_t::bank_mmio_load(reg_t addr, size_t len, uint8_t* bytes)
{
    if (in_bank_mmio(addr)) {
        return bank_bus.load(addr, len, bytes);
    }
    return false;
}

bool bank_t::bank_mmio_store(reg_t addr, size_t len, const uint8_t* bytes)
{
    if (in_bank_mmio(addr)) {
        return bank_bus.store(addr, len, bytes);
    }
    return false;
}

bool bank_t::is_bottom_ddr(reg_t addr) const
{
    if (addr < GLB_BOTTOM_REGION_SIZE)
        return true;
    else 
        return false;
}

reg_t bank_t::bottom_ddr_to_upper(reg_t addr) const
{
    if (!is_bottom_ddr(addr))
        return 0;

    return  GLB_DIE0_UPPER_REGION_BANK0_START_ADDR+(GLB_UPPER_REGION_SIZE*bank_id) + addr;
}

/* 取bank内mem_t类型的资源, 如 sysdma llb ddr */
char* bank_t::bank_addr_to_mem(reg_t addr) 
{
    reg_t address = 0;

    if (is_bottom_ddr(addr)) {
        address = bottom_ddr_to_upper(addr);
    } else {
        address = addr;
    }

    auto desc = bank_bus.find_device(address);

    if (auto mem = dynamic_cast<mem_t *>(desc.second)) {
        if (address - desc.first < mem->size())
            return mem->contents() + (address - desc.first);
        return NULL;
    }

    return NULL;
}

/* 取npc内的 l1buffer im sp 等mem_t类型的资源 */
char* bank_t::npc_addr_to_mem(reg_t addr, uint32_t idxinbank) 
{
    return get_core_by_idxinbank(idxinbank)->addr_to_mem(addr);
}

char* bank_t::dmae_vm_addr_to_mem(reg_t paddr, reg_t len, reg_t channel, processor_t* proc)
{
    if (4 <= channel) 
        return nullptr;

    return sysdma[channel/2]->dmae_vm_addr_to_mem(paddr, len, channel, proc);
}

void bank_t::dmae_vm_mov(uint64_t src_addr, uint64_t dst_addr, int ele_size,
              uint32_t shape_x, uint32_t shape_y, uint32_t shape_z,
              uint32_t stride_s_x, uint32_t stride_s_y,
              uint32_t stride_d_x, uint32_t stride_d_y,
              uint32_t channel ,processor_t *l1_proc)
{
    if (4 <= channel) 
        return;

    sysdma[channel/2]->sysdma_vm_mov(src_addr, dst_addr, ele_size,
              shape_x, shape_y, shape_z,
              stride_s_x, stride_s_y,
              stride_d_x, stride_d_y,
              channel%2, l1_proc);
}
