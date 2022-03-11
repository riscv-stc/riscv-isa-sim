// See LICENSE for license details.

#include "mmu.h"
#include "arith.h"
#include "simif.h"
#include "bankif.h"
#include "processor.h"

mmu_t::mmu_t(simif_t* sim, bankif_t *bank, processor_t* proc, ipa_t *ipa)
 : sim(sim), bank(bank), proc(proc), ipa(ipa),
#ifdef RISCV_ENABLE_DUAL_ENDIAN
  target_big_endian(false),
#endif
  check_triggers_fetch(false),
  check_triggers_load(false),
  check_triggers_store(false),
  matched_trigger(NULL)
{
  flush_tlb();
  yield_load_reservation();
}

mmu_t::~mmu_t()
{
}

void mmu_t::flush_icache()
{
  for (size_t i = 0; i < ICACHE_ENTRIES; i++)
    icache[i].tag = -1;
}

void mmu_t::flush_tlb()
{
  memset(tlb_insn_tag, -1, sizeof(tlb_insn_tag));
  memset(tlb_load_tag, -1, sizeof(tlb_load_tag));
  memset(tlb_store_tag, -1, sizeof(tlb_store_tag));

  flush_icache();
}

void mmu_t::throw_access_exception(bool virt, reg_t addr, access_type type)
{
  switch (type) {
    case FETCH: throw trap_instruction_access_fault(virt, addr, 0, 0);
    case LOAD: throw trap_load_access_fault(virt, addr, 0, 0);
    case STORE: throw trap_store_access_fault(virt, addr, 0, 0);
    default: abort();
  }
}

reg_t mmu_t::translate(reg_t addr, reg_t len, access_type type, uint32_t xlate_flags)
{
  if (!proc)
    return addr;

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

  reg_t paddr = walk(addr, type, mode, virt, mxr) | (addr & (PGSIZE-1));
  if (!pma_ok(paddr, len, type,!!(xlate_flags&RISCV_XLATE_AMO_FLAG)))
    throw_access_exception(virt, addr, type);
  if (!pmp_ok(paddr, len, type, mode))
    throw_access_exception(virt, addr, type);
  return paddr;
}

tlb_entry_t mmu_t::fetch_slow_path(reg_t vaddr)
{
  char *host_addr = nullptr;
  int bankid = bank ? bank->get_bankid() : 0;
  int idxinbank = proc ? proc->get_idxinbank() : 0;
  reg_t paddr = translate(vaddr, sizeof(fetch_temp), FETCH, 0);

    if(ipa && ipa->is_ipa_enabled()) {
        if (!ipa->pmp_ok(paddr, sizeof(fetch_temp))) {
            throw_access_exception((proc) ? proc->state.v : false, paddr, FETCH);
        }
        paddr = ipa->translate(paddr, sizeof(fetch_temp));
        if (IPA_INVALID_ADDR == paddr) {
            throw_access_exception((proc) ? proc->state.v : false, paddr, FETCH);
        }
    }

  if ((host_addr = (proc&&bank) ? bank->npc_addr_to_mem(paddr,idxinbank) : nullptr) ||
        (host_addr = bank ? bank->bank_addr_to_mem(paddr) : nullptr) ||
        (host_addr = sim->addr_to_mem(paddr))) {

    return refill_tlb(vaddr, paddr, host_addr, FETCH);
  } else {
    if (!(((proc&&bank) && bank->npc_mmio_load(paddr, sizeof fetch_temp, (uint8_t*)&fetch_temp,idxinbank)) ||
        (bank && bank->bank_mmio_load(paddr, sizeof fetch_temp, (uint8_t*)&fetch_temp)) ||
        (sim->mmio_load(paddr, sizeof fetch_temp, (uint8_t*)&fetch_temp)))) {

      throw trap_instruction_access_fault(proc->state.v, vaddr, 0, 0);
    }
    tlb_entry_t entry = {(char*)&fetch_temp - vaddr, paddr - vaddr};
    return entry;
  }
}

