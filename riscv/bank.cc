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
            hwsync_t *hwsync, FILE *log_file, bool pcie_enabled, size_t board_id, 
            size_t chip_id, int bank_nprocs,int bankid, 
            const std::vector<int> hartids, bool halted) : nprocs(bank_nprocs),
            bank_id(bankid), pcie_enabled(pcie_enabled), procs(std::max(size_t(bank_nprocs),size_t(1))),
            npc_bus(std::max(size_t(bank_nprocs),size_t(1))),is_finish(false)
{
    /* 添加 sysdma */
    switch (bankid) {
    case 0:
        bank_bus.add_device(SYSDMA0_BASE, new sysdma_device_t(0, sim, this));
        bank_bus.add_device(SYSDMA1_BASE, new sysdma_device_t(1, sim, this));
        break;
    case 1:
        bank_bus.add_device(SYSDMA2_BASE, new sysdma_device_t(2, sim, this));
        bank_bus.add_device(SYSDMA3_BASE, new sysdma_device_t(3, sim, this));
        break;
    case 2:
        bank_bus.add_device(SYSDMA4_BASE, new sysdma_device_t(4, sim, this));
        bank_bus.add_device(SYSDMA5_BASE, new sysdma_device_t(5, sim, this));
        break;
    case 3:
        bank_bus.add_device(SYSDMA6_BASE, new sysdma_device_t(6, sim, this));
        bank_bus.add_device(SYSDMA7_BASE, new sysdma_device_t(7, sim, this));
        break;
    default:
        throw std::runtime_error("unsupported core id");
    }

    /* 添加 llb */
    mem_t *llb = new mem_t(LLB_BANK_BUFFER_SIZE);
    bank_bus.add_device(LLB_AXI0_BUFFER_START, llb);
    bank_bus.add_device(LLB_AXI1_BUFFER_START, llb);

    /* DDR */
    if (ddr_size > 0) {
        bank_bus.add_device(GLB_DIE0_UPPER_REGION_BANK0_START_ADDR+bank_id*GLB_UPPER_REGION_SIZE, new mem_t(ddr_size));
    }

    /* pcie driver */
    if(pcie_enabled) {
        pcie_driver = new pcie_driver_t(sim, this, bank_id, pcie_enabled, board_id, chip_id);
    }

    /* 创建 processor */
    for (size_t i = 0; i < nprocs; i++) {
        int hart_id = hartids.empty() ? (i + bank_id * nprocs) : hartids[bank_id*nprocs+i];
        procs[i] = new processor_t(isa, priv, varch, sim, this, hwsync, i,
                    hart_id, bank_id, halted, log_file);
    }

    /* 为每个proc添加 npc_bus 资源 */
    for (int i = 0; i < nprocs; i++) {
        npc_bus[i] = new bus_t();

        npc_bus[i]->add_device(l1_buffer_start, new mem_t(l1_buffer_size));
        npc_bus[i]->add_device(im_buffer_start, new mem_t(im_buffer_size));
        npc_bus[i]->add_device(sp_buffer_start, new mem_t(sp_buffer_size));
        npc_bus[i]->add_device(MISC_START, new misc_device_t(procs[i]));
        
        mbox_device_t *box = new mbox_device_t(pcie_driver, procs[i] , pcie_enabled);
        npc_bus[i]->add_device(MBOX_START, box);
        procs[i]->add_mbox(box);
    }

    return ;
}

bank_t::~bank_t() {
    if(pcie_enabled) {
        delete pcie_driver;
    }

    for (size_t i = 0; i < nprocs; i++) {
        delete procs[i];
    }
    
    for (size_t i = 0; i < nprocs; i++) {
        delete npc_bus[i];
    }

    return ;
}

/* 取npc核内非 mem_t类型的资源，如 misc mbox */
bool bank_t::npc_mmio_load(reg_t addr, size_t len, uint8_t* bytes, uint32_t idxinbank)
{
  return npc_bus[idxinbank]->load(addr & 0xffffffff, len, bytes);
}

bool bank_t::npc_mmio_store(reg_t addr, size_t len, const uint8_t* bytes, uint32_t idxinbank)
{
  return npc_bus[idxinbank]->store(addr & 0xffffffff, len, bytes);
}

/* 取bank内非 mem_t类型的资源，如 hwsync */
bool bank_t::bank_mmio_load(reg_t addr, size_t len, uint8_t* bytes)
{
    return bank_bus.load(addr, len, bytes);
}

bool bank_t::bank_mmio_store(reg_t addr, size_t len, const uint8_t* bytes)
{
    return bank_bus.store(addr, len, bytes);
}

bool bank_t::in_npc_mem(reg_t addr, local_device_type type) {
  auto desc = npc_bus[0]->find_device(addr);

  if (type == IO_DEVICE) {
    if (auto mem = dynamic_cast<misc_device_t *>(desc.second)) {
      if (addr - desc.first <= mem->size()) {
        return true;
      }
    }

    if (auto mem = dynamic_cast<mbox_device_t *>(desc.second)) {
      if (addr - desc.first <= mem->size()) {
        return true;
      }
    }
    return false;
  } else if (type == L1_BUFFER) {
    if (auto mem = dynamic_cast<mem_t *>(desc.second)) {
      reg_t start_addr = l1_buffer_start;
      if (desc.first == start_addr && addr - desc.first <= mem->size()) {
        return true;
      }
    }
  } else if (type == SP_BUFFER) {
    if (auto mem = dynamic_cast<mem_t *>(desc.second)) {
      reg_t start_addr = sp_buffer_start;
      if (desc.first == start_addr && addr - desc.first <= mem->size()) {
        return true;
      }
    }
  } else if (IM_BUFFER == type) {
    if (auto mem = dynamic_cast<mem_t *>(desc.second)) {
      reg_t start_addr = im_buffer_start;
      if (desc.first == start_addr && addr - desc.first <= mem->size()) {
        return true;
      }
    }
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
    auto desc = npc_bus[idxinbank]->find_device(addr);

    if (auto mem = dynamic_cast<mem_t *>(desc.second)) {
        if (addr - desc.first < mem->size())
            return mem->contents() + (addr - desc.first);
        return NULL;
    }

  return NULL;
}

void bank_t::set_bank_finish(bool finish)
{
    is_finish = finish;
}

bool bank_t::is_bank_finish(void)
{
    return is_finish;
}
