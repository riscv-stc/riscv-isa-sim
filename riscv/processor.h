// See LICENSE for license details.
#ifndef _RISCV_PROCESSOR_H
#define _RISCV_PROCESSOR_H

#include "decode.h"
#include "config.h"
#include "devices.h"
#include "trap.h"
#include "simif.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <cassert>
#include <functional>
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

// regnum, data
typedef std::unordered_map<reg_t, freg_t> commit_log_reg_t;

// addr, value, size
typedef std::vector<std::tuple<reg_t, uint64_t, uint32_t>> commit_log_mem_t;
// addr, value, inst_type
typedef std::vector<std::tuple< reg_t, void *, uint32_t>> commit_log_mem_stc_t; 

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

enum VRM{
  RNU = 0,
  RNE,
  RDN,
  ROD,
  INVALID_RM
};

template<uint64_t N>
struct type_usew_t;

template<>
struct type_usew_t<8>
{
  using type=uint8_t;
};

template<>
struct type_usew_t<16>
{
  using type=uint16_t;
};

template<>
struct type_usew_t<32>
{
  using type=uint32_t;
};

template<>
struct type_usew_t<64>
{
  using type=uint64_t;
};

template<uint64_t N>
struct type_sew_t;

template<>
struct type_sew_t<8>
{
  using type=int8_t;
};

template<>
struct type_sew_t<16>
{
  using type=int16_t;
};

template<>
struct type_sew_t<32>
{
  using type=int32_t;
};

template<>
struct type_sew_t<64>
{
  using type=int64_t;
};

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
  bool v;
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

  reg_t mtval2;
  reg_t mtinst;
  reg_t hstatus;
  reg_t hideleg;
  reg_t hedeleg;
  uint32_t hcounteren;
  reg_t htval;
  reg_t htinst;
  reg_t hgatp;
  reg_t vsstatus;
  reg_t vstvec;
  reg_t vsscratch;
  reg_t vsepc;
  reg_t vscause;
  reg_t vstval;
  reg_t vsatp;

  reg_t dpc;
  reg_t dscratch0, dscratch1;
  dcsr_t dcsr;
  reg_t mhsp_ctl;
  reg_t msp_bound;
  reg_t msp_base;
  reg_t tselect;
  mcontrol_t mcontrol[num_triggers];
  reg_t tdata2[num_triggers];
  bool debug_mode;

  static const int max_pmp = 16;
  uint8_t pmpcfg[max_pmp];
  reg_t pmpaddr[max_pmp];

  uint32_t fflags;
  uint32_t frm;
  uint32_t vme_shape_s;
  uint32_t vme_stride_d;
  uint32_t vme_stride_s;
  uint32_t mme_shape_s1;
  uint32_t mme_shape_s2;
  uint32_t mme_stride_d;
  uint32_t mme_stride_s;
  
  uint32_t vstart;
  uint32_t vxsat;
  uint32_t vxrm;
  uint32_t vl;
  uint32_t vtype;
  const uint32_t vlenb = VREG_LENGTH;
  
  uint32_t conv_FM_in;
  uint32_t conv_Cin;
  uint32_t conv_FM_out;
  uint32_t conv_Cout;
  uint32_t conv_kernel_params1;
  uint32_t conv_kernel_params2;
  uint32_t conv_padding;
  uint32_t mme_dequant_coeff;
  uint32_t mme_quant_coeff;
  uint32_t mme_sparseidx_base;
  uint32_t mme_sparseidx_stride;
  uint32_t vme_data_type;
  uint32_t mme_data_type;
  uint32_t vme_relu_threshhold;
  uint32_t vme_FM_in;
  uint32_t vme_Cin;
  uint32_t vme_FM_out;
  uint32_t vme_Cout;
  uint32_t vme_kernel_param1;
  uint32_t vme_FM_padding;
  uint32_t vme_kernel_param2;
  uint32_t vme_max_min_idx;
  uint32_t conv_dequant_coeff;
  uint32_t ncp_busy;
  uint32_t mte_icdest;
  uint32_t mte_shape;
  uint32_t mte_stride_s;
  uint32_t mte_stride_d;
  uint32_t mte_data_type;
  uint32_t dmae_data_type;
  uint32_t dmae_shape_1;
  uint32_t dmae_shape_2 = 1; //default z=1
  uint32_t dmae_stride_s1;
  uint32_t dmae_stride_s2;
  uint32_t dmae_stride_d1;
  uint32_t dmae_stride_d2;
  uint32_t dmae_ctrl;
  uint32_t tmisc;
  uint32_t tcsr;
  uint32_t dma_shape_row;
  uint32_t dma_shape_col;
  uint32_t dma_stride_ddr;
  reg_t wfi_flag;
  /* mextip is ext interrupt pending status for mbox,
   * just effect mip ext interrupt bit. */
  volatile reg_t mextip;
  bool serialized; // whether timer CSRs are in a well-defined state

  bool async_started = false;
  bool pld = false;
  // When true, execute a single instruction and then enter debug mode.  This
  // can only be set by executing dret.
  enum {
      STEP_NONE,
      STEP_STEPPING,
      STEP_STEPPED
  } single_step;