reg_t reg_from_bytes(size_t len, const uint8_t* bytes)
{
  switch (len) {
    case 1:
      return bytes[0];
    case 2:
      return bytes[0] |
        (((reg_t) bytes[1]) << 8);
    case 4:
      return bytes[0] |
        (((reg_t) bytes[1]) << 8) |
        (((reg_t) bytes[2]) << 16) |
        (((reg_t) bytes[3]) << 24);
    case 8:
      return bytes[0] |
        (((reg_t) bytes[1]) << 8) |
        (((reg_t) bytes[2]) << 16) |
        (((reg_t) bytes[3]) << 24) |
        (((reg_t) bytes[4]) << 32) |
        (((reg_t) bytes[5]) << 40) |
        (((reg_t) bytes[6]) << 48) |
        (((reg_t) bytes[7]) << 56);
  }
  abort();
}

bool mmu_t::mmio_ok(reg_t addr, access_type type)
{
  // Disallow access to debug region when not in debug mode
  if (addr >= DEBUG_START && addr <= DEBUG_END && proc && !proc->state.debug_mode)
    return false;

  return true;
}

bool mmu_t::mmio_load(reg_t addr, size_t len, uint8_t* bytes)
{
    if (!mmio_ok(addr, LOAD))
        return false;

    if ((bank->bank_mmio_load(addr, len, bytes)) || (sim->mmio_load(addr, len, bytes))) {
        return true;
    }
    return false;
}

bool mmu_t::mmio_store(reg_t addr, size_t len, const uint8_t* bytes)
{
    if (!mmio_ok(addr, STORE))
        return false;
        
    if ((bank->bank_mmio_store(addr, len, bytes)) || (sim->mmio_store(addr, len, bytes))) {
        return true;
    }
    return false;
}

void mmu_t::load_slow_path(reg_t addr, reg_t len, uint8_t* bytes, uint32_t xlate_flags)
{
  char *host_addr = nullptr;
  int bankid = bank ? bank->get_bankid() : 0;
  int idxinbank = proc ? proc->get_idxinbank() : 0;
  reg_t paddr = translate(addr, len, LOAD, xlate_flags);
    if(ipa && ipa->is_ipa_enabled()) {
        if (!ipa->pmp_ok(paddr, len)) {
            throw_access_exception((proc) ? proc->state.v : false, paddr, LOAD);
        }
        paddr = ipa->translate(paddr, len);
        if (IPA_INVALID_ADDR == paddr) {
            throw_access_exception((proc) ? proc->state.v : false, paddr, LOAD);
        }
    }

  if ((host_addr = (proc&&bank) ? bank->npc_addr_to_mem(paddr,idxinbank) : nullptr) ||
        (host_addr = bank ? bank->bank_addr_to_mem(paddr) : nullptr) ||
        (host_addr = sim->addr_to_mem(paddr))) {

    memcpy(bytes, host_addr, len);
    if (tracer.interested_in_range(paddr, paddr + PGSIZE, LOAD))
      tracer.trace(paddr, len, LOAD);
    else
      refill_tlb(addr, paddr, host_addr, LOAD);
  } else if (!(((proc&&bank) && bank->npc_mmio_load(paddr, len, bytes,idxinbank)) ||
        (bank && bank->bank_mmio_load(paddr, len, bytes)) ||
        (sim->mmio_load(paddr, len, bytes)))) {

    throw trap_load_access_fault((proc) ? proc->state.v : false, addr, 0, 0);
  }

  if (!matched_trigger) {
    reg_t data = reg_from_bytes(len, bytes);
    matched_trigger = trigger_exception(OPERATION_LOAD, addr, data);
    if (matched_trigger)
      throw *matched_trigger;
  }
}

