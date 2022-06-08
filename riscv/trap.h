// See LICENSE for license details.

#ifndef _RISCV_TRAP_H
#define _RISCV_TRAP_H

#include "decode.h"
#include <stdlib.h>

struct state_t;

class trap_t
{
 public:
  trap_t(reg_t which) : which(which) {}
  virtual const char* name();
  virtual bool has_tval() { return false; }
  virtual reg_t get_tval() { return 0; }
  reg_t cause() { return which; }
 private:
  char _name[16];
  reg_t which;
};

class mem_trap_t : public trap_t
{
 public:
  mem_trap_t(reg_t which, reg_t tval)
    : trap_t(which), tval(tval) {}
  bool has_tval() override { return true; }
  reg_t get_tval() override { return tval; }
 private:
  reg_t tval;
};

#define DECLARE_TRAP(n, x) class trap_##x : public trap_t { \
 public: \
  trap_##x() : trap_t(n) {} \
  const char* name() { return "trap_"#x; } \
};

#define DECLARE_MEM_TRAP(n, x) class trap_##x : public mem_trap_t { \
 public: \
  trap_##x(reg_t tval) : mem_trap_t(n, tval) {} \
  const char* name() { return "trap_"#x; } \
};

DECLARE_MEM_TRAP(CAUSE_MISALIGNED_FETCH, instruction_address_misaligned)
DECLARE_MEM_TRAP(CAUSE_FETCH_ACCESS, instruction_access_fault)
DECLARE_MEM_TRAP(CAUSE_ILLEGAL_INSTRUCTION, illegal_instruction)
DECLARE_MEM_TRAP(CAUSE_BREAKPOINT, breakpoint)
DECLARE_MEM_TRAP(CAUSE_MISALIGNED_LOAD, load_address_misaligned)
DECLARE_MEM_TRAP(CAUSE_MISALIGNED_STORE, store_address_misaligned)
DECLARE_MEM_TRAP(CAUSE_LOAD_ACCESS, load_access_fault)
DECLARE_MEM_TRAP(CAUSE_STORE_ACCESS, store_access_fault)
DECLARE_TRAP(CAUSE_USER_ECALL, user_ecall)
DECLARE_TRAP(CAUSE_SUPERVISOR_ECALL, supervisor_ecall)
DECLARE_TRAP(CAUSE_HYPERVISOR_ECALL, hypervisor_ecall)
DECLARE_TRAP(CAUSE_MACHINE_ECALL, machine_ecall)
DECLARE_MEM_TRAP(CAUSE_FETCH_PAGE_FAULT, instruction_page_fault)
DECLARE_MEM_TRAP(CAUSE_LOAD_PAGE_FAULT, load_page_fault)
DECLARE_MEM_TRAP(CAUSE_STORE_PAGE_FAULT, store_page_fault)

DECLARE_TRAP(CAUSE_NCP_ILLEGAL_ENCODING, ncp_illegal_encoding)
DECLARE_TRAP(CAUSE_NCP_VILL_INVALID_INST, ncp_vill_invalid_inst)
DECLARE_TRAP(CAUSE_NCP_WR_VL_VTYPE, ncp_wr_vl_vtype)
DECLARE_TRAP(CAUSE_NCP_RW_ILLEGAL_CSR, ncp_rw_illegal_csr)

DECLARE_MEM_TRAP(CAUSE_NCP_RVV_MISALIGNED_BASE, ncp_rvv_misaligned_base)
DECLARE_MEM_TRAP(CAUSE_NCP_RVV_MISALIGNED_OFFSET, ncp_rvv_misaligned_offset)
DECLARE_MEM_TRAP(CAUSE_NCP_RVV_ACCESS , ncp_rvv_access)
DECLARE_TRAP(CAUSE_NCP_RVV_INVALID_SAME_RDRS, ncp_rvv_invalid_same_rdrs)

DECLARE_MEM_TRAP(CAUSE_NCP_CUST_MISALIGNED_BASE, ncp_cust_misaligned_base)
DECLARE_TRAP(CAUSE_NCP_CUST_INVALID_PARAM, ncp_cust_invalid_param)
DECLARE_MEM_TRAP(CAUSE_NCP_CUST_MISALIGNED_STRIDE, ncp_cust_misaligned_stride)
DECLARE_MEM_TRAP(CAUSE_NCP_CUST_ACCESS, ncp_cust_access)

DECLARE_TRAP(CAUSE_TCP_ILLEGAL_ENCODING, tcp_illegal_encoding)
DECLARE_TRAP(CAUSE_TCP_ICMOV_INVALID_CORE, tcp_icmov_invalid_core)
DECLARE_MEM_TRAP(CAUSE_TCP_ACCESS_START, tcp_access_start)
DECLARE_MEM_TRAP(CAUSE_TCP_ACCESS_START_ICMOV, tcp_access_start_icmov)
DECLARE_MEM_TRAP(CAUSE_TCP_ACCESS_END_L1, tcp_access_end_l1)
DECLARE_MEM_TRAP(CAUSE_TCP_ACCESS_END_LLB, tcp_access_end_llb)
DECLARE_TRAP(CAUSE_TCP_INVALID_PARAM, tcp_invalid_param)
DECLARE_TRAP(CAUSE_TCP_RW_ILLEGAL_CSR, tcp_rw_illegal_csr)

DECLARE_TRAP(CAUSE_STACK_OVERFLOW_EXCEPTION, stack_overflow_exception)
DECLARE_TRAP(CAUSE_STACK_UNDERFLOW_EXCEPTION, stack_underflow_exception)
DECLARE_TRAP(CAUSE_NCP_SYNC_TIMEOUT_TRIGGER, sync_timeout_trigger)
#endif
