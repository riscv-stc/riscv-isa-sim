// See LICENSE for license details.
#ifndef _RISCV_PROCESSOR_H
#define _RISCV_PROCESSOR_H

#include "decode.h"
#include "config.h"
#include "devices.h"
#include "trap.h"
#include <string>
#include <vector>
#include <map>
#include "simif.h"
#include "debug_rom_defines.h"

class processor_t;
class mmu_t;
typedef reg_t (*insn_func_t)(processor_t*, insn_t, reg_t);
class simif_t;
class hwsync_t;
class trap_t;
class extension_t;
class disassembler_t;

#define VECTOR_REG_LEN    (VREG_LENGTH * 8)
#define VECTOR_STRIP_LEN  (VREG_LENGTH * 8)

struct insn_desc_t
{
  insn_bits_t match;
  insn_bits_t mask;
  insn_func_t rv32;
  insn_func_t rv64;
};

struct commit_log_reg_t
{
  reg_t addr;
  freg_t data;
};

typedef struct
{
  uint8_t prv;
  bool step;
  bool ebreakm;
  bool ebreakh;
  bool ebreaks;
  bool ebreaku;
  bool halt;
  uint8_t cause;
} dcsr_t;

typedef enum
{
  ACTION_DEBUG_EXCEPTION = MCONTROL_ACTION_DEBUG_EXCEPTION,
  ACTION_DEBUG_MODE = MCONTROL_ACTION_DEBUG_MODE,
  ACTION_TRACE_START = MCONTROL_ACTION_TRACE_START,
  ACTION_TRACE_STOP = MCONTROL_ACTION_TRACE_STOP,
  ACTION_TRACE_EMIT = MCONTROL_ACTION_TRACE_EMIT
} mcontrol_action_t;

typedef enum
{
  MATCH_EQUAL = MCONTROL_MATCH_EQUAL,
  MATCH_NAPOT = MCONTROL_MATCH_NAPOT,
  MATCH_GE = MCONTROL_MATCH_GE,
  MATCH_LT = MCONTROL_MATCH_LT,
  MATCH_MASK_LOW = MCONTROL_MATCH_MASK_LOW,
  MATCH_MASK_HIGH = MCONTROL_MATCH_MASK_HIGH
} mcontrol_match_t;

typedef struct
{
  uint8_t type;
  bool dmode;
  uint8_t maskmax;
  bool select;
  bool timing;
  mcontrol_action_t action;
  bool chain;
  mcontrol_match_t match;
  bool m;
  bool h;
  bool s;
  bool u;
  bool execute;
  bool store;
  bool load;
} mcontrol_t;

// architectural state of a RISC-V hart
struct state_t
{
  void reset(reg_t max_isa);

  static const int num_triggers = 4;

  reg_t pc;
  regfile_t<reg_t, NXPR, true> XPR;
  regfile_t<freg_t, NFPR, false> FPR;
  regfile_t<vreg_t, NVPR, false> VPR;

  // control and status registers
  reg_t prv;    // TODO: Can this be an enum instead?
  reg_t misa;
  reg_t mstatus;
  reg_t mepc;
  reg_t mtval;
  reg_t mscratch;
  reg_t mtvec;
  reg_t mcause;
  reg_t minstret;
  reg_t mie;
  reg_t mip;
  reg_t medeleg;
  reg_t mideleg;
  uint32_t mcounteren;
  uint32_t scounteren;
  reg_t sepc;
  reg_t stval;
  reg_t sscratch;
  reg_t stvec;
  reg_t satp;
  reg_t scause;
  reg_t dpc;
  reg_t dscratch;
  dcsr_t dcsr;
  reg_t tselect;
  mcontrol_t mcontrol[num_triggers];
  reg_t tdata2[num_triggers];

  static const int n_pmp = 16;
  uint8_t pmpcfg[n_pmp];
  reg_t pmpaddr[n_pmp];

  uint32_t fflags;
  uint32_t frm;
  
  uint32_t shape_s1;
  uint32_t shape_s2;
  uint32_t stride_d;
  uint32_t stride_s;
  uint32_t m_shape_s1;
  uint32_t m_shape_s2;
  uint32_t m_stride_d;
  uint32_t m_stride_s;
  
  uint32_t vstart;
  uint32_t vxsat;
  uint32_t vxrm;
  uint32_t vl;
  uint32_t vtype;
  const uint32_t vlenb = VREG_LENGTH;
  