void mmu_t::store_slow_path(reg_t addr, reg_t len, const uint8_t* bytes, uint32_t xlate_flags)
{
  char *host_addr = nullptr;
  int bankid = bank ? bank->get_bankid() : 0;
  int idxinbank = proc ? proc->get_idxinbank() : 0;
  reg_t paddr = translate(addr, len, STORE, xlate_flags);
    if(ipa && ipa->is_ipa_enabled()) {
        if (!ipa->pmp_ok(paddr, len)) {
            throw_access_exception((proc) ? proc->state.v : false, paddr, STORE);
        }
        paddr = ipa->translate(paddr, len);
        if (IPA_INVALID_ADDR == paddr) {
            throw_access_exception((proc) ? proc->state.v : false, paddr, STORE);
        }
    }

  if (!matched_trigger) {
    reg_t data = reg_from_bytes(len, bytes);
    matched_trigger = trigger_exception(OPERATION_STORE, addr, data);
    if (matched_trigger)
      throw *matched_trigger;
  }

  if ((host_addr = (proc&&bank) ? bank->npc_addr_to_mem(paddr,idxinbank) : nullptr) ||
        (host_addr = bank ? bank->bank_addr_to_mem(paddr) : nullptr) ||
        (host_addr = sim->addr_to_mem(paddr))) {

    memcpy(host_addr, bytes, len);
    if (tracer.interested_in_range(paddr, paddr + PGSIZE, STORE))
      tracer.trace(paddr, len, STORE);
    else
      refill_tlb(addr, paddr, host_addr, STORE);
  } else if (!(((proc&&bank) && bank->npc_mmio_store(paddr, len, bytes,idxinbank)) ||
        (bank && bank->bank_mmio_store(paddr, len, bytes)) ||
        (sim->mmio_store(paddr, len, bytes)))) {

    throw trap_store_access_fault((proc) ? proc->state.v : false, addr, 0, 0);
  }
}

/* 访问npc核内的 8M npc local region,不经过mmu和ipa */
size_t mmu_t::npc_addr_to_mem(reg_t paddr)
{
  if (proc->get_xlen() == 32) {
    paddr &= 0xffffffff;
  }
  if (proc) {
      return (size_t)bank->npc_addr_to_mem(paddr, proc->get_idxinbank());
  } else {
      return 0;
  }
}

/**
 * mte访存特点:
 * 1) 访问范围, llb, l1
 * 2) 不经过 mmu , 经过 ipa at转换 
 * 3) l1访存会跨核跨bank
*/
char * mmu_t::mte_addr_to_mem(reg_t paddr, int procid)
{
    int len = 1;
    char *host_addr = nullptr;
    int bankid = sim->get_bankid(sim->coreid_to_idxinsim(procid));
    int idxinbank = sim->get_idxinbank(sim->coreid_to_idxinsim(procid));

    if (proc->get_xlen() == 32) {
        paddr &= 0xffffffff;
    }

    if(ipa && ipa->is_ipa_enabled()) {
        if (!ipa->pmp_ok(paddr, len)) {
            throw_access_exception((proc) ? proc->state.v : false, paddr, LOAD);
        }
        paddr = ipa->translate(paddr, len);
        if (IPA_INVALID_ADDR == paddr) {
            throw_access_exception((proc) ? proc->state.v : false, paddr, LOAD);
        }
    }

    if(IS_NPC_LOCAL_REGION(paddr)) {
        host_addr = sim->npc_addr_to_mem(paddr, bankid, idxinbank);
    } else {
        host_addr = sim->addr_to_mem(paddr);
    }

    // if (nullptr) {
    //     throw trap_load_access_fault((proc) ? proc->state.v : false, paddr, 0, 0);
    // }
    return host_addr;
}