#ifdef RISCV_ENABLE_COMMITLOG
  commit_log_reg_t log_reg_write;
  commit_log_mem_t log_mem_read;
  commit_log_mem_t log_mem_write;
  commit_log_mem_stc_t log_mem_stc_write; 
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

typedef enum {
  // 65('A') ~ 90('Z') is reserved for standard isa in misa
  EXT_ZFH   = 0,
  EXT_ZVEDIV,
} isa_extension_t;

typedef enum {
  IMPL_MMU_SV32,
  IMPL_MMU_SV39,
  IMPL_MMU_SV48,
  IMPL_MMU_BARE,
  IMPL_MMU,
} impl_extension_t;

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
  processor_t(const char* isa, const char* priv, const char* varch,
              simif_t* sim, hwsync_t *hs, uint32_t idx,
              uint32_t id, bool halt_on_reset,
              FILE *log_file);
  ~processor_t();

  void set_debug(bool value);
  void set_histogram(bool value);
#ifdef RISCV_ENABLE_COMMITLOG
  void enable_log_commits();
  bool get_log_commits_enabled() const { return log_commits_enabled; }
#endif
  void reset();
  void step(size_t n); // run for n cycles
  void set_rx_active();
  void set_vtype(reg_t val);
  void set_csr(int which, reg_t val);
  reg_t get_csr(int which, insn_t insn, bool write, bool peek = 0);
  reg_t get_csr(int which) { return get_csr(which, insn_t(0), false, true); }
  mmu_t* get_mmu() { return mmu; }
  simif_t* get_sim() { return sim; };
  uint32_t get_syncs() {return synctimes; };
  uint32_t set_syncs(uint32_t times) {return synctimes = times; };
  uint32_t get_hwsync_status();
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
    if (ext >= 'A' && ext <= 'Z')
      return ((state.misa >> (ext - 'A')) & 1);
    else
      return extension_table[ext];
  }
  void set_impl(uint8_t impl, bool val) { impl_table[impl] = val; }
  bool supports_impl(uint8_t impl) const {
    return impl_table[impl];
  }
  reg_t pc_alignment_mask() {
    return ~(reg_t)(supports_extension('C') ? 0 : 2);
  }
  void check_pc_alignment(reg_t pc) {
    if (unlikely(pc & ~pc_alignment_mask()))
      throw trap_instruction_address_misaligned(pc, 0, 0);
  }
  reg_t legalize_privilege(reg_t);
  void set_privilege(reg_t);
  void set_virt(bool);
  void update_histogram(reg_t pc);
  const disassembler_t* get_disassembler() { return disassembler; }

  FILE *get_log_file() { return log_file; }

  void register_insn(insn_desc_t);
  void register_extension(extension_t*);

  void sync();
  void pld(uint32_t coremap);
  uint32_t get_sync_group();

  void run_async(std::function<void()> func);
  void run_async(std::function<void()> func, bool flag);
  bool async_done();
  bool async_state() { return state.async_started; };

  void set_exit() { exit_request = true; };
  bool exited() { return exit_request; };

  // MMIO slave interface
  bool load(reg_t addr, size_t len, uint8_t* bytes);
  bool store(reg_t addr, size_t len, const uint8_t* bytes);

  // When true, display disassembly of each instruction that's executed.
  bool debug;
  // When true, take the slow simulation path.
  bool slow_path();
  bool halted() { return state.debug_mode; }
  enum {
    HR_NONE,    /* Halt request is inactive. */
    HR_REGULAR, /* Regular halt request/debug interrupt. */
    HR_GROUP    /* Halt requested due to halt group. */
  } halt_request;

  // Return the index of a trigger that matched, or -1.
  inline int trigger_match(trigger_operation_t operation, reg_t address, reg_t data)
  {
    if (state.debug_mode)
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
            reg_t mask = ~((1 << (cto(state.tdata2[i])+1)) - 1);
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
  mbox_device_t* add_mbox(mbox_device_t *box);

  void set_pmp_num(reg_t pmp_num);
  void set_pmp_granularity(reg_t pmp_granularity);
  void set_mmu_capability(int cap);

  const char* get_symbol(uint64_t addr);

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
  bool log_commits_enabled;
  FILE *log_file;
  bool halt_on_reset;
  mbox_device_t *mbox;
  std::vector<bool> extension_table;
  std::vector<bool> impl_table;
  

  std::vector<insn_desc_t> instructions;
  std::map<reg_t,uint64_t> pc_histogram;

  std::thread *async_thread = nullptr;
  std::function<void()> async_function = nullptr;
  std::exception_ptr async_trap = nullptr;
  std::mutex async_mutex;
  std::condition_variable async_cond;
  bool async_running;
  bool exit_request;

  static const size_t OPCODE_CACHE_SIZE = 8191;
  insn_desc_t opcode_cache[OPCODE_CACHE_SIZE];

  void take_pending_interrupt(reg_t interrupts) { take_interrupt(interrupts & state.mie); }
  void take_interrupt(reg_t mask); // take first enabled interrupt in mask
  void take_trap(trap_t& t, reg_t epc); // take an exception
  void disasm(insn_t insn); // disassemble and print an instruction
  int paddr_bits();

  reg_t pmp_tor_mask() { return -(reg_t(1) << (lg_pmp_granularity - PMP_SHIFT)); }

  void enter_debug_mode(uint8_t cause);

  friend class mmu_t;
  friend class clint_t;
  friend class extension_t;
  friend class pcie_driver_t;
  friend class mbox_device_t;

  void parse_varch_string(const char*);
  void parse_priv_string(const char*);
  void parse_isa_string(const char*);
  void build_opcode_map();
  void register_base_instructions();
  insn_func_t decode_insn(insn_t insn);
  reg_t cal_satp(reg_t val) const;

  // Track repeated executions for processor_t::disasm()
  uint64_t last_pc, last_bits, executions;
  reg_t n_pmp;
  reg_t lg_pmp_granularity;

public:
  class vectorUnit_t {
    public:
      processor_t* p;
      void *reg_file;
      char reg_referenced[NVPR];
      int setvl_count;
      reg_t vlmax;
      reg_t vstart, vxrm, vxsat, vl, vtype, vlenb;
      reg_t vma, vta;
      reg_t vediv, vsew;
      float vflmul;
      reg_t ELEN, VLEN;
      bool vill;
      bool vstart_alu;

      // vector element for varies SEW
      template<class T>
        T& elt(reg_t vReg, reg_t n, bool is_write = false){
          assert(vsew != 0);
          assert((VLEN >> 3)/sizeof(T) > 0);
          reg_t elts_per_reg = (VLEN >> 3) / (sizeof(T));
          vReg += n / elts_per_reg;
          n = n % elts_per_reg;
#ifdef WORDS_BIGENDIAN
          // "V" spec 0.7.1 requires lower indices to map to lower significant
          // bits when changing SEW, thus we need to index from the end on BE.
  	  n ^= elts_per_reg - 1;
#endif
          reg_referenced[vReg] = 1;

#ifdef RISCV_ENABLE_COMMITLOG
          if (is_write)
            p->get_state()->log_reg_write[((vReg) << 4) | 2] = {0, 0};
#endif

          T *regStart = (T*)((char*)reg_file + vReg * (VLEN >> 3));
          return regStart[n];
        }
    public:

      void reset();

      vectorUnit_t(){
        reg_file = 0;
      }

      ~vectorUnit_t(){
        free(reg_file);
        reg_file = 0;
      }

      reg_t set_vl(int rd, int rs1, reg_t reqVL, reg_t newType);

      reg_t get_vlen() { return VLEN; }
      reg_t get_elen() { return ELEN; }
      reg_t get_slen() { return VLEN; }

      VRM get_vround_mode() {
        return (VRM)vxrm;
      }
  };

  vectorUnit_t VU;
};

reg_t illegal_instruction(processor_t* p, insn_t insn, reg_t pc);

#define REGISTER_INSN(proc, name, match, mask) \
  extern reg_t rv32_##name(processor_t*, insn_t, reg_t); \
  extern reg_t rv64_##name(processor_t*, insn_t, reg_t); \
  proc->register_insn((insn_desc_t){match, mask, rv32_##name, rv64_##name});

#endif