  uint32_t conv_FM_in;
  uint32_t conv_Depth_in;
  uint32_t conv_FM_out;
  uint32_t conv_Depth_out;
  uint32_t conv_S_kernel;
  uint32_t conv_kernel;
  uint32_t conv_padding;
  uint32_t  m_dequant_coeff;
  uint32_t  conv_dequant_coeff;
  uint32_t ncp_busy;
  uint32_t mte_coremap;
  uint32_t mte_icdest;
  uint32_t mte_shape; 
  uint32_t mte_stride_llb;

  uint32_t tmisc; 
  uint32_t tcsr;
  
  uint32_t dma_shape_row;
  uint32_t dma_shape_col;
  uint32_t dma_stride_ddr;
  reg_t wfi_flag;
  reg_t interrupt_flag;
  reg_t mextip;
  
  bool serialized; // whether timer CSRs are in a well-defined state

  bool async_started;

  // When true, execute a single instruction and then enter debug mode.  This
  // can only be set by executing dret.
  enum {
      STEP_NONE,
      STEP_STEPPING,
      STEP_STEPPED
  } single_step;

#ifdef RISCV_ENABLE_COMMITLOG
  commit_log_reg_t log_reg_write;
  reg_t last_inst_priv;
  int last_inst_xlen;
  int last_inst_flen;
#endif
};

typedef enum {
  OPERATION_EXECUTE,
  OPERATION_STORE,
  OPERATION_LOAD,
} trigger_operation_t;

// Count number of contiguous 1 bits starting from the LSB.
static int cto(reg_t val)
{
  int res = 0;
  while ((val & 1) == 1)
    val >>= 1, res++;
  return res;
}

// this class represents one processor in a RISC-V machine.
class processor_t : public abstract_device_t
{
public:
  processor_t(const char* isa, simif_t* sim, hwsync_t *hs, uint32_t idx, uint32_t id, bool halt_on_reset=false);
  ~processor_t();

  void set_debug(bool value);
  void set_histogram(bool value);
  void reset();
  void step(size_t n); // run for n cycles
  void set_rx_active();
  void set_vtype(reg_t val);
  void set_csr(int which, reg_t val);
  reg_t get_csr(int which);
  mmu_t* get_mmu() { return mmu; };
  simif_t* get_sim() { return sim; };
  uint32_t get_syncs() {return synctimes; };
  uint32_t set_syncs(uint32_t times) {return synctimes = times; };
  uint32_t get_idx() {return idx; };
  uint32_t get_id() {return id; };
  state_t* get_state() { return &state; }
  unsigned get_xlen() { return xlen; }
  unsigned get_max_xlen() { return max_xlen; }
  unsigned get_elen() { return elen; }
  unsigned get_slen() { return slen; }
  unsigned get_vlen() { return vlen; }
  std::string get_isa_string() { return isa_string; }
  unsigned get_flen() {
    return supports_extension('Q') ? 128 :
           supports_extension('D') ? 64 :
           supports_extension('F') ? 32 : 0;
  }
  extension_t* get_extension() { return ext; }
  bool supports_extension(unsigned char ext) {
    if (ext >= 'a' && ext <= 'z') ext += 'A' - 'a';
    return ext >= 'A' && ext <= 'Z' && ((state.misa >> (ext - 'A')) & 1);
  }
  reg_t pc_alignment_mask() {
    return ~(reg_t)(supports_extension('C') ? 0 : 2);
  }
  void check_pc_alignment(reg_t pc) {
    if (unlikely(pc & ~pc_alignment_mask()))
      throw trap_instruction_address_misaligned(pc);
  }
  reg_t legalize_privilege(reg_t);
  void set_privilege(reg_t);
  void update_histogram(reg_t pc);
  const disassembler_t* get_disassembler() { return disassembler; }

  void register_insn(insn_desc_t);
  void register_extension(extension_t*);

  void sync(uint32_t coremap = 0);

  void run_async(std::function<void()> func);
  bool async_done();

  // MMIO slave interface
  bool load(reg_t addr, size_t len, uint8_t* bytes);
  bool store(reg_t addr, size_t len, const uint8_t* bytes);

  // When true, display disassembly of each instruction that's executed.
  bool debug;
  // When true, take the slow simulation path.
  bool slow_path();
  bool halted() { return state.dcsr.cause ? true : false; }
  bool halt_request;