char * mmu_t::mte_addr_to_mem(reg_t paddr)
{
    int len = 1;
    char *host_addr = nullptr;
    int idxinbank = proc ? proc->get_idxinbank() : 0;

    if (proc->get_xlen() == 32) {
        paddr &= 0xffffffff;
    }

    if(ipa && ipa->is_ipa_enabled()) {
        if (!ipa->pmp_ok(paddr, len)) {
            throw_access_exception((proc) ? proc->state.v : false, paddr, LOAD);
        }
        paddr = ipa->translate(paddr, len);
        if (IPA_INVALID_ADDR == paddr) {
            throw_access_exception((proc) ? proc->state.v : false, paddr, LOAD);
        }
    }

    if(IS_NPC_LOCAL_REGION(paddr)) {
        host_addr = bank->npc_addr_to_mem(paddr, idxinbank);
    } else {
        host_addr = sim->addr_to_mem(paddr);
    }

    // if (nullptr==host_addr) {
    //     throw trap_load_access_fault((proc) ? proc->state.v : false, paddr, 0, 0);
    // }
    return host_addr;
}

/* 执行dmae指令产生的mmu问题和ipa at问题报中断，而不是trap */
void mmu_t::dmae_smmu_trap(reg_t paddr, int channel)
{
    uint32_t mcu_irq_bit = 0;
    uint32_t mcu_irq_status = 0;

    mcu_irq_bit = channel + MCU_IRQ_STATUS_BIT_DMA0_SMMU0;
    if (MCU_IRQ_STATUS_BIT_DMA3_SMMU0 < mcu_irq_bit) {
        mcu_irq_bit = MCU_IRQ_STATUS_BIT_DMA3_SMMU0;
    }

    proc->mmio_load(MISC_START+MCU_IRQ_STATUS_OFFSET, 4, (uint8_t*)(&mcu_irq_status));
    mcu_irq_status |= (1<<mcu_irq_bit);
    proc->misc_dev->ro_register_write(MCU_IRQ_STATUS_OFFSET, (uint32_t)mcu_irq_status);
}

void mmu_t::dmae_ipa_trap(reg_t paddr, int channel)
{
    uint32_t mcu_irq_bit = 0;
    uint32_t mcu_irq_status = 0;

    mcu_irq_bit = channel + MCU_IRQ_STATUS_BIT_DMA0_ATU0;
    if (MCU_IRQ_STATUS_BIT_DMA3_ATU0 < mcu_irq_bit) {
        mcu_irq_bit = MCU_IRQ_STATUS_BIT_DMA3_ATU0;
    }

    proc->mmio_load(MISC_START+MCU_IRQ_STATUS_OFFSET, 4, (uint8_t*)(&mcu_irq_status));
    mcu_irq_status |= (1<<mcu_irq_bit);
    proc->misc_dev->ro_register_write(MCU_IRQ_STATUS_OFFSET, (uint32_t)mcu_irq_status);
}

/* dmae的smmu,与mmu使用相同的页表项, 区别是出错时产生中断而不是trap */
reg_t mmu_t::smmu_translate(reg_t addr, reg_t len, reg_t channel, access_type type, uint32_t xlate_flags)
{
  if (!proc)
    return addr;

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

  reg_t paddr = walk(addr, type, mode, virt, mxr) | (addr & (PGSIZE-1));
  if (!pma_ok(paddr, len, type,!!(xlate_flags&RISCV_XLATE_AMO_FLAG)))
    dmae_smmu_trap(addr, channel);
  if (!pmp_ok(paddr, len, type, mode))
    dmae_smmu_trap(addr, channel);
  return paddr;
}