  // Return the index of a trigger that matched, or -1.
  inline int trigger_match(trigger_operation_t operation, reg_t address, reg_t data)
  {
    if (state.dcsr.cause)
      return -1;

    bool chain_ok = true;

    for (unsigned int i = 0; i < state.num_triggers; i++) {
      if (!chain_ok) {
        chain_ok |= !state.mcontrol[i].chain;
        continue;
      }

      if ((operation == OPERATION_EXECUTE && !state.mcontrol[i].execute) ||
          (operation == OPERATION_STORE && !state.mcontrol[i].store) ||
          (operation == OPERATION_LOAD && !state.mcontrol[i].load) ||
          (state.prv == PRV_M && !state.mcontrol[i].m) ||
          (state.prv == PRV_S && !state.mcontrol[i].s) ||
          (state.prv == PRV_U && !state.mcontrol[i].u)) {
        continue;
      }

      reg_t value;
      if (state.mcontrol[i].select) {
        value = data;
      } else {
        value = address;
      }

      // We need this because in 32-bit mode sometimes the PC bits get sign
      // extended.
      if (xlen == 32) {
        value &= 0xffffffff;
      }

      switch (state.mcontrol[i].match) {
        case MATCH_EQUAL:
          if (value != state.tdata2[i])
            continue;
          break;
        case MATCH_NAPOT:
          {
            reg_t mask = ~((1 << cto(state.tdata2[i])) - 1);
            if ((value & mask) != (state.tdata2[i] & mask))
              continue;
          }
          break;
        case MATCH_GE:
          if (value < state.tdata2[i])
            continue;
          break;
        case MATCH_LT:
          if (value >= state.tdata2[i])
            continue;
          break;
        case MATCH_MASK_LOW:
          {
            reg_t mask = state.tdata2[i] >> (xlen/2);
            if ((value & mask) != (state.tdata2[i] & mask))
              continue;
          }
          break;
        case MATCH_MASK_HIGH:
          {
            reg_t mask = state.tdata2[i] >> (xlen/2);
            if (((value >> (xlen/2)) & mask) != (state.tdata2[i] & mask))
              continue;
          }
          break;
      }

      if (!state.mcontrol[i].chain) {
        return i;
      }
      chain_ok = true;
    }
    return -1;
  }

  void trigger_updated();

private:
  simif_t* sim;
  hwsync_t *hwsync;
  mmu_t* mmu; // main memory is always accessed via the mmu
  extension_t* ext;
  disassembler_t* disassembler;
  state_t state;
  uint32_t idx;
  uint32_t id;
  unsigned max_xlen;
  unsigned xlen;
  unsigned elen;
  unsigned slen;
  unsigned vlen;
  uint32_t synctimes;
  reg_t max_isa;
  std::string isa_string;
  bool histogram_enabled;
  bool halt_on_reset;

  std::vector<insn_desc_t> instructions;
  std::map<reg_t,uint64_t> pc_histogram;

  std::function<void()> async_function = nullptr;
  std::exception_ptr async_trap = nullptr;
  std::mutex async_mutex;
  std::condition_variable async_cond;
  bool async_running;

  static const size_t OPCODE_CACHE_SIZE = 8191;
  insn_desc_t opcode_cache[OPCODE_CACHE_SIZE];

  void take_pending_interrupt() { take_interrupt((state.mip | (state.mextip ? (1 << IRQ_M_EXT) : 0)) & state.mie); }
  void take_interrupt(reg_t mask); // take first enabled interrupt in mask
  void take_trap(trap_t& t, reg_t epc); // take an exception
  void disasm(insn_t insn); // disassemble and print an instruction
  int paddr_bits();

  void enter_debug_mode(uint8_t cause);

  friend class mmu_t;
  friend class clint_t;
  friend class extension_t;
  friend class pcie_driver_t;
  friend class mbox_device_t;
  
  void parse_isa_string(const char* isa);
  void build_opcode_map();
  void register_base_instructions();
  insn_func_t decode_insn(insn_t insn);

  // Track repeated executions for processor_t::disasm()
  uint64_t last_pc, last_bits, executions;
};

reg_t illegal_instruction(processor_t* p, insn_t insn, reg_t pc);

#define REGISTER_INSN(proc, name, match, mask) \
  extern reg_t rv32_##name(processor_t*, insn_t, reg_t); \
  extern reg_t rv64_##name(processor_t*, insn_t, reg_t); \
  proc->register_insn((insn_desc_t){match, mask, rv32_##name, rv64_##name});

#endif