/**
 * dmae 访存特点:
 * 1) 访问范围 l1, llb, glb
 * 2) 经过 mmu 和 ipa at
*/
char * mmu_t::dmae_addr_to_mem(reg_t paddr, reg_t len, reg_t channel,access_type type, uint32_t xlate_flags)
{
    char *host_addr = nullptr;
    int idxinbank = proc ? proc->get_idxinbank() : 0;

    if (proc->get_xlen() == 32) {
        paddr &= 0xffffffff;
    }

    paddr = smmu_translate(paddr, len, channel, type, xlate_flags);
    if(ipa && ipa->is_ipa_enabled()) {
        if (!ipa->pmp_ok(paddr, len)) {
            dmae_ipa_trap(paddr, channel);
            return nullptr;
        }
        paddr = ipa->translate(paddr, len);
        if (IPA_INVALID_ADDR == paddr) {
            dmae_ipa_trap(paddr, channel);
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

tlb_entry_t mmu_t::refill_tlb(reg_t vaddr, reg_t paddr, char* host_addr, access_type type)
{
  reg_t idx = (vaddr >> PGSHIFT) % TLB_ENTRIES;
  reg_t expected_tag = vaddr >> PGSHIFT;

  if ((tlb_load_tag[idx] & ~TLB_CHECK_TRIGGERS) != expected_tag)
    tlb_load_tag[idx] = -1;
  if ((tlb_store_tag[idx] & ~TLB_CHECK_TRIGGERS) != expected_tag)
    tlb_store_tag[idx] = -1;
  if ((tlb_insn_tag[idx] & ~TLB_CHECK_TRIGGERS) != expected_tag)
    tlb_insn_tag[idx] = -1;

  if ((check_triggers_fetch && type == FETCH) ||
      (check_triggers_load && type == LOAD) ||
      (check_triggers_store && type == STORE))
    expected_tag |= TLB_CHECK_TRIGGERS;

  if (pmp_homogeneous(paddr & ~reg_t(PGSIZE - 1), PGSIZE)) {
    if (type == FETCH) tlb_insn_tag[idx] = expected_tag;
    else if (type == STORE) tlb_store_tag[idx] = expected_tag;
    else tlb_load_tag[idx] = expected_tag;
  }

  tlb_entry_t entry = {host_addr - vaddr, paddr - vaddr};
  tlb_data[idx] = entry;
  return entry;
}

reg_t mmu_t::pmp_ok(reg_t addr, reg_t len, access_type type, reg_t mode)
{
  if (!proc || proc->n_pmp == 0)
    return true;

  reg_t base = 0;
  for (size_t i = 0; i < proc->n_pmp; i++) {
    reg_t tor = (proc->state.pmpaddr[i] & proc->pmp_tor_mask()) << PMP_SHIFT;
    uint8_t cfg = proc->state.pmpcfg[i];

    if (cfg & PMP_A) {
      bool is_tor = (cfg & PMP_A) == PMP_TOR;
      bool is_na4 = (cfg & PMP_A) == PMP_NA4;

      reg_t mask = (proc->state.pmpaddr[i] << 1) | (!is_na4) | ~proc->pmp_tor_mask();
      mask = ~(mask & ~(mask + 1)) << PMP_SHIFT;

      // Check each 4-byte sector of the access
      bool any_match = false;
      bool all_match = true;
      for (reg_t offset = 0; offset < len; offset += 1 << PMP_SHIFT) {
        reg_t cur_addr = addr + offset;
        bool napot_match = ((cur_addr ^ tor) & mask) == 0;
        bool tor_match = base <= cur_addr && cur_addr < tor;
        bool match = is_tor ? tor_match : napot_match;
        any_match |= match;
        all_match &= match;
      }

      if (any_match) {
        // If the PMP matches only a strict subset of the access, fail it
        if (!all_match)
          return false;

        return
          (mode == PRV_M && !(cfg & PMP_L)) ||
          (type == LOAD && (cfg & PMP_R)) ||
          (type == STORE && (cfg & PMP_W)) ||
          (type == FETCH && (cfg & PMP_X));
      }
    }

    base = tor;
  }

  return mode == PRV_M;
}

reg_t mmu_t::pmp_homogeneous(reg_t addr, reg_t len)
{
  if ((addr | len) & (len - 1))
    abort();

  if (!proc)
    return true;

  reg_t base = 0;
  for (size_t i = 0; i < proc->n_pmp; i++) {
    reg_t tor = (proc->state.pmpaddr[i] & proc->pmp_tor_mask()) << PMP_SHIFT;
    uint8_t cfg = proc->state.pmpcfg[i];

    if (cfg & PMP_A) {
      bool is_tor = (cfg & PMP_A) == PMP_TOR;
      bool is_na4 = (cfg & PMP_A) == PMP_NA4;

      bool begins_after_lower = addr >= base;
      bool begins_after_upper = addr >= tor;
      bool ends_before_lower = (addr & -len) < (base & -len);
      bool ends_before_upper = (addr & -len) < (tor & -len);
      bool tor_homogeneous = ends_before_lower || begins_after_upper ||
        (begins_after_lower && ends_before_upper);

      reg_t mask = (proc->state.pmpaddr[i] << 1) | (!is_na4) | ~proc->pmp_tor_mask();
      mask = ~(mask & ~(mask + 1)) << PMP_SHIFT;
      bool mask_homogeneous = ~(mask << 1) & len;
      bool napot_homogeneous = mask_homogeneous || ((addr ^ tor) / len) != 0;

      if (!(is_tor ? tor_homogeneous : napot_homogeneous))
        return false;
    }

    base = tor;
  }

  return true;
}

reg_t mmu_t::pma_ok(reg_t addr, reg_t len, access_type type, bool is_amo)
{
  if (!proc || proc->n_pma == 0)
    return true;

  for (size_t i = 0; i < proc->n_pma; i++) {
    reg_t tor = proc->state.pmaaddr[i] << PMA_SHIFT;
    uint8_t cfg = proc->state.pmacfg[i];

    if (cfg & PMA_ETYP) {
      reg_t mask = (proc->state.pmaaddr[i] << 1) | (!false);
      mask = ~(mask & ~(mask + 1)) << PMA_SHIFT;

      // Check each 4-byte sector of the access
      bool any_match = false;
      bool all_match = true;
      for (reg_t offset = 0; offset < len; offset += 1 << PMA_SHIFT) {
        reg_t cur_addr = addr + offset;
        bool napot_match = ((cur_addr ^ tor) & mask) == 0;
        bool match = napot_match;
        any_match |= match;
        all_match &= match;
      }

      if (any_match) {
        // If the PMP matches only a strict subset of the access, fail it
        if (!all_match)
          return false;

        if ((15 == (cfg&PMA_MTYP)>>2) || (is_amo && (cfg&PMA_NAMO)))
          return false;
        else
          return true;
      }
    }
  }

  return true;
}

reg_t mmu_t::s2xlate(reg_t gva, reg_t gpa, access_type type, access_type trap_type, bool virt, bool mxr)
{
  if (!virt)
    return gpa;

  vm_info vm = decode_vm_info(proc->max_xlen, true, 0, proc->get_state()->hgatp);
  if (vm.levels == 0)
    return gpa;

  reg_t base = vm.ptbase;
  for (int i = vm.levels - 1; i >= 0; i--) {
    int ptshift = i * vm.idxbits;
    int idxbits = (i == (vm.levels - 1)) ? vm.idxbits + vm.widenbits : vm.idxbits;
    reg_t idx = (gpa >> (PGSHIFT + ptshift)) & ((reg_t(1) << idxbits) - 1);

    // check that physical address of PTE is legal
    auto pte_paddr = base + idx * vm.ptesize;
    char *ppte = nullptr;
    if (!(bank && (ppte=bank->bank_addr_to_mem(pte_paddr)))) {
        ppte = sim->addr_to_mem(pte_paddr);
    }
    
    if (!ppte || !pma_ok(pte_paddr, vm.ptesize, LOAD)) {
      throw_access_exception(virt, gva, trap_type);
    }
    if (!ppte || !pmp_ok(pte_paddr, vm.ptesize, LOAD, PRV_S)) {
      throw_access_exception(virt, gva, trap_type);
    }

    reg_t pte = vm.ptesize == 4 ? from_target(*(target_endian<uint32_t>*)ppte) : from_target(*(target_endian<uint64_t>*)ppte);
    reg_t ppn = (pte & ~reg_t(PTE_N)) >> PTE_PPN_SHIFT;

    if (PTE_TABLE(pte)) { // next level of page table
      base = ppn << PGSHIFT;
    } else if (!(pte & PTE_V) || (!(pte & PTE_R) && (pte & PTE_W))) {
      break;
    } else if (!(pte & PTE_U)) {
      break;
    } else if (type == FETCH ? !(pte & PTE_X) :
               type == LOAD ?  !(pte & PTE_R) && !(mxr && (pte & PTE_X)) :
                               !((pte & PTE_R) && (pte & PTE_W))) {
      break;
    } else if ((ppn & ((reg_t(1) << ptshift) - 1)) != 0) {
      break;
    } else {
      reg_t ad = PTE_A | ((type == STORE) * PTE_D);
#ifdef RISCV_ENABLE_DIRTY
      // set accessed and possibly dirty bits.
      if ((pte & ad) != ad) {
        if (!pma_ok(pte_paddr, vm.ptesize, STORE))
          throw_access_exception(virt, gva, trap_type);
        if (!pmp_ok(pte_paddr, vm.ptesize, STORE, PRV_S))
          throw_access_exception(virt, gva, trap_type);
        *(target_endian<uint32_t>*)ppte |= to_target((uint32_t)ad);
      }
#else
      // take exception if access or possibly dirty bit is not set.
      if ((pte & ad) != ad)
        break;
#endif
      reg_t vpn = gpa >> PGSHIFT;
      reg_t page_mask = (reg_t(1) << PGSHIFT) - 1;

      int napot_bits = ((pte & PTE_N) ? (ctz(ppn) + 1) : 0);
      if (((pte & PTE_N) && (ppn == 0 || i != 0)) || (napot_bits != 0 && napot_bits != 4))
        break;

      reg_t page_base = ((ppn & ~((reg_t(1) << napot_bits) - 1))
                        | (vpn & ((reg_t(1) << napot_bits) - 1))
                        | (vpn & ((reg_t(1) << ptshift) - 1))) << PGSHIFT;
      return page_base | (gpa & page_mask);
    }
  }

  switch (trap_type) {
    case FETCH: throw trap_instruction_guest_page_fault(gva, gpa >> 2, 0);
    case LOAD: throw trap_load_guest_page_fault(gva, gpa >> 2, 0);
    case STORE: throw trap_store_guest_page_fault(gva, gpa >> 2, 0);
    default: abort();
  }
}

reg_t mmu_t::walk(reg_t addr, access_type type, reg_t mode, bool virt, bool mxr)
{
  reg_t page_mask = (reg_t(1) << PGSHIFT) - 1;
  reg_t satp = (virt) ? proc->get_state()->vsatp : proc->get_state()->satp;
  vm_info vm = decode_vm_info(proc->max_xlen, false, mode, satp);
  if (vm.levels == 0)
    return s2xlate(addr, addr & ((reg_t(2) << (proc->xlen-1))-1), type, type, virt, mxr) & ~page_mask; // zero-extend from xlen

  bool s_mode = mode == PRV_S;
  bool sum = get_field(proc->state.mstatus, MSTATUS_SUM);

  // verify bits xlen-1:va_bits-1 are all equal
  int va_bits = PGSHIFT + vm.levels * vm.idxbits;
  reg_t mask = (reg_t(1) << (proc->xlen - (va_bits-1))) - 1;
  reg_t masked_msbs = (addr >> (va_bits-1)) & mask;
  if (masked_msbs != 0 && masked_msbs != mask)
    vm.levels = 0;

  reg_t base = vm.ptbase;
  for (int i = vm.levels - 1; i >= 0; i--) {
    int ptshift = i * vm.idxbits;
    reg_t idx = (addr >> (PGSHIFT + ptshift)) & ((1 << vm.idxbits) - 1);

    // check that physical address of PTE is legal
    auto pte_paddr = s2xlate(addr, base + idx * vm.ptesize, LOAD, type, virt, false);
    char *ppte = nullptr;
    if (!(bank && (ppte=bank->bank_addr_to_mem(pte_paddr)))) {
        ppte = sim->addr_to_mem(pte_paddr);
    }
    if (!ppte || !pma_ok(pte_paddr, vm.ptesize, LOAD))
      throw_access_exception(virt, addr, type);
    if (!ppte || !pmp_ok(pte_paddr, vm.ptesize, LOAD, PRV_S))
      throw_access_exception(virt, addr, type);

    reg_t pte = vm.ptesize == 4 ? from_target(*(target_endian<uint32_t>*)ppte) : from_target(*(target_endian<uint64_t>*)ppte);
    reg_t ppn = (pte & ~reg_t(PTE_N)) >> PTE_PPN_SHIFT;

    if (PTE_TABLE(pte)) { // next level of page table
      base = ppn << PGSHIFT;
    } else if ((pte & PTE_U) ? s_mode && (type == FETCH || !sum) : !s_mode) {
      break;
    } else if (!(pte & PTE_V) || (!(pte & PTE_R) && (pte & PTE_W))) {
      break;
    } else if (type == FETCH ? !(pte & PTE_X) :
               type == LOAD ?  !(pte & PTE_R) && !(mxr && (pte & PTE_X)) :
                               !((pte & PTE_R) && (pte & PTE_W))) {
      break;
    } else if ((ppn & ((reg_t(1) << ptshift) - 1)) != 0) {
      break;
    } else {
      reg_t ad = PTE_A | ((type == STORE) * PTE_D);
#ifdef RISCV_ENABLE_DIRTY
      // set accessed and possibly dirty bits.
      if ((pte & ad) != ad) {
        if (!pma_ok(pte_paddr, vm.ptesize, STORE))
          throw_access_exception(virt, addr, type);
        if (!pmp_ok(pte_paddr, vm.ptesize, STORE, PRV_S))
          throw_access_exception(virt, addr, type);
        *(target_endian<uint32_t>*)ppte |= to_target((uint32_t)ad);
      }
#else
      // take exception if access or possibly dirty bit is not set.
      if ((pte & ad) != ad)
        break;
#endif
      // for superpage or Zsn NAPOT mappings, make a fake leaf PTE for the TLB's benefit.
      reg_t vpn = addr >> PGSHIFT;

      int napot_bits = ((pte & PTE_N) ? (ctz(ppn) + 1) : 0);
      if (((pte & PTE_N) && (ppn == 0 || i != 0)) || (napot_bits != 0 && napot_bits != 4))
        break;

      reg_t page_base = ((ppn & ~((reg_t(1) << napot_bits) - 1))
                        | (vpn & ((reg_t(1) << napot_bits) - 1))
                        | (vpn & ((reg_t(1) << ptshift) - 1))) << PGSHIFT;
      reg_t phys = page_base | (addr & page_mask);
      return s2xlate(addr, phys, type, type, virt, mxr) & ~page_mask;
    }
  }

  switch (type) {
    case FETCH: throw trap_instruction_page_fault(virt, addr, 0, 0);
    case LOAD: throw trap_load_page_fault(virt, addr, 0, 0);
    case STORE: throw trap_store_page_fault(virt, addr, 0, 0);
    default: abort();
  }
}

void mmu_t::register_memtracer(memtracer_t* t)
{
  flush_tlb();
  tracer.hook(t);
}
