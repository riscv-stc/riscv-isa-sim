// See LICENSE for license details.

#ifndef _RISCV_DECODE_H
#define _RISCV_DECODE_H

#if (-1 != ~0) || ((-1 >> 1) != -1)
# error spike requires a two''s-complement c++ implementation
#endif

#include <algorithm>
#include <cstdint>
#include <string.h>
#include <strings.h>
#include "encoding.h"
#include "config.h"
#include "common.h"
#include "softfloat_types.h"
#include "specialize.h"
#include "eigen3_ops.h"
#include <cinttypes>

using namespace std;

typedef int64_t sreg_t;
typedef uint64_t reg_t;

#ifdef __SIZEOF_INT128__
typedef __int128 int128_t;
typedef unsigned __int128 uint128_t;
#endif

const int NXPR = 32;
const int NFPR = 32;
const int NVPR = 32;
const int NCSR = 4096;

#define X_RA 1
#define X_SP 2
#define X_GP 3

#define VCSR_VXRM_SHIFT 1
#define VCSR_VXRM  (0x3 << VCSR_VXRM_SHIFT)

#define VCSR_VXSAT_SHIFT 0
#define VCSR_VXSAT  (0x1 << VCSR_VXSAT_SHIFT)

#define FP_RD_NE  0
#define FP_RD_0   1
#define FP_RD_DN  2
#define FP_RD_UP  3
#define FP_RD_NMM 4

#define FSR_RD_SHIFT 5
#define FSR_RD   (0x7 << FSR_RD_SHIFT)

#define FSR_BF16_SHIFT  (8)    
#define FSR_BF16   (0x1 << FSR_BF16_SHIFT)

#define FPEXC_NX 0x01
#define FPEXC_UF 0x02
#define FPEXC_OF 0x04
#define FPEXC_DZ 0x08
#define FPEXC_NV 0x10

#define FSR_AEXC_SHIFT 0
#define FSR_NVA  (FPEXC_NV << FSR_AEXC_SHIFT)
#define FSR_OFA  (FPEXC_OF << FSR_AEXC_SHIFT)
#define FSR_UFA  (FPEXC_UF << FSR_AEXC_SHIFT)
#define FSR_DZA  (FPEXC_DZ << FSR_AEXC_SHIFT)
#define FSR_NXA  (FPEXC_NX << FSR_AEXC_SHIFT)
#define FSR_AEXC (FSR_NVA | FSR_OFA | FSR_UFA | FSR_DZA | FSR_NXA)

#define insn_length(x) \
  (((x) & 0x03) < 0x03 ? 2 : \
   ((x) & 0x1f) < 0x1f ? 4 : \
   ((x) & 0x3f) < 0x3f ? 6 : \
   8)
#define MAX_INSN_LENGTH 8
#define PC_ALIGN 2

#define CORE_COUNT 32

typedef uint64_t insn_bits_t;
class insn_t
{
public:
  insn_t() = default;
  insn_t(insn_bits_t bits) : b(bits) {}
  insn_bits_t bits() { return b & ~((UINT64_MAX) << (length() * 8)); }
  int length() { return insn_length(b); }
  int64_t i_imm() { return int64_t(b) >> 20; }
  int64_t ade_imm() { return ((x(14, 1)) + (x(15, 2) <<15) + (x(17, 3) << 12) + (x(20, 1) << 11) + (x(21, 10) << 1) + (imm_sign() << 17)); }
  int64_t ade_lhimm() { return ((x(15, 2) <<15) + (x(17, 3) << 12) + (x(20, 1) << 11) + (x(21, 10) << 1) + (imm_sign() << 17)); }
  int64_t ade_lwimm() { return ((x(15, 2) <<15) + (x(17, 3) << 12) + (x(20, 1) << 11) + (x(21, 1) << 17) + (x(22, 9) << 2) + (imm_sign() << 18)); }
  int64_t ade_ldimm() { return ((x(15, 2) <<15) + (x(17, 3) << 12) + (x(20, 1) << 11) + (x(21, 2) << 17) + (x(23, 8) << 3) + (imm_sign() << 19)); }
  int64_t ade_sbimm() { return ((x(7, 1) << 11) + (x(8, 4) << 1) + (x(14, 1)) + (x(15, 2) << 15) + (x(17, 3) << 12) + (x(25, 6) << 5) + (imm_sign() << 17)); }
  int64_t ade_shimm() { return ((x(7, 1) << 11) + (x(8, 4) << 1) + (x(15, 2) << 15) + (x(17, 3) << 12) + (x(25, 6) << 5) + (imm_sign() << 17)); }
  int64_t ade_swimm() { return ((x(7, 1) << 11) + (x(8, 1) << 17) + (x(9, 3) << 2) + (x(15, 2) << 15) + (x(17, 3) << 12) + (x(25, 6) << 5) + (imm_sign() << 18)); }
  int64_t ade_sdimm() { return ((x(7, 1) << 11) + (x(8, 2) << 17) + (x(10, 2) << 3) + (x(15, 2) << 15) + (x(17, 3) << 12) + (x(25, 6) << 5) + (imm_sign() << 19)); }
  int64_t shamt() { return x(20, 6); }
  int64_t s_imm() { return x(7, 5) + (xs(25, 7) << 5); }
  int64_t sb_imm() { return (x(8, 4) << 1) + (x(25,6) << 5) + (x(7,1) << 11) + (imm_sign() << 12); }
  int64_t ade_bbc_imm() {return ((x(8, 4) << 1) + (x(25, 5) << 5) + (imm_sign() << 10)); }
  int64_t ade_bbc_cimm() {return ((x(7, 1) << 5) + (x(20, 5))); }
  int64_t ade_bec_cimm() {return ((x(7, 1) << 5) + (x(20, 5))) + (x(30, 1) << 6); }
  int64_t ade_bf_msb() {return (x(26, 6)); }
  int64_t ade_bf_lsb() {return (x(20, 6));}
  int64_t u_imm() { return int64_t(b) >> 12 << 12; }
  int64_t uj_imm() { return (x(21, 10) << 1) + (x(20, 1) << 11) + (x(12, 8) << 12) + (imm_sign() << 20); }
  uint64_t v_uimm() { return x(15, 5); }
  uint64_t z_imm() { return x(20, 11); }
  uint64_t rd() { return x(7, 5); }
  uint64_t rs1() { return x(15, 5); }
  uint64_t rs2() { return x(20, 5); }
  uint64_t rs3() { return x(27, 5); }
  uint64_t rm() { return x(12, 3); }
  uint64_t csr() { return x(20, 12); }
  uint64_t dim() { return (x(25, 1) << 1) + x(14, 1); }
  uint64_t ts() { return (x(25, 1) << 1) + x(14, 1); }
  uint64_t iorw() { return x(20, 8); }
  uint64_t op_ve() { return x(0, 7); }

  int64_t rvc_imm() { return x(2, 5) + (xs(12, 1) << 5); }
  int64_t rvc_zimm() { return x(2, 5) + (x(12, 1) << 5); }
  int64_t rvc_addi4spn_imm() { return (x(6, 1) << 2) + (x(5, 1) << 3) + (x(11, 2) << 4) + (x(7, 4) << 6); }
  int64_t rvc_addi16sp_imm() { return (x(6, 1) << 4) + (x(2, 1) << 5) + (x(5, 1) << 6) + (x(3, 2) << 7) + (xs(12, 1) << 9); }
  int64_t rvc_lwsp_imm() { return (x(4, 3) << 2) + (x(12, 1) << 5) + (x(2, 2) << 6); }
  int64_t rvc_ldsp_imm() { return (x(5, 2) << 3) + (x(12, 1) << 5) + (x(2, 3) << 6); }
  int64_t rvc_swsp_imm() { return (x(9, 4) << 2) + (x(7, 2) << 6); }
  int64_t rvc_sdsp_imm() { return (x(10, 3) << 3) + (x(7, 3) << 6); }
  int64_t rvc_lw_imm() { return (x(6, 1) << 2) + (x(10, 3) << 3) + (x(5, 1) << 6); }
  int64_t rvc_ld_imm() { return (x(10, 3) << 3) + (x(5, 2) << 6); }
  int64_t rvc_j_imm() { return (x(3, 3) << 1) + (x(11, 1) << 4) + (x(2, 1) << 5) + (x(7, 1) << 6) + (x(6, 1) << 7) + (x(9, 2) << 8) + (x(8, 1) << 10) + (xs(12, 1) << 11); }
  int64_t rvc_b_imm() { return (x(3, 2) << 1) + (x(10, 2) << 3) + (x(2, 1) << 5) + (x(5, 2) << 6) + (xs(12, 1) << 8); }
  int64_t rvc_simm3() { return x(10, 3); }
  uint64_t rvc_rd() { return rd(); }
  uint64_t rvc_rs1() { return rd(); }
  uint64_t rvc_rs2() { return x(2, 5); }
  uint64_t rvc_rs1s() { return 8 + x(7, 3); }
  uint64_t rvc_rs2s() { return 8 + x(2, 3); }

  uint64_t v_vm() { return x(25, 1); }
  uint64_t v_wd() { return x(26, 1); }
  uint64_t v_nf() { return x(29, 3); }
  uint64_t v_simm5() { return xs(15, 5); }
  uint64_t v_zimm5() { return x(15, 5); }
  uint64_t v_zimm10() { return x(20, 10); }
  uint64_t v_zimm11() { return x(20, 11); }
  uint64_t v_lmul() { return x(20, 2); }
  uint64_t v_frac_lmul() { return x(22, 1); }
  uint64_t v_sew() { return 1 << (x(23, 3) + 3); }
  uint64_t v_width() { return x(12, 3); }
  uint64_t v_mop() { return x(26, 2); }
  uint64_t v_lumop() { return x(20, 5); }
  uint64_t v_sumop() { return x(20, 5); }
  uint64_t v_vta() { return x(26, 1); }
  uint64_t v_vma() { return x(27, 1); }
  uint64_t v_mew() { return x(28, 1); }

private:
  insn_bits_t b;
  uint64_t x(int lo, int len) { return (b >> lo) & ((insn_bits_t(1) << len)-1); }
  uint64_t xs(int lo, int len) { return int64_t(b) << (64-lo-len) >> (64-len); }
  uint64_t imm_sign() { return xs(63, 1); }
};

template <class T, size_t N, bool zero_reg>
class regfile_t
{
public:
  void write(size_t i, T value)
  {
    if (!zero_reg || i != 0)
      data[i] = value;
  }
  const T& operator [] (size_t i) const
  {
    return data[i];
  }
  regfile_t()
  {
    reset();
  }
  void reset()
  {
    memset(data, 0, sizeof(data));
  }
private:
  T data[N];
};

// helpful macros, etc
#define MMU (*p->get_mmu())
#define STATE (*p->get_state())
#define P (*p)
#define FLEN (p->get_flen())

// Seems that 0x0 doesn't work.
#define DEBUG_BASE              (0xc0500000)
#define DEBUG_ROM_BASE          (DEBUG_BASE)
#define IS_EXECUTE_IN_DEBUGROM(pc) ((((DEBUG_ROM_BASE + 0x800) <= (zext32(pc))) \
  && ((DEBUG_ROM_BASE + 0x884) >= (zext32(pc)))) || (((DEBUG_ROM_BASE + 0x360) <= (zext32(pc))) \
  && ((DEBUG_ROM_BASE + 0x374) > (zext32(pc)))))
#define READ_REG(reg) (unlikely(IS_EXECUTE_IN_DEBUGROM(pc) && (reg == 0)) ? DEBUG_ROM_BASE : STATE.XPR[reg])

#define READ_FREG(reg) STATE.FPR[reg]
#define RD READ_REG(insn.rd())
#define RDGP READ_REG(X_GP)

#define RS1 READ_REG(insn.rs1())
#define RS2 READ_REG(insn.rs2())
#define RS3 READ_REG(insn.rs3())
#define WRITE_RD(value) WRITE_REG(insn.rd(), value)
#define WRITE_RD_STC(value) WRITE_REG_STC(insn.rd(), value)
#define WRITE_RS1(value) WRITE_REG(insn.rs1(), value)
#define WRITE_RS2(value) WRITE_REG(insn.rs2(), value)

#define GET_BIT(x, bit) ((x & (1 << bit)) >> bit)
// #define	SET_BIT(x, bit, val)	(x |= (val << bit))

#define __BITS_PER_LONG 64

static inline void set_bit(int nr, unsigned long *addr)
{
	addr[nr / __BITS_PER_LONG] |= 1UL << (nr % __BITS_PER_LONG);
}

static inline void clear_bit(int nr, unsigned long *addr)
{
	addr[nr / __BITS_PER_LONG] &= ~(1UL << (nr % __BITS_PER_LONG));
}

#define GET_RS1_POSI_BIT  GET_BIT(RS1, insn.ade_bbc_cimm())


#ifndef RISCV_ENABLE_COMMITLOG
# define WRITE_REG(reg, value) STATE.XPR.write(reg, value)
# define WRITE_REG_STC(reg, value) STATE.XPR.write(reg, value)
# define WRITE_FREG(reg, value) DO_WRITE_FREG(reg, freg(value))
# define WRITE_FREG_STC(reg, value) DO_WRITE_FREG(reg, freg(value))
# define WRITE_VSTATUS
#else
   /* 0 : int           8 : stc int
    * 1 : floating      9 : stc floating
    * 2 : vector reg    
    * 3 : vector hint
    * 4 : csr
    */
# define WRITE_REG(reg, value) ({ \
    reg_t wdata = (value); /* value may have side effects */ \
    STATE.log_reg_write[(reg) << 4] = {wdata, 0}; \
    STATE.XPR.write(reg, wdata); \
  })
# define WRITE_REG_STC(reg, value) ({ \
    reg_t wdata = (value); /* value may have side effects */ \
    STATE.log_reg_write[((reg) << 4)| 8] = {wdata, 0}; \
    STATE.XPR.write(reg, wdata); \
  })
# define WRITE_FREG(reg, value) ({ \
    freg_t wdata = freg(value); /* value may have side effects */ \
    STATE.log_reg_write[((reg) << 4) | 1] = wdata; \
    DO_WRITE_FREG(reg, wdata); \
  })
# define WRITE_FREG_STC(reg, value) ({ \
    freg_t wdata = freg(value); /* value may have side effects */ \
    STATE.log_reg_write[((reg) << 4) | 9] = wdata; \
    DO_WRITE_FREG(reg, wdata); \
  }) 
  
# define WRITE_VSTATUS STATE.log_reg_write[3] = {0, 0};
#endif

#ifndef RISCV_ENABLE_COMMITLOG  
# define WRITE_MEM_STC(addr, val, type)({})
#else
# define WRITE_MEM_STC(addr, val, type) \
  STATE.log_mem_stc_write.push_back(std::make_tuple(addr, val, type));  
#endif

// RVC macros
#define WRITE_RVC_RS1S(value) WRITE_REG(insn.rvc_rs1s(), value)
#define WRITE_RVC_RS2S(value) WRITE_REG(insn.rvc_rs2s(), value)
#define WRITE_RVC_FRS2S(value) WRITE_FREG(insn.rvc_rs2s(), value)
#define RVC_RS1 READ_REG(insn.rvc_rs1())
#define RVC_RS2 READ_REG(insn.rvc_rs2())
#define RVC_RS1S READ_REG(insn.rvc_rs1s())
#define RVC_RS2S READ_REG(insn.rvc_rs2s())
#define RVC_FRS2 READ_FREG(insn.rvc_rs2())
#define RVC_FRS2S READ_FREG(insn.rvc_rs2s())
#define RVC_SP READ_REG(X_SP)

//rvv macros
#define VILL (P.VU.vill)

// vme macros
#define VME_DTYPE ((STATE.vme_data_type) & 0xFF)
#define VME_DTYPE_VD (STATE.vme_data_type & 0xFF)
#define VME_DTYPE_VS1 ((STATE.vme_data_type & 0xFF00) >> 8)
#define VME_DTYPE_VS2 ((STATE.vme_data_type & 0xFF0000) >> 16)

#define VME_WIN ((STATE.vme_FM_in & 0xFFFF0000) >> 16)
#define VME_HIN (STATE.vme_FM_in & 0xFFFF)

#define VME_IFM_C_STRIDE ((STATE.vme_Cin & 0xFFFF0000) >> 16)
#define VME_CIN (STATE.vme_Cin & 0xFFFF)

#define VME_WOUT ((STATE.vme_FM_out & 0xFFFF0000) >> 16)
#define VME_HOUT (STATE.vme_FM_out & 0xFFFF)

#define VME_OFM_C_STRIDE (STATE.vme_Cout & 0xFFFF)

#define VME_KW ((STATE.vme_kernel_param1 & 0xFF000000) >> 24)
#define VME_KH ((STATE.vme_kernel_param1 & 0xFF0000) >> 16)
#define VME_DILATION_H ((STATE.vme_kernel_param1 & 0xFF00) >> 8)
#define VME_SH (STATE.vme_kernel_param1 & 0xFF)

#define VME_N_PAD_U ((STATE.vme_FM_padding & 0xFF000000) >> 24)
#define VME_N_PAD_D ((STATE.vme_FM_padding & 0xFF0000) >> 16)
#define VME_N_PAD_L ((STATE.vme_FM_padding & 0xFF00) >> 8)
#define VME_N_PAD_R (STATE.vme_FM_padding & 0xFF)

#define VME_DILATION_W ((STATE.vme_kernel_param2 & 0xFF000000) >> 24)
#define VME_SW ((STATE.vme_kernel_param2 & 0xFF0000) >> 16)
#define VME_K_C_STRIDE (STATE.vme_kernel_param2 & 0xFFFF)

#define VME_RELU_THRESHHOLD (STATE.vme_relu_threshhold & 0xFFFFFFFF)

// FPU macros
#define FRS1 READ_FREG(insn.rs1())
#define FRS2 READ_FREG(insn.rs2())
#define FRS3 READ_FREG(insn.rs3())
#define dirty_fp_state (STATE.mstatus |= MSTATUS_FS | (xlen == 64 ? MSTATUS64_SD : MSTATUS32_SD))
#define dirty_ext_state (STATE.mstatus |= MSTATUS_XS | (xlen == 64 ? MSTATUS64_SD : MSTATUS32_SD))
#define dirty_vs_state (STATE.mstatus |= MSTATUS_VS | (xlen == 64 ? MSTATUS64_SD : MSTATUS32_SD))
#define DO_WRITE_FREG(reg, value) (STATE.FPR.write(reg, value), dirty_fp_state)
#define WRITE_FRD(value) WRITE_FREG(insn.rd(), value)
#define WRITE_FRD_STC(value) WRITE_FREG_STC(insn.rd(), value) 

// VPU macros
#define VRS1 READ_VREG(insn.rs1())
#define VRS2 READ_VREG(insn.rs2())
#define VRS3 READ_VREG(insn.rd())
#define VR0  READ_VREG(0)
#define VRD  READ_VREG(insn.rd())
#define FRD  READ_FREG(insn.rd())
#define SEW (8<<((STATE.vtype>>VTYPE_SEW_SHIFT) & VTYPE_VSEW))
#define LMUL (1<<((STATE.vtype>>VTYPE_LMUL_SHIFT) & VTYPE_VLMUL))
#define VL (STATE.vl)
#define VUIMM	(insn.v_uimm())
#define VSTART (STATE.vstart)
#define VLMAX (LMUL*(VLEN/SEW))
#define VLMAX_NO_LMUL	(VLEN/SEW)
#define WRITE_VRD_H(value, idx) WRITE_VREG_H(insn.rd(), idx, value)
#define WRITE_VRD_B(value, idx) WRITE_VREG_B(insn.rd(), idx, value)

#define DIM (insn.dim())
#define TS (insn.ts())
#define DIM_DM (insn.dim()&1)
#define VM (insn.vm())
#define VLMUL_I (insn.vlmul())
#define VSEW_I (insn.vsew())
#define VEDIV_I (insn.vediv())
#define SHAPE1_COLUMN         ((STATE.vme_shape_s & 0xFFFF0000) >> 16)
#define SHAPE1_ROW            (STATE.vme_shape_s & 0xFFFF)
#define STRIDE_RD             (STATE.vme_stride_d & 0xFFFF)
#define STRIDE_RS1            (STATE.vme_stride_s & 0xFFFF)
#define STRIDE_RS2            ((STATE.vme_stride_s & 0xFFFF0000) >> 16)
#define VME_DATA_TYPE         (STATE.vme_data_type)
#define MME_SPARSE_BASE       (STATE.mme_sparseidx_base)
#define MME_SPARSE_STRIDE     (STATE.mme_sparseidx_stride & 0xFFFF)

#define BC_SHAPE1_COLUMN      ((STATE.mme_shape_s1 & 0xFFFF0000) >> 16)
#define BC_SHAPE1_ROW         (STATE.mme_shape_s1 & 0xFFFF)
#define BC_SHAPE2_COLUMN      ((STATE.mme_shape_s2 & 0xFFFF0000) >> 16)
#define BC_SHAPE2_ROW         (STATE.mme_shape_s2 & 0xFFFF)
#define BC_STRIDE_RD          (STATE.mme_stride_d & 0xFFFF)
#define BC_STRIDE_RS1         (STATE.mme_stride_s & 0xFFFF)
#define BC_STRIDE_RS2         ((STATE.mme_stride_s & 0xFFFF0000) >> 16)
#define MME_DATA_TYPE         (STATE.mme_data_type)   

#define MTE_SHAPE_COLUMN      ((STATE.mte_shape & 0xFFFF0000) >> 16)
#define MTE_SHAPE_ROW         (STATE.mte_shape & 0xFFFF)
#define MTE_STRIDE_RS1        (STATE.mte_stride_s & 0xFFFFFF)
#define MTE_STRIDE_RD         (STATE.mte_stride_d & 0xFFFFFF)
#define MTE_DATA_TYPE         (STATE.mte_data_type)
#define MTE_DATA_TYPE_RD      (STATE.mte_data_type & 0xFF)
#define MTE_DATA_TYPE_RS1     ((STATE.mte_data_type & 0xFF00) >> 8)

#define DMAE_DATA_TYPE        (STATE.dmae_data_type & 0xFFFF)
#define DMAE_SHAPE_X          (STATE.dmae_shape_1 & 0xFFFF | STATE.dmae_shape_2 & 0x3F0000)
#define DMAE_SHAPE_Y          ((STATE.dmae_shape_1 & 0xFFFF0000) >> 16)
#define DMAE_SHAPE_Z          (STATE.dmae_shape_2 & 0xFFFF)

#define DMAE_STRIDE_S_X       (STATE.dmae_stride_s1)
#define DMAE_STRIDE_S_Y       (STATE.dmae_stride_s2)
#define DMAE_STRIDE_D_X       (STATE.dmae_stride_d1)
#define DMAE_STRIDE_D_Y       (STATE.dmae_stride_d2)

#define DMAE_CHANNEL          (STATE.dmae_ctrl)

#define SRC_CORE_ID           ((STATE.mte_icdest >> 16) & 0xFF)
#define DST_CORE_ID           (STATE.mte_icdest & 0xFF)

#define DMA_SHAPE_COLUMN      (STATE.dma_shape_col)
#define DMA_SHAPE_ROW         (STATE.dma_shape_row)
#define STRIDE_DDR            (STATE.dma_stride_ddr)

#define CONV_INFM_WH          (STATE.conv_FM_in)
#define CONV_CIN_REG          (STATE.conv_Cin)
#define CONV_OUTFM_WH         (STATE.conv_FM_out)
#define CONV_COUT_REG         (STATE.conv_Cout)
#define CONV_KERNEL_PARAMS1   (STATE.conv_kernel_params1)
#define CONV_KERNEL_PARAMS2   (STATE.conv_kernel_params2)
#define CONV_KERNEL_PARAMS3   (STATE.conv_kernel_params3)
#define CONV_PADDING          (STATE.conv_padding)
#define MME_QUANT_COEFF       (STATE.mme_quant_coeff)
#define MME_DEQUANT_COEFF     (STATE.mme_dequant_coeff)

#define CONV_IN_COLUMN	      ((STATE.conv_FM_in & 0xFFFF0000) >> 16)
#define CONV_IN_ROW	          (STATE.conv_FM_in & 0xFFFF)
#define CONV_OUT_COLUMN	      ((STATE.conv_FM_out & 0xFFFF0000) >> 16)
#define CONV_OUT_ROW	        (STATE.conv_FM_out & 0xFFFF)
#define CONV_CIN	            (STATE.conv_Cin & 0xFFFF)
#define CONV_COUT	            (STATE.conv_Cout & 0xFFFF)
#define CONV_IN_STRIDE	      ((STATE.conv_Cin & 0xFFFF0000) >> 16)
#define CONV_W_STRIDE	        (STATE.conv_kernel_params1 & 0xFF)
#define CONV_OUT_STRIDE	      ((STATE.conv_Cout & 0xFFFF0000) >> 16)
#define CONV_KW 	            ((STATE.conv_kernel_params1 & 0xFF000000) >> 24)
#define CONV_KH 	            ((STATE.conv_kernel_params1 & 0x00FF0000) >> 16)
#define CONV_DH 	            ((STATE.conv_kernel_params1 & 0x0000FF00) >>  8)
#define CONV_SH 	            ((STATE.conv_kernel_params1 & 0x000000FF) >>  0)

#define CONV_DW 	            ((STATE.conv_kernel_params2 & 0xFF000000) >> 24)  
#define CONV_SW 	            ((STATE.conv_kernel_params2 & 0x00FF0000) >> 16)
#define CONV_S_KERNEL 	      ((STATE.conv_kernel_params2 & 0x0000FFFF) >>  0)  

#define CONV_S2_STRIDE        (STATE.conv_kernel_params3 & 0xFFFF)

// commitlog
#define CMT_LOG_VME           (0x0100)  
#define CMT_LOG_VME_COM       (0x0101)
#define CMT_LOG_VME_COM_H     (0x0102)
#define CMT_LOG_VME_COM_W     (0x0104)
#define CMT_LOG_VME_CONV      (0x0108)  //vedwconv, pool
#define CMT_LOG_VME_DATA8     (0x0110)  //invalid vme_data_type
#define CMT_LOG_VME_DATA16    (0x0111) 
#define CMT_LOG_VME_DATA32    (0x0112) 
#define CMT_LOG_MTE           (0x0201)
#define CMT_LOG_DMAE          (0x0401)
#define CMT_LOG_MME           (0x0800)
#define CMT_LOG_MME_METR      (0x0801)
#define CMT_LOG_MME_MEMUL_MM  (0x0802)  //.mm, .sp.mm 
#define CMT_LOG_MME_MEMUL_TS  (0x0804)  //.ts1.mm
#define CMT_LOG_MME_REDUCE    (0x0808)
#define CMT_LOG_MME_CONV      (0x0810)
#define CMT_LOG_MME_DATA16    (0x0812)

#define TRAP_MATRIX           (0x0)
#define TRAP_CONV             (0x1)

//#define TMODE	(STATE.tmode)
//
//#define TCSR_RX_ACTIVE_MASK (0x2)
//#define TCSR_RX_READY_MASK (0x1)
//
//#define TPARA0_TAG_MASK (0xff)
//#define TPARA0_TAG_SHIFT (0)
///*LUT 13-15*/
//#define TPARA0_LUT_MASK (0x7)
//#define TPARA0_LUT_SHIFT (13)
///*Core ID 16-26*/
//#define TPARA0_CORE_MASK (0x7FF)
//#define TPARA0_CORE_SHIFT (16)
///*Chip ID 27-31*/
//#define TPARA0_CHIP_MASK (0x1F)
//#define TPARA0_CHIP_SHIFT (27)
//#define TPARA0	(STATE.tpara0)

#define check_v0hmask(x) \
	if(!VM & !(READ_VREG(0).vh[x] & 0x1)) continue;

#define check_v0bmask(x) \
	if(!VM & !(READ_VREG(0).vb[x] & 0x1)) continue;

#define v0b_mask(x)	(!VM & !(READ_VREG(0).vb[x] & 0x1))
#define v0h_mask(x)	(!VM & !(READ_VREG(0).vh[x] & 0x1))

#define check_vstart if(VSTART >= VL) VSTART = 0; \
					 else

#define check_vl() if(VL == 0) return;

// throw trap if rvv inst access out of l1&im buffer
#define check_rvv_access_without_exception(x, len) \
        (!(p->get_sim()->in_local_mem(zext_xlen(x), L1_BUFFER) && \
              p->get_sim()->in_local_mem(zext_xlen(x) + len-1, L1_BUFFER)) && \
            !(p->get_sim()->in_local_mem(zext_xlen(x), IM_BUFFER) && \
              p->get_sim()->in_local_mem(zext_xlen(x) + len-1, IM_BUFFER)))


//don't modify elment of big than vl
#define vector_for_each(x) for(unsigned int (x) = VSTART; (x) < VL; (x)++)
#define vector_for_each_from_zero(x) for(unsigned int (x) = 0; (x) < VL; (x)++)
#define vector_for_each_no_lmlu(x) for(unsigned int (x) = 0; (x) < VLMAX_NO_LMUL; (x)++)

#define vdh_clear_exceed(idx) if(idx >= VL) {WRITE_VRD_H(0,idx); continue;} \
								else

#define sst_fill(x, esize_in, esize_out) ({(x).shape1_column = SHAPE1_COLUMN; \
					 (x).shape1_row = SHAPE1_ROW; \
					 (x).stride_rd = STRIDE_RD / esize_out; \
					 (x).stride_rs1 = STRIDE_RS1 ? STRIDE_RS1 / esize_in : SHAPE1_COLUMN; \
					 (x).stride_rs2 = STRIDE_RS2 ? STRIDE_RS2 / esize_in : SHAPE1_COLUMN; \
                     (x).relu_threshhold = VME_RELU_THRESHHOLD;\
                     softfloat_roundingMode = STATE.frm;})

#define vme_ss_fill(ss, esize) do { \
    ss.row = VME_HIN; \
    ss.column = VME_WIN; \
    ss.ifm_c_stride = VME_IFM_C_STRIDE ? VME_IFM_C_STRIDE : VME_CIN; \
    ss.cin = VME_CIN; \
    ss.wout = VME_WOUT; \
    ss.hout = VME_HOUT; \
    ss.ofm_c_stride = VME_OFM_C_STRIDE ? VME_OFM_C_STRIDE : VME_CIN; \
    ss.k_c_stride = VME_K_C_STRIDE ? VME_K_C_STRIDE : VME_CIN; \
    ss.kw = VME_KW; \
    ss.kh = VME_KH; \
    ss.sh = VME_SH; \
    ss.sw = VME_SW == 0? ss.sh : VME_SW; \
    ss.n_pad_u = VME_N_PAD_U; \
    ss.n_pad_d = VME_N_PAD_D; \
    ss.n_pad_l = VME_N_PAD_L; \
    ss.n_pad_r = VME_N_PAD_R; \
    ss.k_dilation_h = VME_DILATION_H; \
    ss.k_dilation_w = VME_DILATION_W == 0? ss.k_dilation_h: VME_DILATION_W; \
    ss.relu_threshhold = VME_RELU_THRESHHOLD; \
    softfloat_roundingMode = STATE.frm; \
} while (0);

#define bc_sst_fill(x, esize_in, esize_out) ({ \
           (x).shape1_column = BC_SHAPE1_COLUMN; \
           (x).shape1_row = BC_SHAPE1_ROW; \
           (x).shape2_column = BC_SHAPE2_COLUMN; \
           (x).shape2_row = BC_SHAPE2_ROW; \
           (x).stride_rd = BC_STRIDE_RD; \
           (x).stride_rs1 = BC_STRIDE_RS1 ? BC_STRIDE_RS1: BC_SHAPE1_COLUMN; \
           (x).stride_rs2 = BC_STRIDE_RS2 ? BC_STRIDE_RS2: BC_SHAPE2_COLUMN;\
           softfloat_roundingMode = STATE.frm;})


#define memul_sst_fill(x, esize_src1, esize_src2, esize_out) ({ \
           (x).shape1_column = BC_SHAPE1_COLUMN; \
           (x).shape1_row = BC_SHAPE1_ROW; \
           (x).shape2_column = BC_SHAPE2_COLUMN; \
           (x).shape2_row = BC_SHAPE2_ROW; \
           (x).stride_rd = BC_STRIDE_RD; \
           (x).stride_rs1 = BC_STRIDE_RS1 ? BC_STRIDE_RS1 : BC_SHAPE1_COLUMN; \
           (x).stride_rs2 = BC_STRIDE_RS2 ? BC_STRIDE_RS2 : BC_SHAPE2_COLUMN; \
           (x).stride_idx = MME_SPARSE_STRIDE ? MME_SPARSE_STRIDE : BC_SHAPE2_COLUMN; \
           (x).mme_quant_coeff.v = MME_QUANT_COEFF; \
           (x).mme_dequant_coeff.v = MME_DEQUANT_COEFF;\
           softfloat_roundingMode = STATE.frm;})


#define conv_fill(x) ({(x).conv_fm_in = CONV_INFM_WH; \
           (x).conv_cin = CONV_CIN_REG; \
           (x).conv_fm_out = CONV_OUTFM_WH; \
           (x).conv_cout = CONV_COUT_REG; \
           (x).conv_kernel_params1 = CONV_KERNEL_PARAMS1; \
           (x).conv_kernel_params2 = CONV_KERNEL_PARAMS2; \
           (x).conv_kernel_params3 = CONV_KERNEL_PARAMS3; \
           (x).conv_padding = CONV_PADDING; \
           (x).mme_quant_coeff.v = MME_QUANT_COEFF; \
           (x).mme_dequant_coeff.v = MME_DEQUANT_COEFF; \
           (x).stride_idx = MME_SPARSE_STRIDE;\
           softfloat_roundingMode = STATE.frm;})

#define SHAMT (insn.i_imm() & 0x3F)
#define BRANCH_TARGET (pc + insn.sb_imm())
#define BBCBRANCH_TARGET (pc + insn.ade_bbc_imm())
#define JUMP_TARGET (pc + insn.uj_imm())
#define RM ({ int rm = insn.rm(); \
              if(rm == 7) rm = STATE.frm; \
              if(rm > 4) throw trap_illegal_instruction(insn.bits()); \
              rm; })

#define get_field(reg, mask) (((reg) & (decltype(reg))(mask)) / ((mask) & ~((mask) << 1)))
#define set_field(reg, mask, val) (((reg) & ~(decltype(reg))(mask)) | (((decltype(reg))(val) * ((mask) & ~((mask) << 1))) & (decltype(reg))(mask)))

#define require(x) if (unlikely(!(x))) throw trap_illegal_instruction(insn.bits())
#define require_privilege(p) require(STATE.prv >= (p))
#define require_novirt() if (unlikely(STATE.v)) throw trap_virtual_instruction(insn.bits())
#define require_rv64 require(xlen == 64)
#define require_rv32 require(xlen == 32)
#define require_extension(s) require(p->supports_extension(s))
#define require_impl(s) require(p->supports_impl(s))
#define require_fp require((STATE.mstatus & MSTATUS_FS) != 0)
#define require_accelerator require((STATE.mstatus & MSTATUS_XS) != 0)

#define require_vector_vs require((STATE.mstatus & MSTATUS_VS) != 0);
#define require_vector(alu) \
  do { \
    require_vector_vs; \
    require_extension('V'); \
    require(!P.VU.vill); \
    if (alu && !P.VU.vstart_alu) \
      require(P.VU.vstart == 0); \
    WRITE_VSTATUS; \
    dirty_vs_state; \
  } while (0);
#define require_vector_novtype(is_log, alu) \
  do {  \
    require_vector_vs; \
    require_extension('V'); \
    if (alu && !P.VU.vstart_alu) \
      require(P.VU.vstart == 0); \
    if (is_log) \
      WRITE_VSTATUS; \
    dirty_vs_state; \
  } while (0);
#define require_align(val, pos) require(is_aligned(val, pos))
#define require_noover(astart, asize, bstart, bsize) \
  require(!is_overlapped(astart, asize, bstart, bsize))
#define require_noover_widen(astart, asize, bstart, bsize) \
  require(!is_overlapped_widen(astart, asize, bstart, bsize))
#define require_vm do { if (insn.v_vm() == 0) require(insn.rd() != 0);} while(0);

#define set_fp_exceptions ({ if (softfloat_exceptionFlags) { \
                               dirty_fp_state; \
                               STATE.fflags |= softfloat_exceptionFlags; \
                             } \
                             softfloat_exceptionFlags = 0; })

#define sext32(x) ((sreg_t)(int32_t)(x))
#define zext32(x) ((reg_t)(uint32_t)(x))
#define sext_xlen(x) (((sreg_t)(x) << (64-xlen)) >> (64-xlen))
#define zext_xlen(x) (((reg_t)(x) << (64-xlen)) >> (64-xlen))

#define set_pc(x) \
  do { p->check_pc_alignment(x); \
       npc = sext_xlen(x); \
     } while(0)

#define set_pc_and_serialize(x) \
  do { reg_t __npc = (x) & p->pc_alignment_mask(); \
       npc = PC_SERIALIZE_AFTER; \
       STATE.pc = __npc; \
     } while(0)

class wait_for_interrupt_t {};

#define wfi() \
  do { set_pc_and_serialize(npc); \
       npc = PC_SERIALIZE_WFI; \
       throw wait_for_interrupt_t(); \
     } while(0)

#define serialize() set_pc_and_serialize(npc)

/* Sentinel PC values to serialize simulator pipeline */
#define PC_SERIALIZE_BEFORE 3
#define PC_SERIALIZE_AFTER 5
#define PC_SERIALIZE_WFI 7
#define invalid_pc(pc) ((pc) & 1)

/* Convenience wrappers to simplify softfloat code sequences */
#define isBoxedBF16(r) (isBoxedF32(r) && ((uint64_t)((r.v[0] >> 16) + 1) == ((uint64_t)1 << 48))) 
#define unboxBF16(r) (isBoxedBF16(r) ? (uint16_t)r.v[0] : defaultNaNBF16UI)
#define isBoxedF16(r) (isBoxedF32(r) && ((uint64_t)((r.v[0] >> 16) + 1) == ((uint64_t)1 << 48)))
#define unboxF16(r) (isBoxedF16(r) ? (uint16_t)r.v[0] : defaultNaNF16UI)
#define isBoxedF32(r) (isBoxedF64(r) && ((uint32_t)((r.v[0] >> 32) + 1) == 0))
#define unboxF32(r) (isBoxedF32(r) ? (uint32_t)r.v[0] : defaultNaNF32UI)
#define isBoxedF64(r) ((r.v[1] + 1) == 0)
#define unboxF64(r) (isBoxedF64(r) ? r.v[0] : defaultNaNF64UI)
typedef float128_t freg_t;
inline bfloat16_t bf16(uint16_t v) { return { v }; } 
inline float16_t f16(uint16_t v) { return { v }; }
inline float32_t f32(uint32_t v) { return { v }; }
inline float64_t f64(uint64_t v) { return { v }; }
inline bfloat16_t bf16(freg_t r) { return bf16(unboxBF16(r)); } 
inline float16_t f16(freg_t r) { return f16(unboxF16(r)); }
inline float32_t f32(freg_t r) { return f32(unboxF32(r)); }
inline float64_t f64(freg_t r) { return f64(unboxF64(r)); }
inline float128_t f128(freg_t r) { return r; }
inline freg_t freg(bfloat16_t f) { return { ((uint64_t)-1 << 16) | f.v, (uint64_t)-1 }; }
inline freg_t freg(float16_t f) { return { ((uint64_t)-1 << 16) | f.v, (uint64_t)-1 }; }
inline freg_t freg(float32_t f) { return { ((uint64_t)-1 << 32) | f.v, (uint64_t)-1 }; }
inline freg_t freg(float64_t f) { return { f.v, (uint64_t)-1 }; }
inline freg_t freg(float128_t f) { return f; }
#define BF16_SIGN ((uint16_t)1 << 15)  
#define F16_SIGN ((uint16_t)1 << 15)
#define F32_SIGN ((uint32_t)1 << 31)
#define F64_SIGN ((uint64_t)1 << 63)
#define fsgnj16bf(a, b, n, x) \
  bf16((bf16(a).v & ~BF16_SIGN) | ((((x) ? bf16(a).v : (n) ? BF16_SIGN : 0) ^ bf16(b).v) & BF16_SIGN))
#define fsgnj16(a, b, n, x) \
  f16((f16(a).v & ~F16_SIGN) | ((((x) ? f16(a).v : (n) ? F16_SIGN : 0) ^ f16(b).v) & F16_SIGN))
#define fsgnj32(a, b, n, x) \
  f32((f32(a).v & ~F32_SIGN) | ((((x) ? f32(a).v : (n) ? F32_SIGN : 0) ^ f32(b).v) & F32_SIGN))
#define fsgnj64(a, b, n, x) \
  f64((f64(a).v & ~F64_SIGN) | ((((x) ? f64(a).v : (n) ? F64_SIGN : 0) ^ f64(b).v) & F64_SIGN))

#define isNaNF128(x) isNaNF128UI(x.v[1], x.v[0])
inline float128_t defaultNaNF128()
{
  float128_t nan;
  nan.v[1] = defaultNaNF128UI64;
  nan.v[0] = defaultNaNF128UI0;
  return nan;
}
inline freg_t fsgnj128(freg_t a, freg_t b, bool n, bool x)
{
  a.v[1] = (a.v[1] & ~F64_SIGN) | (((x ? a.v[1] : n ? F64_SIGN : 0) ^ b.v[1]) & F64_SIGN);
  return a;
}
inline freg_t f128_negate(freg_t a)
{
  a.v[1] ^= F64_SIGN;
  return a;
}

#define validate_csr(which, write) ({ \
  if (!STATE.serialized) return PC_SERIALIZE_BEFORE; \
  STATE.serialized = false; \
  unsigned csr_priv = get_field((which), 0x300); \
  unsigned csr_read_only = get_field((which), 0xC00) == 3; \
  if ((((write) && csr_read_only) || STATE.prv < csr_priv) && \
    (which > CSR_USER7 || which < CSR_USER0)) \
    throw trap_illegal_instruction(0); \
  (which); })
  
#define VREG_LENGTH (128)
/*Vector instruction support*/
union vreg_t{
	unsigned char vb[VREG_LENGTH];
	unsigned short vh[VREG_LENGTH / 2];
}; //vector reg length 256bit
//typedef int64_t vreg_t; //vector reg length 256bit

/* For debug only. This will fail if the native machine's float types are not IEEE */
inline float to_f(float32_t f){float r; memcpy(&r, &f, sizeof(r)); return r;}
inline double to_f(float64_t f){double r; memcpy(&r, &f, sizeof(r)); return r;}
inline long double to_f(float128_t f){long double r; memcpy(&r, &f, sizeof(r)); return r;}

// Vector macros
#define e8 8      // 8b elements
#define e16 16    // 16b elements
#define e32 32    // 32b elements
#define e64 64    // 64b elements
#define e128 128  // 128b elements
#define e256 256  // 256b elements
#define e512 512  // 512b elements
#define e1024 1024  // 1024b elements

#define vsext(x, sew) (((sreg_t)(x) << (64-sew)) >> (64-sew))
#define vzext(x, sew) (((reg_t)(x) << (64-sew)) >> (64-sew))

#define DEBUG_RVV 0

#if DEBUG_RVV
#define DEBUG_RVV_FP_VV \
  printf("vfp(%lu) vd=%f vs1=%f vs2=%f\n", i, to_f(vd), to_f(vs1), to_f(vs2));
#define DEBUG_RVV_FP_VF \
  printf("vfp(%lu) vd=%f vs1=%f vs2=%f\n", i, to_f(vd), to_f(rs1), to_f(vs2));
#define DEBUG_RVV_FMA_VV \
  printf("vfma(%lu) vd=%f vs1=%f vs2=%f vd_old=%f\n", i, to_f(vd), to_f(vs1), to_f(vs2), to_f(vd_old));
#define DEBUG_RVV_FMA_VF \
  printf("vfma(%lu) vd=%f vs1=%f vs2=%f vd_old=%f\n", i, to_f(vd), to_f(rs1), to_f(vs2), to_f(vd_old));
#else
#define DEBUG_RVV_FP_VV 0
#define DEBUG_RVV_FP_VF 0
#define DEBUG_RVV_FMA_VV 0
#define DEBUG_RVV_FMA_VF 0
#endif

//
// vector: masking skip helper
//
#define VI_MASK_VARS \
  const int midx = i / 64; \
  const int mpos = i % 64;

#define VI_LOOP_ELEMENT_SKIP(BODY) \
  VI_MASK_VARS \
  if (insn.v_vm() == 0) { \
    BODY; \
    bool skip = ((P.VU.elt<uint64_t>(0, midx) >> mpos) & 0x1) == 0; \
    if (skip) {\
        continue; \
    }\
  }

#define VI_ELEMENT_SKIP(inx) \
  if (inx >= vl) { \
    continue; \
  } else if (inx < P.VU.vstart) { \
    continue; \
  } else { \
    VI_LOOP_ELEMENT_SKIP(); \
  }

//
// vector: operation and register acccess check helper
//
static inline bool is_overlapped(const int astart, int asize,
                                const int bstart, int bsize)
{
  asize = asize == 0 ? 1 : asize;
  bsize = bsize == 0 ? 1 : bsize;

  const int aend = astart + asize;
  const int bend = bstart + bsize;

  return std::max(aend, bend) - std::min(astart, bstart) < asize + bsize;
}

static inline bool is_overlapped_widen(const int astart, int asize,
                                       const int bstart, int bsize)
{
  asize = asize == 0 ? 1 : asize;
  bsize = bsize == 0 ? 1 : bsize;

  const int aend = astart + asize;
  const int bend = bstart + bsize;

  if (astart < bstart &&
      is_overlapped(astart, asize, bstart, bsize) &&
      !is_overlapped(astart, asize, bstart + bsize, bsize)) {
      return false;
  } else  {
    return std::max(aend, bend) - std::min(astart, bstart) < asize + bsize;
  }
}

static inline bool is_aligned(const unsigned val, const unsigned pos)
{
  return pos ? (val & (pos - 1)) == 0 : true;
}

#define VI_NARROW_CHECK_COMMON \
  require_vector(true);\
  require(P.VU.vflmul <= 4); \
  require(P.VU.vsew * 2 <= P.VU.ELEN); \
  require_align(insn.rs2(), P.VU.vflmul * 2); \
  require_align(insn.rd(), P.VU.vflmul); \
  require_vm; \

#define VI_WIDE_CHECK_COMMON \
  require_vector(true);\
  require(P.VU.vflmul <= 4); \
  require(P.VU.vsew * 2 <= P.VU.ELEN); \
  require_align(insn.rd(), P.VU.vflmul * 2); \
  require_vm; \

#define VI_CHECK_ST_INDEX(elt_width) \
  require_vector(false); \
  float vemul = ((float)elt_width / P.VU.vsew * P.VU.vflmul); \
  require(vemul >= 0.125 && vemul <= 8); \
  reg_t emul = vemul < 1 ? 1 : vemul; \
  reg_t flmul = P.VU.vflmul < 1 ? 1 : P.VU.vflmul; \
  require_align(insn.rd(), P.VU.vflmul); \
  require_align(insn.rs2(), vemul); \
  require((nf * flmul) <= (NVPR / 4) && \
          (insn.rd() + nf * flmul) <= NVPR); \

#define VI_CHECK_LD_INDEX(elt_width) \
  VI_CHECK_ST_INDEX(elt_width); \
  for (reg_t idx = 0; idx < nf; ++idx) { \
    reg_t flmul = P.VU.vflmul < 1 ? 1 : P.VU.vflmul; \
    reg_t seg_vd = insn.rd() + flmul * idx;  \
    if (elt_width > P.VU.vsew) { \
      if (seg_vd != insn.rs2()) \
        require_noover(seg_vd, P.VU.vflmul, insn.rs2(), vemul); \
    } else if (elt_width < P.VU.vsew) { \
      if (vemul < 1) {\
        require_noover(seg_vd, P.VU.vflmul, insn.rs2(), vemul); \
      } else {\
        require_noover_widen(seg_vd, P.VU.vflmul, insn.rs2(), vemul); \
      } \
    } \
    if (nf >= 2) { \
      require_noover(seg_vd, P.VU.vflmul, insn.rs2(), vemul); \
    } \
  } \
  require_vm; \

#define VI_CHECK_MSS(is_vs1) \
  if (insn.rd() != insn.rs2()) \
    require_noover(insn.rd(), 1, insn.rs2(), P.VU.vflmul); \
  require_align(insn.rs2(), P.VU.vflmul); \
  if (is_vs1) {\
    if (insn.rd() != insn.rs1()) \
      require_noover(insn.rd(), 1, insn.rs1(), P.VU.vflmul); \
    require_align(insn.rs1(), P.VU.vflmul); \
  } \

#define VI_CHECK_SSS(is_vs1) \
  require_vm; \
  if (P.VU.vflmul > 1) { \
    require_align(insn.rd(), P.VU.vflmul); \
    require_align(insn.rs2(), P.VU.vflmul); \
    if (is_vs1) { \
      require_align(insn.rs1(), P.VU.vflmul); \
    } \
  }

#define VI_CHECK_STORE(elt_width, is_mask_ldst) \
  require_vector(false); \
  reg_t veew = is_mask_ldst ? 1 : sizeof(elt_width##_t) * 8; \
  float vemul = is_mask_ldst ? 1 : ((float)veew / P.VU.vsew * P.VU.vflmul); \
  reg_t emul = vemul < 1 ? 1 : vemul; \
  require(vemul >= 0.125 && vemul <= 8); \
  require_align(insn.rd(), vemul); \
  require((nf * emul) <= (NVPR / 4) && \
          (insn.rd() + nf * emul) <= NVPR); \

#define VI_CHECK_LOAD(elt_width, is_mask_ldst) \
  VI_CHECK_STORE(elt_width, is_mask_ldst); \
  require_vm; \

#define VI_CHECK_DSS(is_vs1) \
  VI_WIDE_CHECK_COMMON; \
  require_align(insn.rs2(), P.VU.vflmul); \
  if (P.VU.vflmul < 1) {\
    require_noover(insn.rd(), P.VU.vflmul * 2, insn.rs2(), P.VU.vflmul); \
  } else {\
    require_noover_widen(insn.rd(), P.VU.vflmul * 2, insn.rs2(), P.VU.vflmul); \
  } \
  if (is_vs1) {\
    require_align(insn.rs1(), P.VU.vflmul); \
    if (P.VU.vflmul < 1) {\
      require_noover(insn.rd(), P.VU.vflmul * 2, insn.rs1(), P.VU.vflmul); \
    } else {\
      require_noover_widen(insn.rd(), P.VU.vflmul * 2, insn.rs1(), P.VU.vflmul); \
    } \
  }

#define VI_CHECK_QSS(is_vs1) \
  require_vector(true);\
  p->supports_extension(EXT_ZVQMAC); \
  require(P.VU.vflmul <= 2); \
  require(P.VU.vsew * 4 <= P.VU.ELEN); \
  require_align(insn.rd(), P.VU.vflmul * 4); \
  require_align(insn.rs2(), P.VU.vflmul); \
  require_vm; \
  if (P.VU.vflmul < 1) {\
    require_noover(insn.rd(), P.VU.vflmul * 4, insn.rs2(), P.VU.vflmul); \
  } else {\
    require_noover_widen(insn.rd(), P.VU.vflmul * 4, insn.rs2(), P.VU.vflmul); \
  } \
  if (is_vs1) {\
     require_align(insn.rs1(), P.VU.vflmul); \
    if (P.VU.vflmul < 1) {\
      require_noover(insn.rd(), P.VU.vflmul * 4, insn.rs1(), P.VU.vflmul); \
    } else {\
      require_noover_widen(insn.rd(), P.VU.vflmul * 4, insn.rs1(), P.VU.vflmul); \
    } \
  }

#define VI_CHECK_DDS(is_rs) \
  VI_WIDE_CHECK_COMMON; \
  require_align(insn.rs2(), P.VU.vflmul * 2); \
  if (is_rs) { \
     require_align(insn.rs1(), P.VU.vflmul); \
    if (P.VU.vflmul < 1) {\
      require_noover(insn.rd(), P.VU.vflmul * 2, insn.rs1(), P.VU.vflmul); \
    } else {\
      require_noover_widen(insn.rd(), P.VU.vflmul * 2, insn.rs1(), P.VU.vflmul); \
    } \
  }

#define VI_CHECK_SDS(is_vs1) \
  VI_NARROW_CHECK_COMMON; \
  if (insn.rd() != insn.rs2()) \
    require_noover(insn.rd(), P.VU.vflmul, insn.rs2(), P.VU.vflmul * 2); \
  if (is_vs1) \
    require_align(insn.rs1(), P.VU.vflmul); \

#define VI_CHECK_REDUCTION(is_wide) \
  require_vector(true);\
  if (is_wide) {\
    require(P.VU.vsew * 2 <= P.VU.ELEN); \
  } \
  require_align(insn.rs2(), P.VU.vflmul); \
  require(P.VU.vstart == 0); \

#define VI_CHECK_SLIDE(is_over) \
  require_align(insn.rs2(), P.VU.vflmul); \
  require_align(insn.rd(), P.VU.vflmul); \
  require_vm; \
  if (is_over) \
    require(insn.rd() != insn.rs2()); \

//throw trp if VL set invalid
#define check_ncp_vill_invalid() \
        if (VILL) \
             throw trap_ncp_vill_invalid_inst();

#define check_tcp_mte_dtype \
  switch (MTE_DATA_TYPE) { \
    case 0x0: \
      break; \
    case 0x101: \
      break; \
    case 0x202: \
      break; \
    case 0x303: \
      break; \
    default: \
        throw trap_tcp_illegal_encoding(); \
  }

// throw trap if cust inst access out of l1&im buffer
#define check_cust_access(x, len) \
        if (!(p->get_sim()->in_local_mem(zext_xlen(x), L1_BUFFER) && \
              p->get_sim()->in_local_mem(zext_xlen(x) + len - 1, L1_BUFFER)) && \
            !(p->get_sim()->in_local_mem(zext_xlen(x), IM_BUFFER) && \
              p->get_sim()->in_local_mem(zext_xlen(x) + len - 1, IM_BUFFER))) { \
            throw trap_ncp_cust_access(false, x, 0, 0); \
        }

// throw trap if cust inst access out of l1 buffer
#define check_cust_access_l1(x, len) \
        if (!(p->get_sim()->in_local_mem(zext_xlen(x), L1_BUFFER) && \
              p->get_sim()->in_local_mem(zext_xlen(x) + len - 1, L1_BUFFER))) { \
            throw trap_ncp_cust_access(false, x, 0, 0); \
        }

// throw trap if cust inst access out of im buffer
#define check_cust_access_im(x, len) \
        if (!(p->get_sim()->in_local_mem(zext_xlen(x), IM_BUFFER) && \
              p->get_sim()->in_local_mem(zext_xlen(x) + len - 1, IM_BUFFER))) { \
            throw trap_ncp_cust_access(false, x, 0, 0); \
        }

// throw trap if cust inst access out of sp_idx buffer
#define check_cust_access_sp(x, len) \
        if (!(p->get_sim()->in_local_mem(zext_xlen(x), SP_BUFFER) && \
              p->get_sim()->in_local_mem(zext_xlen(x) + len - 1, SP_BUFFER))) { \
            throw trap_ncp_cust_access(false, x, 0, 0); \
        }

// throw trap if cust inst access misaligned base address
#define check_cust_misaligned_base(x, type) \
        if (unlikely(x & (sizeof(type) - 1))) { \
            throw trap_ncp_cust_misaligned_base(false, x, 0, 0); \
        }

// throw trap if cust inst source address access with misaligned stride
#define check_cust_misaligned_stride_src(x, type, stride) \
        if (unlikely(stride && (stride & (sizeof(type)-1)))) { \
            throw trap_ncp_cust_misaligned_stride(false, x, 0, 0); \
        }

// throw trap if cust inst dest access with misaligned stride, or stride < width
#define check_cust_misaligned_stride_dst(x, type, stride, col) \
        if (unlikely(stride && (stride & (sizeof(type)-1) || stride < col*sizeof(type)))) { \
            throw trap_ncp_cust_misaligned_stride(false, x, 0, 0); \
        }

// throw trap if cust inst use invalid shape, col=0 or row=0
#define check_cust_invalid_shape(col, row) \
        if (unlikely(col == 0 || row == 0)) { \
            throw trap_ncp_cust_invalid_param(); \
        }   

// throw trap if cust mme inst use invalid npu_v2 data_type
#define check_cust_invalid_npu_data_type(type) \
        if (!(unlikely( type==0x0 || type==0x10101 || type==0x20202 || type==0x30303 || type==0x3030b || type==0x3030c ||\
          type==0x3040b || type==0x3040c || type==0x3030f || type==0x30310 || type==0x3040f || type==0x30410 ||\
          type==0x3090b || type==0x30a0b || type==0x3090c || type==0x30a0c || type==0x30d0f || type==0x30e0f ||\
          type==0x30d10 || type==0x30e10 || type==0x1     || type==0x2     || type==0x10102))) {\
            throw trap_ncp_vill_invalid_inst(); \
        } 

// throw trap if cust mme inst use invalid data_type(vme or mme reduce)
#define check_cust_invalid_vme_or_reduce_data_type(type) \
        if (unlikely(      type==0x30303 || type==0x3030b || type==0x3030c ||\
          type==0x3040b || type==0x3040c || type==0x3030f || type==0x30310 || type==0x3040f || type==0x30410 ||\
          type==0x3090b || type==0x30a0b || type==0x3090c || type==0x30a0c || type==0x30d0f || type==0x30e0f ||\
          type==0x30d10 || type==0x30e10 || type==0x10102)) {\
            throw trap_ncp_cust_invalid_param(); \
        } 

// throw trap if cust mme inst use invalid mme data_type(transpose)
#define check_cust_invalid_mme_tran_data_type(type) \
        if (unlikely(      type==0x3030b || type==0x3030c || \
          type==0x3040b || type==0x3040c || type==0x3030f || type==0x30310 || type==0x3040f || type==0x30410 ||\
          type==0x3090b || type==0x30a0b || type==0x3090c || type==0x30a0c || type==0x30d0f || type==0x30e0f ||\
          type==0x30d10 || type==0x30e10 || type==0x10102   )) {\
            throw trap_ncp_cust_invalid_param(); \
        } 

// throw trap if cust inst use invalid mme data_type(matrix and conv)
#define check_cust_invalid_mme_matrix_conv_data_type(type) \
        if (unlikely(type==0x30303)) {\
            throw trap_ncp_cust_invalid_param(); \
        }    

// throw trap if cust inst use invalid params-misaligned-4 (matrix and conv)
#define check_cust_invalid_params_misaligned_4(conv) \
        int paras = conv ? CONV_CIN : BC_SHAPE1_COLUMN;\
        if (unlikely(paras%4 != 0)) { \
            throw trap_ncp_cust_invalid_param(); \
        }

// throw trap if cust inst use misaligned base(int8 mv, half world)
#define check_traps_misaligned_int8_mv() \
        if (unlikely(MME_DEQUANT_COEFF & 0x1)) { \
            throw trap_ncp_cust_misaligned_base(false, MME_DEQUANT_COEFF, 0, 0); \
        } \

// throw trap if cust inst use invalid shape, col=0 or row=0
#define check_sp_invalid_shape(col, row) \
        if (unlikely(col == 0 || row == 0 || !!(row%4))) { \
            throw trap_ncp_cust_invalid_param(); \
        }

//  throw trap if conv inst use invalid param
#define check_cust_invalid_conv_param(fm_in, depth_in, kernel) ({\
        int in_w = (fm_in >> 16) & 0xffff; \
        int in_h = (fm_in) & 0xffff; \
        int in_c = (depth_in) & 0xffff; \
        if (unlikely(in_w == 0 || in_h == 0 || inc_c == 0)) { \
            throw trap_ncp_cust_invalid_param(); \
        } \
        int dilation = (kernel >> 8) & 0xff; \
        int kw = (ss->conv_kernel_params2 >> 24) & 0xff; \
        int kh = (ss->conv_kernel_params2 >> 16) & 0xff; \
        int sk = (kernel) & 0xff; \
        if (unlikely(dilation == 0 || kw == 0 || kh == 0 || sk == 0)) { \
            throw trap_ncp_cust_invalid_param(); \
        } \
        int conv2_s2_stride = ss->conv_kernel_params2 & 0xffff; \
        conv2_s2_stride = conv2_s2_stride == 0? in_c : conv2_s2_stride; \
        if (unlikely(conv2_s2_stride < in_c)) { \
            throw trap_ncp_cust_invalid_param(); \
        } \
  })

#define check_cust_invalid_pool_kernel_param(kh, kw, sh, sw) ({ \
        if (unlikely(kh == 0 || kw ==0 || sh == 0 || sw == 0)) { \
            throw trap_ncp_cust_invalid_param(); \
        } \
})

#define check_cust_invalid_pool_shape(hin, win, cin, hout, wout) ({ \
    if (unlikely(hin == 0 || win ==0 || cin == 0 || hout ==0 || wout == 0)) { \
        throw trap_ncp_cust_invalid_param(); \
    } \
})

#define check_traps_vepool_m(dtype) ({ \
        check_cust_misaligned_base(RS1, dtype); \
        check_cust_misaligned_base(RD, dtype); \
        check_cust_invalid_pool_kernel_param(VME_KH, VME_KW, VME_SH, VME_SW); \
        check_cust_invalid_pool_shape(VME_HIN, VME_WIN, VME_CIN, VME_HOUT, VME_WOUT); \
        int rs1_size = VME_IFM_C_STRIDE ? \
                      (VME_IFM_C_STRIDE * (VME_HIN * VME_WIN - 1) * sizeof(dtype) + VME_CIN * sizeof(dtype)) : \
                      (VME_HIN * VME_WIN * VME_CIN * sizeof(dtype)); \
        int rd_size = VME_OFM_C_STRIDE ? \
                      (VME_OFM_C_STRIDE * (VME_HOUT * VME_WOUT - 1) * sizeof(dtype) + VME_CIN * sizeof(dtype)) : \
                      (VME_HOUT * VME_WOUT * VME_CIN * sizeof(dtype)); \
        check_cust_access(RS1, rs1_size); \
        check_cust_access(RD, rd_size); \
})

#define check_traps_veconv_m(dtype) ({ \
        check_cust_misaligned_base(RS1, dtype); \
        check_cust_misaligned_base(RS2, dtype); \
        check_cust_misaligned_base(RD, dtype); \
        check_cust_invalid_pool_kernel_param(VME_KH, VME_KW, VME_SH, VME_SW); \
        check_cust_invalid_pool_shape(VME_HIN, VME_WIN, VME_CIN, VME_HOUT, VME_WOUT); \
        if (unlikely(VME_DILATION_H == 0)) { \
            throw trap_ncp_cust_invalid_param(); \
        } \
        int rs1_size = VME_IFM_C_STRIDE ? \
                       (VME_IFM_C_STRIDE * (VME_HIN * VME_WIN - 1) * sizeof(dtype) + VME_CIN * sizeof(dtype)) : \
                       (VME_HIN * VME_WIN * VME_CIN * sizeof(dtype)); \
        int rs2_size = VME_K_C_STRIDE ? \
                       (VME_K_C_STRIDE * (VME_KH * VME_KW - 1) * sizeof(dtype) + VME_CIN * sizeof(dtype)) : \
                       (VME_KH * VME_KW * VME_CIN * sizeof(dtype)); \
        int rd_size = VME_OFM_C_STRIDE ? \
                      (VME_OFM_C_STRIDE * (VME_HOUT * VME_WOUT - 1) * sizeof(dtype) + VME_CIN * sizeof(dtype)) : \
                      (VME_HOUT * VME_WOUT * VME_CIN * sizeof(dtype)); \
        check_cust_access(RS1, rs1_size); \
        check_cust_access(RS2, rs2_size); \
        check_cust_access(RD, rd_size); \
})

// check traps for ve***.mm instructions
#define check_traps_vexxx_mm(etype) ({ \
        check_cust_misaligned_base(RS1, etype); \
        check_cust_misaligned_base(RS2, etype); \
        check_cust_misaligned_base(RD, etype); \
        check_cust_invalid_shape(SHAPE1_COLUMN, SHAPE1_ROW); \
        int rs1_size = STRIDE_RS1 ? \
                       (STRIDE_RS1 * (SHAPE1_ROW -1) + SHAPE1_COLUMN) * sizeof(etype) : \
                       (SHAPE1_COLUMN) * sizeof(etype) * SHAPE1_ROW; \
        int rs2_size = STRIDE_RS2 ? \
                       (STRIDE_RS2 * (SHAPE1_ROW -1) + SHAPE1_COLUMN) * sizeof(etype) : \
                       (SHAPE1_COLUMN) * sizeof(etype) * SHAPE1_ROW; \
        int rd_size = STRIDE_RD ? \
                      (STRIDE_RD * (SHAPE1_ROW -1) + SHAPE1_COLUMN) * sizeof(etype) : \
                      (SHAPE1_COLUMN) * sizeof(etype) * SHAPE1_ROW; \
        check_cust_access(RS1, rs1_size); \
        check_cust_access(RS2, rs2_size); \
        check_cust_access(RD, rd_size); \
    })

// check traps for ve***.mm reduce with float result instructions
#define check_traps_vexxx_mm_reduce_all(etype) ({ \
        check_cust_misaligned_base(RS1, etype); \
        check_cust_misaligned_base(RS2, etype); \
        check_cust_invalid_shape(SHAPE1_COLUMN, SHAPE1_ROW); \
        int rs1_size = STRIDE_RS1 ? \
                       (STRIDE_RS1 * (SHAPE1_ROW -1) + SHAPE1_COLUMN) * sizeof(etype) : \
                       (SHAPE1_COLUMN) * sizeof(etype) * SHAPE1_ROW; \
        int rs2_size = STRIDE_RS2 ? \
                       (STRIDE_RS2 * (SHAPE1_ROW -1) + SHAPE1_COLUMN) * sizeof(etype) : \
                       (SHAPE1_COLUMN) * sizeof(etype) * SHAPE1_ROW; \
        check_cust_access(RS1, rs1_size); \
        check_cust_access(RS2, rs2_size); \
    })

// check traps for ve***.mm reduce with vector result instructions
#define check_traps_vexxx_mm_reduce_vector(etype) ({ \
        check_cust_misaligned_base(RS1, etype); \
        check_cust_misaligned_base(RS2, etype); \
        check_cust_misaligned_base(RD, etype); \
        check_cust_invalid_shape(SHAPE1_COLUMN, SHAPE1_ROW); \
        int rs1_size = STRIDE_RS1 ? \
                       (STRIDE_RS1 * (SHAPE1_ROW -1) + SHAPE1_COLUMN) * sizeof(etype) : \
                       (SHAPE1_COLUMN) * sizeof(etype) * SHAPE1_ROW; \
        int rs2_size = STRIDE_RS2 ? \
                       (STRIDE_RS2 * (SHAPE1_ROW -1) + SHAPE1_COLUMN) * sizeof(etype) : \
                       (SHAPE1_COLUMN) * sizeof(etype) * SHAPE1_ROW; \
        check_cust_access(RS1, rs1_size); \
        check_cust_access(RS2, rs2_size); \
        if (DIM_DM == 0) { \
            check_cust_access(RD, SHAPE1_COLUMN * sizeof(etype)); \
        } else { \
            check_cust_access(RD, SHAPE1_ROW * sizeof(etype)); \
        } \
    })

// check traps for ve***.mv reduce with vector result instructions
#define check_traps_vexxx_mv_reduce_vector(etype) ({ \
        check_cust_misaligned_base(RS1, etype); \
        check_cust_misaligned_base(RS2, etype); \
        check_cust_misaligned_base(RD, etype); \
        check_cust_invalid_shape(SHAPE1_COLUMN, SHAPE1_ROW); \
        int rs1_size = STRIDE_RS1 ? \
                       (STRIDE_RS1 * (SHAPE1_ROW -1) + SHAPE1_COLUMN) * sizeof(etype) : \
                       (SHAPE1_COLUMN) * sizeof(etype) * SHAPE1_ROW; \
        check_cust_access(RS1, rs1_size); \
        if (DIM_DM == 0) { \
            check_cust_access(RS2, SHAPE1_ROW * sizeof(etype)); \
            check_cust_access(RD, SHAPE1_COLUMN * sizeof(etype)); \
        } else { \
            check_cust_access(RS2, SHAPE1_COLUMN * sizeof(etype)); \
            check_cust_access(RD, SHAPE1_ROW * sizeof(etype)); \
        } \
    })

// check traps for ve***.mf reduce with vector result instructions
#define check_traps_vexxx_mf_reduce_vector(etype) ({ \
        check_cust_misaligned_base(RS1, etype); \
        check_cust_misaligned_base(RD, etype); \
        check_cust_invalid_shape(SHAPE1_COLUMN, SHAPE1_ROW); \
        int rs1_size = STRIDE_RS1 ? \
                       (STRIDE_RS1 * (SHAPE1_ROW -1) + SHAPE1_COLUMN) * sizeof(etype) : \
                       (SHAPE1_COLUMN) * sizeof(etype) * SHAPE1_ROW; \
        check_cust_access(RS1, rs1_size); \
        if (DIM_DM == 0) { \
            check_cust_access(RD, SHAPE1_COLUMN * sizeof(etype)); \
        } else { \
            check_cust_access(RD, SHAPE1_ROW * sizeof(etype)); \
        } \
  })

// check traps for ve***.mv instructions
#define check_traps_vexxx_mv(etype) ({ \
        check_cust_misaligned_base(RS1, etype); \
        check_cust_misaligned_base(RS2, etype); \
        check_cust_misaligned_base(RD, etype); \
        check_cust_invalid_shape(SHAPE1_COLUMN, SHAPE1_ROW); \
        int rs1_size = STRIDE_RS1 ? \
                       (STRIDE_RS1 * (SHAPE1_ROW -1) + SHAPE1_COLUMN) * sizeof(etype) : \
                       (SHAPE1_COLUMN) * sizeof(etype) * SHAPE1_ROW; \
        int rd_size = STRIDE_RD ? \
                      (STRIDE_RD * (SHAPE1_ROW -1) + SHAPE1_COLUMN) * sizeof(etype) : \
                      (SHAPE1_COLUMN) * sizeof(etype) * SHAPE1_ROW; \
        check_cust_access(RS1, rs1_size); \
        if (DIM_DM == 0) { \
          check_cust_access(RS2, SHAPE1_COLUMN * sizeof(etype)); \
        } else { \
          check_cust_access(RS2, SHAPE1_ROW * sizeof(etype)); \
        } \
        check_cust_access(RD, rd_size); \
  })

// check traps for ve***.mv instructions, x32 matrix
#define check_traps_vexxx_mv_x32 ({ \
        check_cust_misaligned_base(RS1, int32_t); \
        check_cust_misaligned_base(RS2, int16_t); \
        check_cust_misaligned_base(RD, int16_t); \
        check_cust_invalid_shape(SHAPE1_COLUMN, SHAPE1_ROW); \
        int rs1_size = STRIDE_RS1 ? \
                       (STRIDE_RS1 * (SHAPE1_ROW - 1) + SHAPE1_COLUMN) * sizeof(int32_t) : \
                       (SHAPE1_COLUMN * sizeof(int32_t)) * SHAPE1_ROW; \
        int rd_size = STRIDE_RD ? \
                      (STRIDE_RD * (SHAPE1_ROW - 1) + SHAPE1_COLUMN) * sizeof(int16_t) : \
                      (SHAPE1_COLUMN * sizeof(int16_t)) * SHAPE1_ROW; \
        check_cust_access(RS1, rs1_size); \
        if (DIM_DM == 0) { \
          check_cust_access(RS2, SHAPE1_COLUMN * sizeof(int16_t)); \
        } else { \
          check_cust_access(RS2, SHAPE1_ROW * sizeof(int16_t)); \
        } \
        check_cust_access(RD, rd_size); \
  })

// check traps for ve***.mf instructions
#define check_traps_vexxx_mf(etype) ({ \
        check_cust_misaligned_base(RS1, etype); \
        check_cust_misaligned_base(RD, etype); \
        check_cust_invalid_shape(SHAPE1_COLUMN, SHAPE1_ROW); \
        int rs1_size = STRIDE_RS1 ? \
                       (STRIDE_RS1 * (SHAPE1_ROW -1) + SHAPE1_COLUMN) * sizeof(etype) : \
                       (SHAPE1_COLUMN) * sizeof(etype) * SHAPE1_ROW; \
        int rd_size = STRIDE_RD ? \
                      (STRIDE_RD * (SHAPE1_ROW -1) + SHAPE1_COLUMN) * sizeof(etype) : \
                      (SHAPE1_COLUMN) * sizeof(etype) * SHAPE1_ROW; \
        check_cust_access(RS1, rs1_size); \
        check_cust_access(RD, rd_size); \
  })

// check traps for ve***.mf instructions, x32 matrix
#define check_traps_vexxx_mf_x32 ({ \
        check_cust_misaligned_base(RS1, int32_t); \
        check_cust_misaligned_base(RD, int16_t); \
        check_cust_invalid_shape(SHAPE1_COLUMN, SHAPE1_ROW); \
        int rs1_size = STRIDE_RS1 ? \
                       (STRIDE_RS1 * (SHAPE1_ROW -1) + SHAPE1_COLUMN) * sizeof(int32_t) : \
                       (SHAPE1_COLUMN * sizeof(int32_t)) * SHAPE1_ROW; \
        int rd_size = STRIDE_RD ? \
                      (STRIDE_RD * (SHAPE1_ROW -1) + SHAPE1_COLUMN) * sizeof(int16_t) : \
                      (SHAPE1_COLUMN * sizeof(int16_t)) * SHAPE1_ROW; \
        check_cust_access(RS1, rs1_size); \
        check_cust_access(RD, rd_size); \
  })

// check traps for ve***.mf instructions, .x8.hf.mf
#define check_traps_vexxx_mf_x8out ({ \
        check_cust_misaligned_base(RS1, int16_t); \
        check_cust_invalid_shape(SHAPE1_COLUMN, SHAPE1_ROW); \
        int rs1_size = STRIDE_RS1 ? \
                       (STRIDE_RS1 * (SHAPE1_ROW -1) + SHAPE1_COLUMN) * sizeof(int16_t) : \
                       (SHAPE1_COLUMN * sizeof(int16_t)) * SHAPE1_ROW; \
        int rd_size = STRIDE_RD ? \
                      (STRIDE_RD * (SHAPE1_ROW -1) + SHAPE1_COLUMN) * sizeof(int8_t) : \
                      (SHAPE1_COLUMN * sizeof(int8_t)) * SHAPE1_ROW; \
        check_cust_access(RS1, rs1_size); \
        check_cust_access(RD, rd_size); \
  })

// #define check_vme_data_type \
//   if ( (VME_DATA_TYPE != 0x0 ) && (VME_DATA_TYPE != 0x010101 ) && (VME_DATA_TYPE != 0x020202 ) ) \
//     throw trap_ncp_cust_invalid_param();    

#define check_vme_stride_d( width, stride_d ) \
  if ( ( stride_d != 0 ) && ( stride_d < width ) ) \
    throw trap_ncp_cust_invalid_param(); 

// check traps for ve***.m instructions, element-wise
#define check_traps_vexxx_m_element_wise(etype) ({ \
        check_cust_misaligned_base(RS1, etype); \
        check_cust_misaligned_base(RD, etype); \
        check_cust_invalid_shape(SHAPE1_COLUMN, SHAPE1_ROW); \
        int esize = sizeof(etype); \
        int rs1_size = STRIDE_RS1 ? \
                       (STRIDE_RS1 * (SHAPE1_ROW -1) + SHAPE1_COLUMN) * esize : \
                       (SHAPE1_COLUMN * esize) * SHAPE1_ROW; \
        int rd_size = STRIDE_RD ? \
                      (STRIDE_RD * (SHAPE1_ROW -1) + SHAPE1_COLUMN) * esize : \
                      (SHAPE1_COLUMN * esize) * SHAPE1_ROW; \
        check_cust_access(RS1, rs1_size); \
        check_cust_access(RD, rd_size); \
        check_vme_stride_d(SHAPE1_COLUMN, STRIDE_RD); \
  })

// check traps for verand.v instructions
#define check_traps_verand_v(etype) ({ \
        check_cust_misaligned_base(RS1, etype); \
        check_cust_misaligned_base(RD, etype); \
        check_cust_invalid_shape(SHAPE1_COLUMN, SHAPE1_ROW); \
        int esize = sizeof(etype); \
        int rs1_size = 256; \
        int rd_size = STRIDE_RD ? (STRIDE_RD * (SHAPE1_ROW -1) * esize + SHAPE1_COLUMN * esize) : (SHAPE1_COLUMN * esize) * SHAPE1_ROW; \
        check_cust_access(RS1, rs1_size); \
        check_cust_access(RD, rd_size); \
  })

// check traps for verand.m instructions
#define check_traps_verand_m(etype) ({ \
        check_cust_misaligned_base(RD, etype); \
        check_cust_invalid_shape(SHAPE1_COLUMN, SHAPE1_ROW); \
        int esize = sizeof(etype); \
        int rd_size = STRIDE_RD ? (STRIDE_RD * (SHAPE1_ROW -1) * esize + SHAPE1_COLUMN * esize) : (SHAPE1_COLUMN * esize) * SHAPE1_ROW; \
        check_cust_access(RD, rd_size); \
  })

// check traps for ve***.m instructions, reduce all
#define check_traps_vexxx_m_reduce_all(etype) ({ \
        check_cust_misaligned_base(RS1, etype); \
        check_cust_invalid_shape(SHAPE1_COLUMN, SHAPE1_ROW); \
        int rs1_size = STRIDE_RS1 ? \
                       (STRIDE_RS1 * (SHAPE1_ROW -1) + SHAPE1_COLUMN) * sizeof(etype) : \
                       (SHAPE1_COLUMN) * sizeof(etype) * SHAPE1_ROW; \
        check_cust_access(RS1, rs1_size); \
  })

// check traps for ve***.m instructions, reduce to vector
#define check_traps_vexxx_m_reduce_vector(etype) ({ \
        check_cust_misaligned_base(RS1, etype); \
        check_cust_misaligned_base(RD, etype); \
        check_cust_invalid_shape(SHAPE1_COLUMN, SHAPE1_ROW); \
        int rs1_size = STRIDE_RS1 ? \
                       (STRIDE_RS1 * (SHAPE1_ROW -1) + SHAPE1_COLUMN) * sizeof(int16_t) : \
                       (SHAPE1_COLUMN) * sizeof(etype) * SHAPE1_ROW; \
        check_cust_access(RS1, rs1_size); \
        if (DIM_DM == 0) { \
          check_cust_access(RD, SHAPE1_COLUMN * sizeof(etype)); \
        } else { \
          check_cust_access(RD, SHAPE1_ROW * sizeof(etype)); \
        } \
  })


// check traps for ve***.m instructions, element-wise
#define check_traps_vexxx_m_convert(dtype, stype) ({ \
        check_cust_misaligned_base(RS1, stype); \
        check_cust_misaligned_base(RD, dtype); \
        check_cust_invalid_shape(SHAPE1_COLUMN, SHAPE1_ROW); \
        int rs1_size = STRIDE_RS1 ? \
                       (STRIDE_RS1 * (SHAPE1_ROW -1) + SHAPE1_COLUMN) * sizeof(stype) : \
                       SHAPE1_COLUMN * sizeof(stype) * SHAPE1_ROW; \
        int rd_size = STRIDE_RD ? \
                      (STRIDE_RD * (SHAPE1_ROW -1) + SHAPE1_COLUMN) * sizeof(dtype) : \
                      SHAPE1_COLUMN * sizeof(dtype) * SHAPE1_ROW; \
        check_cust_access(RS1, rs1_size); \
        check_cust_access(RD, rd_size); \
  })

// check traps for velut.m instructions
#define check_traps_velut_m ({ \
        check_cust_misaligned_base(RS1, int16_t); \
        check_cust_misaligned_base(RS2, int16_t); \
        check_cust_misaligned_base(RD, int16_t); \
        check_cust_invalid_shape(SHAPE1_COLUMN, SHAPE1_ROW); \
        int rs1_size = STRIDE_RS1 ? \
                       (STRIDE_RS1 * (SHAPE1_ROW -1) + SHAPE1_COLUMN) * sizeof(int16_t) : \
                       (SHAPE1_COLUMN * sizeof(int16_t)) * SHAPE1_ROW; \
        int rd_size = STRIDE_RD ? \
                      (STRIDE_RD * (SHAPE1_ROW -1) + SHAPE1_COLUMN) * sizeof(int16_t) : \
                      (SHAPE1_COLUMN * sizeof(int16_t)) * SHAPE1_ROW; \
        check_cust_access(RS1, rs1_size); \
        check_cust_access(RS2, 0x10000); \
        check_cust_access(RD, rd_size); \
  })

// check traps for mov.f instructions
#define check_traps_mov_f(esize) ({ \
        check_cust_misaligned_base(RD, int16_t); \
        check_cust_invalid_shape(SHAPE1_COLUMN, SHAPE1_ROW); \
        check_tcp_data_type \
        int rd_size = STRIDE_RD ? \
                      (STRIDE_RD * (SHAPE1_ROW -1) + SHAPE1_COLUMN) * esize : \
                      (SHAPE1_COLUMN * esize) * SHAPE1_ROW; \
        check_cust_access(RD, rd_size); \
  })

// check traps for mov.v instructions
#define check_traps_mov_v(esize) ({ \
        check_cust_misaligned_base(RS1, int16_t); \
        check_cust_misaligned_base(RD, int16_t); \
        check_cust_invalid_shape(SHAPE1_COLUMN, SHAPE1_ROW); \
        check_tcp_data_type \
        int rd_size = STRIDE_RD ? \
                      (STRIDE_RD * (SHAPE1_ROW -1) + SHAPE1_COLUMN) * esize : \
                      (SHAPE1_COLUMN * esize) * SHAPE1_ROW; \
        if (DIM_DM == 0) { \
          check_cust_access(RS1, SHAPE1_COLUMN * esize); \
        } else { \
          check_cust_access(RS1, SHAPE1_ROW * esize); \
        } \
        check_cust_access(RD, rd_size); \
  })

// check traps for memul.mm instructions (BC_SHAPE2_ROW = 0, no traps)
#define check_traps_memul_mm(rs1_type, rs2_type, out_type) ({ \
        check_cust_misaligned_base(RS1, rs1_type); \
        check_cust_misaligned_base(RS2, rs2_type); \
        check_cust_misaligned_base(RD, out_type); \
        check_cust_invalid_shape(BC_SHAPE1_ROW, BC_SHAPE1_COLUMN); \
        check_cust_invalid_shape(4, BC_SHAPE2_COLUMN); \
        int rs1_size = BC_STRIDE_RS1 ? \
                       (BC_STRIDE_RS1 * (BC_SHAPE1_ROW - 1) + BC_SHAPE1_COLUMN) * sizeof(rs1_type) : \
                       (BC_SHAPE1_COLUMN * sizeof(rs1_type)) * BC_SHAPE1_ROW; \
        int rs2_size = BC_STRIDE_RS2 ? \
                       (BC_STRIDE_RS2 * (BC_SHAPE2_ROW - 1) + BC_SHAPE2_COLUMN) * sizeof(rs2_type) : \
                       (BC_SHAPE2_COLUMN * sizeof(rs2_type)) * BC_SHAPE2_ROW; \
        int rd_size = BC_STRIDE_RD ? \
                      (BC_STRIDE_RD * (BC_SHAPE1_ROW - 1) + BC_SHAPE2_COLUMN) * sizeof(out_type) : \
                      BC_SHAPE2_COLUMN * sizeof(out_type) * BC_SHAPE1_ROW; \
        check_cust_access(RS1, rs1_size); \
        check_cust_access(RS2, rs2_size); \
        check_cust_access_im(RD, rd_size); \
  })
// check traps for memul.tsx.mm instructions
#define check_traps_memul_ts_mm(rs1_type, rs2_type, out_type) ({ \
        check_cust_misaligned_base(RS1, rs1_type); \
        check_cust_misaligned_base(RS2, rs2_type); \
        check_cust_misaligned_base(RD, out_type); \
        check_cust_invalid_shape(BC_SHAPE1_ROW, BC_SHAPE1_COLUMN * sizeof(rs1_type)); \
        check_cust_invalid_shape(4, BC_SHAPE2_COLUMN * sizeof(rs2_type)); \
        int rs1_size = BC_STRIDE_RS1 ? \
                       (BC_STRIDE_RS1 * (BC_SHAPE1_ROW - 1) + BC_SHAPE1_COLUMN) * sizeof(rs1_type) : \
                       (BC_SHAPE1_COLUMN * sizeof(rs1_type)) * BC_SHAPE1_ROW; \
        int rs2_size = BC_STRIDE_RS2 ? \
                       (BC_STRIDE_RS2 * (BC_SHAPE2_ROW - 1) + BC_SHAPE2_COLUMN) * sizeof(rs2_type) : \
                       (BC_SHAPE2_COLUMN * sizeof(rs2_type)) * BC_SHAPE2_ROW; \
        int rd_size = BC_STRIDE_RD ? \
                      (BC_STRIDE_RD * (BC_SHAPE1_COLUMN - 1) + BC_SHAPE2_COLUMN) * sizeof(out_type) : \
                      (BC_SHAPE2_COLUMN * sizeof(out_type)) * BC_SHAPE1_COLUMN; \
        check_cust_access(RS1, rs1_size); \
        check_cust_access(RS2, rs2_size); \
        check_cust_access_im(RD, rd_size); \
  })
  
// check traps for memul.sp.mm instructions
#define check_traps_memul_sp_mm(rs1_type, rs2_type, out_type) ({ \
        check_cust_misaligned_base(RS1, rs1_type); \
        check_cust_misaligned_base(RS2, rs2_type); \
        check_cust_misaligned_base(RD, out_type); \
        if (unlikely(MME_SPARSE_BASE & 1)) { \
            throw trap_ncp_cust_misaligned_base(false, MME_SPARSE_BASE, 0, 0); \
        } \
        check_sp_invalid_shape(BC_SHAPE1_ROW, BC_SHAPE1_COLUMN); \
        check_cust_invalid_shape(4, BC_SHAPE2_COLUMN); \
        int rs1_size = BC_STRIDE_RS1 ? \
                       (BC_STRIDE_RS1 * (BC_SHAPE1_ROW - 1) + BC_SHAPE1_COLUMN) * sizeof(rs1_type) : \
                       (BC_SHAPE1_COLUMN * sizeof(rs1_type)) * BC_SHAPE1_ROW; \
        int rs2_size = BC_STRIDE_RS2 ? \
                       (BC_STRIDE_RS2 * (BC_SHAPE2_ROW - 1) + BC_SHAPE2_COLUMN) * sizeof(rs2_type) : \
                       (BC_SHAPE2_COLUMN * sizeof(rs2_type)) * BC_SHAPE2_ROW; \
        int idx_size = MME_SPARSE_STRIDE ? \
                       (MME_SPARSE_STRIDE * (BC_SHAPE2_ROW - 1) + BC_SHAPE2_ROW): \
                       BC_SHAPE2_COLUMN * BC_SHAPE2_ROW; \
        idx_size = (idx_size + 3) / 4; \
        int rd_size = BC_STRIDE_RD ? \
                      (BC_STRIDE_RD * (BC_SHAPE1_ROW - 1) + BC_SHAPE2_COLUMN) * sizeof(out_type) : \
                      (BC_SHAPE2_COLUMN * sizeof(out_type)) * BC_SHAPE1_ROW;\
        check_cust_access(RS1, rs1_size); \
        check_cust_access(RS2, rs2_size); \
        check_cust_access_im(RD, rd_size); \
        check_cust_access_sp(MME_SPARSE_BASE, idx_size); \
  })
//        check_cust_invalid_params_misaligned_4(TRAP_MATRIX);
// check traps for metr.m instruction
#define check_traps_metr_m(in_type, out_type)({ \
        check_cust_misaligned_base(RS1, in_type); \
        check_cust_misaligned_base(RD, out_type); \
        check_cust_invalid_shape(BC_SHAPE1_ROW, BC_SHAPE1_COLUMN); \
        int rs_size = BC_STRIDE_RS1 ? \
                      (BC_STRIDE_RS1 * (BC_SHAPE1_ROW - 1) + BC_SHAPE1_COLUMN) * sizeof(in_type) : \
                      (BC_SHAPE1_COLUMN * sizeof(in_type)) * BC_SHAPE1_ROW; \
        int rd_size = BC_STRIDE_RD ? \
                      (BC_STRIDE_RD * (BC_SHAPE1_COLUMN - 1) + BC_SHAPE1_ROW) * sizeof(out_type) : \
                      (BC_SHAPE1_ROW * sizeof(out_type)) * BC_SHAPE1_COLUMN; \
        check_cust_access(RS1, rs_size); \
        check_cust_access_im(RD, rd_size); \
  })

// check traps for memin.m/memax.m/meacc.m instructions, reduce all
#define check_traps_mexxx_m(in_type, out_type)({ \
        check_cust_misaligned_base(RS1, in_type); \
        check_cust_misaligned_base(RD, out_type); \
        check_cust_invalid_shape(BC_SHAPE1_ROW, BC_SHAPE1_COLUMN * sizeof(in_type)); \
        int rs1_size = BC_STRIDE_RS1 ? \
                       (BC_STRIDE_RS1 * (BC_SHAPE1_ROW - 1) + BC_SHAPE1_COLUMN) * sizeof(in_type): \
                       BC_SHAPE1_COLUMN * sizeof(in_type) * BC_SHAPE1_ROW; \
        int rd_size = BC_SHAPE1_ROW * sizeof(out_type); \
        check_cust_access(RS1, rs1_size); \
        check_cust_access_im(RD, rd_size); \
  })


// check traps for memin.m/memax.m/meacc.m instructions, reduce all
#define check_traps_meargxxx_m(in_type, out_type)({ \
        check_cust_misaligned_base(RS1, in_type); \
        check_cust_misaligned_base(RD, out_type); \
        check_cust_invalid_shape(BC_SHAPE1_ROW, BC_SHAPE1_COLUMN * sizeof(in_type)); \
        int rs1_size = BC_STRIDE_RS1 ? \
                       (BC_STRIDE_RS1 * (BC_SHAPE1_ROW - 1) + BC_SHAPE1_COLUMN) * sizeof(in_type): \
                       BC_SHAPE1_COLUMN * sizeof(in_type) * BC_SHAPE1_ROW; \
        int rd_size = BC_SHAPE1_ROW * sizeof(out_type); \
        check_cust_access(RS1, rs1_size); \
        check_cust_access_im(RD, rd_size); \
  })

// check traps for meconv.mm instructions
#define check_traps_meconv_mm(rs1_type, rs2_type, out_type) ({ \
        check_cust_misaligned_base(RS1, rs1_type); \
        check_cust_misaligned_base(RS2, rs2_type); \
        check_cust_misaligned_base(RD, out_type); \
        check_cust_invalid_shape(CONV_IN_ROW, CONV_IN_COLUMN); \
        check_cust_invalid_shape(CONV_OUT_ROW, CONV_OUT_COLUMN); \
        check_cust_invalid_shape(CONV_CIN, CONV_COUT); \
        check_cust_invalid_shape(CONV_KH, CONV_KW); \
        check_cust_invalid_shape(CONV_SH, CONV_DH); \
        if (unlikely(CONV_S2_STRIDE && CONV_S2_STRIDE < CONV_CIN)) { \
            throw trap_ncp_cust_invalid_param(); \
        } \
        int rs1_size = CONV_IN_STRIDE ? \
                       (CONV_IN_STRIDE * (CONV_IN_COLUMN * CONV_IN_ROW - 1) + CONV_CIN) * sizeof(rs1_type): \
                       CONV_IN_COLUMN * CONV_IN_ROW * CONV_CIN * sizeof(rs1_type);\
        int rs2_size = CONV_W_STRIDE ? \
                       (CONV_W_STRIDE * (CONV_KH * CONV_KW * CONV_CIN - 1) + CONV_COUT) * sizeof(rs2_type): \
                       CONV_KH * CONV_KW * CONV_CIN * CONV_COUT * sizeof(rs2_type);\
        int rd_size =  CONV_OUT_STRIDE ? \
                       (CONV_OUT_STRIDE * (CONV_OUT_COLUMN * CONV_OUT_ROW - 1) + CONV_COUT) * sizeof(out_type): \
                       CONV_OUT_COLUMN * CONV_OUT_ROW * CONV_COUT * sizeof(out_type);\
        check_cust_access(RS1, rs1_size); \
        check_cust_access(RS2, rs2_size); \
        check_cust_access_im(RD, rd_size); \
  })

// check traps for mme int8 mv instructions(conv=1,conv; 0, matrix)
#define check_traps_int8_mv(conv) ({ \
        check_traps_misaligned_int8_mv(); \
        int mv_size = conv ? CONV_COUT : BC_SHAPE2_COLUMN;\
        check_cust_access_l1(MME_DEQUANT_COEFF, mv_size);\
  })

// check traps for medwconv.mm instructions
#define check_traps_medwconv_mm(rs1_type, rs2_type, out_type) ({ \
        check_cust_misaligned_base(RS1, rs1_type); \
        check_cust_misaligned_base(RS2, rs2_type); \
        check_cust_misaligned_base(RD, out_type); \
        check_cust_invalid_shape(CONV_IN_ROW, CONV_IN_COLUMN); \
        check_cust_invalid_shape(CONV_OUT_ROW, CONV_OUT_COLUMN); \
        check_cust_invalid_shape(CONV_CIN, CONV_CIN); \
        check_cust_invalid_shape(CONV_KH, CONV_KW); \
        if (unlikely(CONV_SH == 0 || CONV_DH == 0)) { \
            throw trap_ncp_cust_invalid_param(); \
        } \
        int rs1_size = CONV_IN_STRIDE ? \
                       (CONV_IN_STRIDE * (CONV_IN_COLUMN * CONV_IN_ROW - 1) + CONV_CIN) * sizeof(rs1_type): \
                       CONV_IN_COLUMN * CONV_IN_ROW * CONV_CIN * sizeof(rs1_type);\
        int rs2_size = CONV_W_STRIDE ? \
                       (CONV_W_STRIDE * (CONV_KH * CONV_KW - 1) + CONV_CIN) * sizeof(rs2_type): \
                       CONV_KH * CONV_KW * CONV_CIN * sizeof(rs2_type);\
        int rd_size =  CONV_OUT_STRIDE ? \
                       (CONV_OUT_STRIDE * (CONV_OUT_COLUMN * CONV_OUT_ROW - 1) + CONV_CIN) * sizeof(out_type): \
                       CONV_OUT_COLUMN * CONV_OUT_ROW * CONV_CIN * sizeof(out_type);\
        check_cust_access(RS1, rs1_size); \
        check_cust_access(RS2, rs2_size); \
        check_cust_access_im(RD, rd_size); \
  })

// check traps for meconv.mm instructions
#define check_traps_medeconv_mm(rs1_type, rs2_type, out_type) ({ \
        check_cust_misaligned_base(RS1, rs1_type); \
        check_cust_misaligned_base(RS2, rs2_type); \
        check_cust_misaligned_base(RD, out_type); \
        check_cust_invalid_shape(CONV_IN_ROW, CONV_IN_COLUMN); \
        check_cust_invalid_shape(CONV_OUT_ROW, CONV_OUT_COLUMN); \
        check_cust_invalid_shape(CONV_CIN, CONV_COUT); \
        check_cust_invalid_shape(CONV_KH, CONV_KW); \
        if (unlikely(CONV_SH == 0 || CONV_SW == 0)) { \
            throw trap_ncp_cust_invalid_param(); \
        } \
        int rs1_size = CONV_IN_STRIDE ? \
                       (CONV_IN_STRIDE * (CONV_IN_COLUMN * CONV_IN_ROW - 1) + CONV_CIN) * sizeof(rs1_type): \
                       CONV_IN_COLUMN * CONV_IN_ROW * CONV_CIN * sizeof(rs1_type);\
        int rs2_size = CONV_W_STRIDE ? \
                       (CONV_W_STRIDE * (CONV_KH * CONV_KW * CONV_CIN - 1) + CONV_COUT) * sizeof(rs2_type): \
                       CONV_KH * CONV_KW * CONV_CIN * CONV_COUT * sizeof(rs2_type);\
        int rd_size =  CONV_OUT_STRIDE ? \
                       (CONV_OUT_STRIDE * (CONV_OUT_COLUMN * CONV_OUT_ROW - 1) + CONV_COUT) * sizeof(out_type): \
                       CONV_OUT_COLUMN * CONV_OUT_ROW * CONV_COUT * sizeof(out_type);\
        check_cust_access(RS1, rs1_size); \
        check_cust_access(RS2, rs2_size); \
        check_cust_access_im(RD, rd_size); \
  })

// check traps for meconv.mm instructions
#define check_traps_medeconv_sp_mm(rs1_type, rs2_type, out_type) ({ \
        check_cust_misaligned_base(RS1, rs1_type); \
        check_cust_misaligned_base(RS2, rs2_type); \
        check_cust_misaligned_base(RD, out_type); \
        if (unlikely(MME_SPARSE_BASE & 1)) { \
            throw trap_ncp_cust_misaligned_base(false, MME_SPARSE_BASE, 0, 0); \
        } \
        check_cust_invalid_shape(CONV_IN_ROW, CONV_IN_COLUMN); \
        check_cust_invalid_shape(CONV_OUT_ROW, CONV_OUT_COLUMN); \
        check_cust_invalid_shape(CONV_CIN, CONV_COUT); \
        check_cust_invalid_shape(CONV_KH, CONV_KW); \
        check_cust_invalid_params_misaligned_4(TRAP_CONV);\
        if (unlikely(CONV_SH == 0 || CONV_SW == 0)) { \
            throw trap_ncp_cust_invalid_param(); \
        } \
        int rs1_size = CONV_IN_STRIDE ? \
                       (CONV_IN_STRIDE * (CONV_IN_COLUMN * CONV_IN_ROW - 1) + CONV_CIN) * sizeof(rs1_type): \
                       CONV_IN_COLUMN * CONV_IN_ROW * CONV_CIN * sizeof(rs1_type);\
        int rs2_size = CONV_W_STRIDE ? \
                       (CONV_W_STRIDE * (CONV_KH * CONV_KW * CONV_CIN - 1) + CONV_COUT) * sizeof(rs2_type): \
                       CONV_KH * CONV_KW * CONV_CIN * CONV_COUT * sizeof(rs2_type);\
        int rd_size =  CONV_OUT_STRIDE ? \
                       (CONV_OUT_STRIDE * (CONV_OUT_COLUMN * CONV_OUT_ROW - 1) + CONV_COUT) * sizeof(out_type): \
                       CONV_OUT_COLUMN * CONV_OUT_ROW * CONV_COUT * sizeof(out_type);\
        int idx_size = MME_SPARSE_STRIDE ? \
                       MME_SPARSE_STRIDE * (CONV_KH * CONV_KW * CONV_CIN - 1) + CONV_COUT : \
                       CONV_KH * CONV_KW * CONV_CIN * CONV_COUT; \
        idx_size = (idx_size + 3) / 4; \
        check_cust_access(RS1, rs1_size); \
        check_cust_access(RS2, rs2_size); \
        check_cust_access_im(RD, rd_size); \
        check_cust_access_sp(MME_SPARSE_BASE, idx_size); \
  })

// check traps for meconv.sp.mm instructions
#define check_traps_meconv_sp_mm(rs1_type, rs2_type, out_type) ({ \
        check_cust_misaligned_base(RS1, rs1_type); \
        check_cust_misaligned_base(RS2, rs2_type); \
        check_cust_misaligned_base(RD, out_type); \
        if (unlikely(MME_SPARSE_BASE & 1)) { \
            throw trap_ncp_cust_misaligned_base(false, MME_SPARSE_BASE, 0, 0); \
        } \
        check_cust_invalid_shape(CONV_IN_ROW, CONV_IN_COLUMN); \
        check_cust_invalid_shape(CONV_OUT_ROW, CONV_OUT_COLUMN); \
        check_cust_invalid_shape(CONV_CIN, CONV_COUT); \
        check_cust_invalid_shape(CONV_KH, CONV_KW); \
        check_cust_invalid_shape(CONV_SH, CONV_DH); \
        if (unlikely(CONV_S2_STRIDE && CONV_S2_STRIDE < CONV_CIN / 2)) { \
            throw trap_ncp_cust_invalid_param(); \
        } \
        int rs1_size = CONV_IN_STRIDE ? \
                       (CONV_IN_STRIDE * (CONV_IN_COLUMN * CONV_IN_ROW - 1) + CONV_CIN) * sizeof(rs1_type): \
                       CONV_IN_COLUMN * CONV_IN_ROW * CONV_CIN * sizeof(rs1_type);\
        int rs2_size = CONV_W_STRIDE ? \
                       (CONV_W_STRIDE * (CONV_KH * CONV_KW * CONV_CIN - 1) + CONV_COUT) * sizeof(rs2_type): \
                       CONV_KH * CONV_KW * CONV_CIN * CONV_COUT * sizeof(rs2_type);\
        int rd_size =  CONV_OUT_STRIDE ? \
                       (CONV_OUT_STRIDE * (CONV_OUT_COLUMN * CONV_OUT_ROW - 1) + CONV_COUT) * sizeof(out_type): \
                       CONV_OUT_COLUMN * CONV_OUT_ROW * CONV_COUT * sizeof(out_type);\
        int idx_size = MME_SPARSE_STRIDE ? \
                       MME_SPARSE_STRIDE * (CONV_KH * CONV_KW * CONV_CIN - 1) + CONV_COUT : \
                       CONV_KH * CONV_KW * CONV_CIN * CONV_COUT; \
        idx_size = (idx_size + 3) / 4; \
        check_cust_access(RS1, rs1_size); \
        check_cust_access(RS2, rs2_size); \
        check_cust_access_im(RD, rd_size); \
        check_cust_access_sp(MME_SPARSE_BASE, idx_size); \
  })
#define check_tcp_data_type \
  if (MTE_DATA_TYPE_RS1 != MTE_DATA_TYPE_RD) \
    trap_tcp_invalid_param();

// throw trap if tcp csr_bf16 valid but not support
#define check_tcp_csr_bf16(csr_bf16) \
        if (csr_bf16) { \
            throw trap_ncp_illegal_encoding(); \
        }

// throw trap if cust inst access misaligned base address
#define check_tcp_misaligned_base_esize(x, esize) \
        if (unlikely(x & ((esize) - 1))) { \
            throw trap_tcp_access_start(x); \
        }

#define check_tcp_misaligned_base(x, type) \
        if (unlikely(x & (sizeof(type) - 1))) { \
            throw trap_tcp_access_start(x); \
        }

// throw trap if tcp inst use invalid shape, col=0 or row=0
#define check_tcp_invalid_shape(col, row) \
        if (unlikely(col == 0 || row == 0)) { \
            throw trap_tcp_invalid_param(); \
        }

// throw trap if tcp inst use invalid dst stride
#define check_tcp_invalid_stride_less_width(col, stride) \
        if (unlikely((stride != 0 ) && (stride < col))) { \
            throw trap_tcp_invalid_param(); \
        }

// throw trap if tcp icmov's target core id not exist
#define check_tcp_icmov_invalid_core_id(core_id, max_id) \
        if (core_id > max_id) { \
            throw trap_tcp_icmov_invalid_core(); \
        }

// throw trap if tcp pld's target coremap not exist
#define check_tcp_icmov_invalid_coremap(coremap) \
        if (coremap == 0) { \
            throw trap_tcp_coremap_zero(); \
        }


// throw trap if tcp source start address in L1Buffer
#define check_tcp_access_start_l1_with_IRAM(x) \
        if ((!(p->get_sim()->in_local_mem(zext_xlen(x), L1_BUFFER))) && \
            (!(p->get_sim()->in_local_mem(zext_xlen(x), SP_BUFFER)))) { \
            throw trap_tcp_access_start(x); \
        }

#define check_tcp_access_start_l1(x) \
        if (!(p->get_sim()->in_local_mem(zext_xlen(x), L1_BUFFER))) { \
            throw trap_tcp_access_start(x); \
        }

// throw trap if tcp source end address in L1Buffer
#define check_tcp_access_end_l1_with_IRAM(x) \
        if ((!(p->get_sim()->in_local_mem(zext_xlen(x), L1_BUFFER))) && \
            (!(p->get_sim()->in_local_mem(zext_xlen(x), SP_BUFFER)))) { \
            throw trap_tcp_access_end_l1(x); \
        }

#define check_tcp_access_end_l1(x) \
        if (!(p->get_sim()->in_local_mem(zext_xlen(x), L1_BUFFER))) { \
            throw trap_tcp_access_end_l1(x); \
        }

// throw trap if tcp source start address in LLB for mov*
#define check_tcp_access_start_llb_mov(x) \
        if (!((zext_xlen(x) >= LLB_AXI0_BUFFER_START && zext_xlen(x) <= LLB_AXI0_BUFFER_START + \
            LLB_BUFFER_SIZE) || (zext_xlen(x) >= LLB_AXI1_BUFFER_START && \
            zext_xlen(x) <= LLB_AXI1_BUFFER_START + LLB_BUFFER_SIZE))) { \
            throw trap_tcp_access_start(x); \
        }

// throw trap if tcp source start address in LLB for pld
#define check_tcp_access_start_llb_pld(x) \
        if (!((zext_xlen(x) >= LLB_AXI0_BUFFER_START && zext_xlen(x) <= LLB_AXI0_BUFFER_START + \
            LLB_BUFFER_SIZE) || (zext_xlen(x) >= LLB_AXI1_BUFFER_START && \
            zext_xlen(x) <= LLB_AXI1_BUFFER_START + LLB_BUFFER_SIZE))) { \
            throw trap_tcp_access_start(x); \
        } 
        

// throw trap if tcp source end address in L1Buffer
#define check_tcp_access_end_llb(x) \
        if (!((zext_xlen(x) >= LLB_AXI0_BUFFER_START && zext_xlen(x) <= LLB_AXI0_BUFFER_START + \
            LLB_BUFFER_SIZE) || (zext_xlen(x) >= LLB_AXI1_BUFFER_START && \
            zext_xlen(x) <= LLB_AXI1_BUFFER_START + LLB_BUFFER_SIZE))) { \
            throw trap_tcp_access_end_llb(x); \
        }

// throw trap if tcp source end address in L1Buffer
#define check_tcp_invalid_param(col, row, llb_sstride) \
        if (unlikely(col == 0 || row == 0) || (llb_sstride > 0 && unlikely(llb_sstride < col *2))) { \
            throw trap_tcp_invalid_param(); \
        }

// check traps for icmov instruction
#define check_traps_icmov ({ \
        check_tcp_icmov_invalid_core_id(DST_CORE_ID, CORE_COUNT) \
        check_tcp_mte_dtype\
        check_tcp_access_start_l1(RS1) \
        check_tcp_access_end_l1(RS1 + RS2) \
        check_tcp_access_end_l1(RD + RS2) \
        check_tcp_invalid_shape(MTE_SHAPE_COLUMN, MTE_SHAPE_ROW); \
})

// check traps for icmov_m instruction
#define check_traps_icmov_m(esize) ({ \
        check_tcp_icmov_invalid_core_id(DST_CORE_ID, CORE_COUNT) \
        check_tcp_mte_dtype\
        check_tcp_misaligned_base_esize(RS1, esize) \
        check_tcp_misaligned_base_esize(RD, esize) \
        check_tcp_access_start_l1(RS1) \
        check_tcp_access_start_l1(RD) \
        check_tcp_invalid_stride_less_width(MTE_SHAPE_COLUMN, MTE_STRIDE_RS1)\
        check_tcp_invalid_stride_less_width(MTE_SHAPE_COLUMN, MTE_STRIDE_RD) \
        check_tcp_invalid_shape(MTE_SHAPE_COLUMN, MTE_SHAPE_ROW); \
        int rs_size = MTE_STRIDE_RS1 ? (MTE_STRIDE_RS1 * (MTE_SHAPE_ROW -1) + MTE_SHAPE_COLUMN) * esize : MTE_SHAPE_COLUMN * esize * MTE_SHAPE_ROW; \
        check_tcp_access_end_l1(RS1 + rs_size) \
        int rd_size = MTE_STRIDE_RD ? (MTE_STRIDE_RD * (MTE_SHAPE_ROW -1) + MTE_SHAPE_COLUMN) * esize : MTE_SHAPE_COLUMN * esize * MTE_SHAPE_ROW; \
        check_tcp_access_end_l1(RD + rd_size) \
})

#define check_trap_sync_mte_r() ({\
        check_tcp_invalid_stride_less_width(MTE_SHAPE_COLUMN, MTE_STRIDE_RS1) \
})

// check traps for pld instruction
#define check_traps_pld(esize) ({ \
        check_tcp_mte_dtype \
        check_tcp_misaligned_base_esize(RS1, esize) \
        check_tcp_misaligned_base_esize(RD, esize) \
        check_tcp_icmov_invalid_coremap(RS2) \
        check_tcp_access_start_l1_with_IRAM(RD) \
        check_tcp_access_start_llb_pld(RS1) \
        check_tcp_invalid_stride_less_width(MTE_SHAPE_COLUMN, MTE_STRIDE_RS1) \
        check_tcp_invalid_stride_less_width(MTE_SHAPE_COLUMN, MTE_STRIDE_RD) \
        check_tcp_invalid_shape(MTE_SHAPE_COLUMN, MTE_SHAPE_ROW); \
        int rd_size = MTE_STRIDE_RD ? (MTE_STRIDE_RD * (MTE_SHAPE_ROW -1) + MTE_SHAPE_COLUMN) * esize : MTE_SHAPE_COLUMN * esize * MTE_SHAPE_ROW; \
        check_tcp_access_end_l1_with_IRAM(RD + rd_size) \
        int rs_size = MTE_STRIDE_RS1 ? (MTE_STRIDE_RS1 * (MTE_SHAPE_ROW -1) + MTE_SHAPE_COLUMN) * esize : MTE_SHAPE_COLUMN * esize * MTE_SHAPE_ROW; \
        check_tcp_access_end_llb(RS1 + rs_size) \
})

// check traps for mov.l1.llb instruction
#define check_traps_mov_l1_llb(etype) ({ \
        check_tcp_misaligned_base(RS1, etype); \
        check_tcp_misaligned_base(RD, etype); \
        check_tcp_invalid_shape(MTE_SHAPE_COLUMN, MTE_SHAPE_ROW); \
        check_tcp_access_start_llb_mov(RS1) \
        check_tcp_access_start_l1_with_IRAM(RD) \
        check_tcp_invalid_stride_less_width(MTE_SHAPE_COLUMN, MTE_STRIDE_RS1) \
        check_tcp_invalid_stride_less_width(MTE_SHAPE_COLUMN, MTE_STRIDE_RD) \
        int rs_size = MTE_STRIDE_RS1 ? (MTE_STRIDE_RS1 * (MTE_SHAPE_ROW -1) + MTE_SHAPE_COLUMN) * esize : MTE_SHAPE_COLUMN * esize * MTE_SHAPE_ROW; \
        check_tcp_access_end_llb(RS1 + rs_size) \
        int rd_size = MTE_STRIDE_RD ? (MTE_STRIDE_RD * (MTE_SHAPE_ROW -1) + MTE_SHAPE_COLUMN) * esize : MTE_SHAPE_COLUMN * esize * MTE_SHAPE_ROW; \
        check_tcp_access_end_l1_with_IRAM(RD + rd_size) \
})

// check traps for mov.llb.l instruction
#define check_traps_mov_llb_l1(etype) ({ \
        check_tcp_misaligned_base(RS1, etype); \
        check_tcp_misaligned_base(RD, etype); \
        check_tcp_invalid_shape(MTE_SHAPE_COLUMN, MTE_SHAPE_ROW); \
        check_tcp_access_start_l1(RS1) \
        check_tcp_access_start_llb_mov(RD) \
        check_tcp_invalid_stride_less_width(MTE_SHAPE_COLUMN, MTE_STRIDE_RS1) \
        check_tcp_invalid_stride_less_width(MTE_SHAPE_COLUMN, MTE_STRIDE_RD) \
        int rs_size = MTE_STRIDE_RS1 ? (MTE_STRIDE_RS1 * (MTE_SHAPE_ROW -1) + MTE_SHAPE_COLUMN) * esize : MTE_SHAPE_COLUMN * esize * MTE_SHAPE_ROW; \
        check_tcp_access_end_l1(RS1 + rs_size) \
        int rd_size = MTE_STRIDE_RD ? (MTE_STRIDE_RD * (MTE_SHAPE_ROW -1) + MTE_SHAPE_COLUMN) * esize : MTE_SHAPE_COLUMN * esize * MTE_SHAPE_ROW; \
        check_tcp_access_end_llb(RD + rd_size) \
})

//check trap mov.l1.glb
#define check_trap_mov_l1_glb(esize)({ \
        check_tcp_access_start_l1(RD) \
        int rd_size = (DMAE_STRIDE_D_Y ? (DMAE_STRIDE_D_Y * (DMAE_SHAPE_Z -1) + DMAE_SHAPE_X * DMAE_SHAPE_Y) : \
          (DMAE_STRIDE_D_X ? (DMAE_STRIDE_D_X  * (DMAE_SHAPE_Y - 1) + DMAE_SHAPE_X): DMAE_SHAPE_X * DMAE_SHAPE_Y) * DMAE_SHAPE_Z) * esize; \
        check_tcp_access_end_l1(RD + rd_size) \
})

//check trap mov.glb.l1
#define check_trap_mov_glb_l1(esize)({ \
        check_tcp_access_start_l1(RS1) \
        int rs_size = (DMAE_STRIDE_S_Y ? (DMAE_STRIDE_S_Y * (DMAE_SHAPE_Z -1) + DMAE_SHAPE_X * DMAE_SHAPE_Y) : \
          (DMAE_STRIDE_S_X ? (DMAE_STRIDE_S_X  * (DMAE_SHAPE_Y - 1) + DMAE_SHAPE_X): DMAE_SHAPE_X * DMAE_SHAPE_Y) * DMAE_SHAPE_Z) * esize; \
        check_tcp_access_end_l1(RS1 + rs_size) \
})

//check trap mov.llb.glb
#define check_trap_mov_llb_glb(esize) ({ \
  check_tcp_access_start_llb_mov(RD) \
   int rd_size = (DMAE_STRIDE_D_Y ? (DMAE_STRIDE_D_Y * (DMAE_SHAPE_Z -1) + DMAE_SHAPE_X * DMAE_SHAPE_Y) : \
          (DMAE_STRIDE_D_X ? (DMAE_STRIDE_D_X  * (DMAE_SHAPE_Y - 1) + DMAE_SHAPE_X): DMAE_SHAPE_X * DMAE_SHAPE_Y) * DMAE_SHAPE_Z) * esize; \
  check_tcp_access_end_llb(RD + rd_size) \
})

//check trap mov.glb.llb
#define check_trap_mov_glb_llb(esize) ({ \
  check_tcp_access_start_llb_mov(RS1) \
  int rs_size = (DMAE_STRIDE_S_Y ? (DMAE_STRIDE_S_Y * (DMAE_SHAPE_Z -1) + DMAE_SHAPE_X * DMAE_SHAPE_Y) : \
          (DMAE_STRIDE_S_X ? (DMAE_STRIDE_S_X  * (DMAE_SHAPE_Y - 1) + DMAE_SHAPE_X): DMAE_SHAPE_X * DMAE_SHAPE_Y) * DMAE_SHAPE_Z) * esize; \
  check_tcp_access_end_llb(RS1 + rs_size) \
})

//check trap mov.llb.llb
#define check_trap_mov_llb_llb(in_esize, out_esize) ({ \
  check_tcp_access_start_llb_mov(RS1) \
  int rs_size = (DMAE_STRIDE_S_Y ? (DMAE_STRIDE_S_Y * (DMAE_SHAPE_Z -1) + DMAE_SHAPE_X * DMAE_SHAPE_Y) : \
          (DMAE_STRIDE_S_X ? (DMAE_STRIDE_S_X  * (DMAE_SHAPE_Y - 1) + DMAE_SHAPE_X): DMAE_SHAPE_X * DMAE_SHAPE_Y) * DMAE_SHAPE_Z) * in_esize; \
  check_tcp_access_end_llb(RS1 + rs_size) \
  check_tcp_access_start_llb_mov(RD) \
   int rd_size = (DMAE_STRIDE_D_Y ? (DMAE_STRIDE_D_Y * (DMAE_SHAPE_Z -1) + DMAE_SHAPE_X * DMAE_SHAPE_Y) : \
          (DMAE_STRIDE_D_X ? (DMAE_STRIDE_D_X  * (DMAE_SHAPE_Y - 1) + DMAE_SHAPE_X): DMAE_SHAPE_X * DMAE_SHAPE_Y) * DMAE_SHAPE_Z) * out_esize; \
  check_tcp_access_end_llb(RD + rd_size) \
})

#define check_trap_mmu_pmp_ok(addr, len, type, mode) ({ \
  reg_t ret = MMU.check_pmp_ok(addr, len, type, mode); \
  if (!ret) \
    throw trap_ncp_cust_access(false, addr, 0, 0); \
})

//
// vector: loop header and end helper
//
#define VI_GENERAL_LOOP_BASE \
  require(P.VU.vsew >= e8 && P.VU.vsew <= e64); \
  require_vector(true);\
  reg_t vl = P.VU.vl; \
  reg_t sew = P.VU.vsew; \
  reg_t rd_num = insn.rd(); \
  reg_t rs1_num = insn.rs1(); \
  reg_t rs2_num = insn.rs2(); \
  for (reg_t i=P.VU.vstart; i<vl; ++i){ 

#define VI_LOOP_BASE \
    VI_GENERAL_LOOP_BASE \
    VI_LOOP_ELEMENT_SKIP();

#define VI_LOOP_END \
  } \
  P.VU.vstart = 0;

#define VI_LOOP_REDUCTION_END(x) \
  } \
  if (vl > 0) { \
    vd_0_des = vd_0_res; \
  } \
  P.VU.vstart = 0; 

#define VI_LOOP_CMP_BASE \
  require(P.VU.vsew >= e8 && P.VU.vsew <= e64); \
  require_vector(true);\
  reg_t vl = P.VU.vl; \
  reg_t sew = P.VU.vsew; \
  reg_t rd_num = insn.rd(); \
  reg_t rs1_num = insn.rs1(); \
  reg_t rs2_num = insn.rs2(); \
  for (reg_t i=P.VU.vstart; i<vl; ++i){ \
    VI_LOOP_ELEMENT_SKIP(); \
    uint64_t mmask = UINT64_C(1) << mpos; \
    uint64_t &vdi = P.VU.elt<uint64_t>(insn.rd(), midx, true); \
    uint64_t res = 0;

#define VI_LOOP_CMP_END \
    vdi = (vdi & ~mmask) | (((res) << mpos) & mmask); \
  } \
  P.VU.vstart = 0;

#define VI_LOOP_MASK(op) \
  require(P.VU.vsew <= e64); \
  require_vector(true);\
  reg_t vl = P.VU.vl; \
  for (reg_t i = P.VU.vstart; i < vl; ++i) { \
    int midx = i / 64; \
    int mpos = i % 64; \
    uint64_t mmask = UINT64_C(1) << mpos; \
    uint64_t vs2 = P.VU.elt<uint64_t>(insn.rs2(), midx); \
    uint64_t vs1 = P.VU.elt<uint64_t>(insn.rs1(), midx); \
    uint64_t &res = P.VU.elt<uint64_t>(insn.rd(), midx, true); \
    res = (res & ~mmask) | ((op) & (1ULL << mpos)); \
  } \
  P.VU.vstart = 0;

#define VI_LOOP_NSHIFT_BASE \
  VI_GENERAL_LOOP_BASE; \
  VI_LOOP_ELEMENT_SKIP({\
    require(!(insn.rd() == 0 && P.VU.vflmul > 1));\
  });


#define INT_ROUNDING(result, xrm, gb) \
  do { \
    const uint64_t lsb = 1UL << (gb); \
    const uint64_t lsb_half = lsb >> 1; \
    switch (xrm) {\
      case VRM::RNU:\
        result += lsb_half; \
        break;\
      case VRM::RNE:\
        if ((result & lsb_half) && ((result & (lsb_half - 1)) || (result & lsb))) \
          result += lsb; \
        break;\
      case VRM::RDN:\
        break;\
      case VRM::ROD:\
        if (result & (lsb - 1)) \
          result |= lsb; \
        break;\
      case VRM::INVALID_RM:\
        assert(true);\
    } \
  } while (0)

//
// vector: integer and masking operand access helper
//
#define VXI_PARAMS(x) \
  type_sew_t<x>::type &vd = P.VU.elt<type_sew_t<x>::type>(rd_num, i, true); \
  type_sew_t<x>::type vs1 = P.VU.elt<type_sew_t<x>::type>(rs1_num, i); \
  type_sew_t<x>::type vs2 = P.VU.elt<type_sew_t<x>::type>(rs2_num, i); \
  type_sew_t<x>::type rs1 = (type_sew_t<x>::type)RS1; \
  type_sew_t<x>::type simm5 = (type_sew_t<x>::type)insn.v_simm5();

#define VV_U_PARAMS(x) \
  type_usew_t<x>::type &vd = P.VU.elt<type_usew_t<x>::type>(rd_num, i, true); \
  type_usew_t<x>::type vs1 = P.VU.elt<type_usew_t<x>::type>(rs1_num, i); \
  type_usew_t<x>::type vs2 = P.VU.elt<type_usew_t<x>::type>(rs2_num, i);

#define VX_U_PARAMS(x) \
  type_usew_t<x>::type &vd = P.VU.elt<type_usew_t<x>::type>(rd_num, i, true); \
  type_usew_t<x>::type rs1 = (type_usew_t<x>::type)RS1; \
  type_usew_t<x>::type vs2 = P.VU.elt<type_usew_t<x>::type>(rs2_num, i);

#define VI_U_PARAMS(x) \
  type_usew_t<x>::type &vd = P.VU.elt<type_usew_t<x>::type>(rd_num, i, true); \
  type_usew_t<x>::type zimm5 = (type_usew_t<x>::type)insn.v_zimm5(); \
  type_usew_t<x>::type vs2 = P.VU.elt<type_usew_t<x>::type>(rs2_num, i);

#define VV_PARAMS(x) \
  type_sew_t<x>::type &vd = P.VU.elt<type_sew_t<x>::type>(rd_num, i, true); \
  type_sew_t<x>::type vs1 = P.VU.elt<type_sew_t<x>::type>(rs1_num, i); \
  type_sew_t<x>::type vs2 = P.VU.elt<type_sew_t<x>::type>(rs2_num, i);

#define VX_PARAMS(x) \
  type_sew_t<x>::type &vd = P.VU.elt<type_sew_t<x>::type>(rd_num, i, true); \
  type_sew_t<x>::type rs1 = (type_sew_t<x>::type)RS1; \
  type_sew_t<x>::type vs2 = P.VU.elt<type_sew_t<x>::type>(rs2_num, i);

#define VI_PARAMS(x) \
  type_sew_t<x>::type &vd = P.VU.elt<type_sew_t<x>::type>(rd_num, i, true); \
  type_sew_t<x>::type simm5 = (type_sew_t<x>::type)insn.v_simm5(); \
  type_sew_t<x>::type vs2 = P.VU.elt<type_sew_t<x>::type>(rs2_num, i);

#define XV_PARAMS(x) \
  type_sew_t<x>::type &vd = P.VU.elt<type_sew_t<x>::type>(rd_num, i, true); \
  type_usew_t<x>::type vs2 = P.VU.elt<type_usew_t<x>::type>(rs2_num, RS1);

#define VV_UCMP_PARAMS(x) \
  type_usew_t<x>::type vs1 = P.VU.elt<type_usew_t<x>::type>(rs1_num, i); \
  type_usew_t<x>::type vs2 = P.VU.elt<type_usew_t<x>::type>(rs2_num, i);

#define VX_UCMP_PARAMS(x) \
  type_usew_t<x>::type rs1 = (type_usew_t<x>::type)RS1; \
  type_usew_t<x>::type vs2 = P.VU.elt<type_usew_t<x>::type>(rs2_num, i);

#define VI_UCMP_PARAMS(x) \
  type_usew_t<x>::type vs2 = P.VU.elt<type_usew_t<x>::type>(rs2_num, i);

#define VV_CMP_PARAMS(x) \
  type_sew_t<x>::type vs1 = P.VU.elt<type_sew_t<x>::type>(rs1_num, i); \
  type_sew_t<x>::type vs2 = P.VU.elt<type_sew_t<x>::type>(rs2_num, i);

#define VX_CMP_PARAMS(x) \
  type_sew_t<x>::type rs1 = (type_sew_t<x>::type)RS1; \
  type_sew_t<x>::type vs2 = P.VU.elt<type_sew_t<x>::type>(rs2_num, i);

#define VI_CMP_PARAMS(x) \
  type_sew_t<x>::type simm5 = (type_sew_t<x>::type)insn.v_simm5(); \
  type_sew_t<x>::type vs2 = P.VU.elt<type_sew_t<x>::type>(rs2_num, i);

#define VI_XI_SLIDEDOWN_PARAMS(x, off) \
  auto &vd = P.VU.elt<type_sew_t<x>::type>(rd_num, i, true); \
  auto vs2 = P.VU.elt<type_sew_t<x>::type>(rs2_num, i + off);

#define VI_XI_SLIDEUP_PARAMS(x, offset) \
  auto &vd = P.VU.elt<type_sew_t<x>::type>(rd_num, i, true); \
  auto vs2 = P.VU.elt<type_sew_t<x>::type>(rs2_num, i - offset);

#define VI_NSHIFT_PARAMS(sew1, sew2) \
  auto &vd = P.VU.elt<type_usew_t<sew1>::type>(rd_num, i, true); \
  auto vs2_u = P.VU.elt<type_usew_t<sew2>::type>(rs2_num, i); \
  auto vs2 = P.VU.elt<type_sew_t<sew2>::type>(rs2_num, i); \
  auto zimm5 = (type_usew_t<sew1>::type)insn.v_zimm5();

#define VX_NSHIFT_PARAMS(sew1, sew2) \
  auto &vd = P.VU.elt<type_usew_t<sew1>::type>(rd_num, i, true); \
  auto vs2_u = P.VU.elt<type_usew_t<sew2>::type>(rs2_num, i); \
  auto vs2 = P.VU.elt<type_sew_t<sew2>::type>(rs2_num, i); \
  auto rs1 = (type_sew_t<sew1>::type)RS1;

#define VV_NSHIFT_PARAMS(sew1, sew2) \
  auto &vd = P.VU.elt<type_usew_t<sew1>::type>(rd_num, i, true); \
  auto vs2_u = P.VU.elt<type_usew_t<sew2>::type>(rs2_num, i); \
  auto vs2 = P.VU.elt<type_sew_t<sew2>::type>(rs2_num, i); \
  auto vs1 = P.VU.elt<type_sew_t<sew1>::type>(rs1_num, i);

#define XI_CARRY_PARAMS(x) \
  auto vs2 = P.VU.elt<type_sew_t<x>::type>(rs2_num, i); \
  auto rs1 = (type_sew_t<x>::type)RS1; \
  auto simm5 = (type_sew_t<x>::type)insn.v_simm5(); \
  auto &vd = P.VU.elt<uint64_t>(rd_num, midx, true);

#define VV_CARRY_PARAMS(x) \
  auto vs2 = P.VU.elt<type_sew_t<x>::type>(rs2_num, i); \
  auto vs1 = P.VU.elt<type_sew_t<x>::type>(rs1_num, i); \
  auto &vd = P.VU.elt<uint64_t>(rd_num, midx, true);

#define XI_WITH_CARRY_PARAMS(x) \
  auto vs2 = P.VU.elt<type_sew_t<x>::type>(rs2_num, i); \
  auto rs1 = (type_sew_t<x>::type)RS1; \
  auto simm5 = (type_sew_t<x>::type)insn.v_simm5(); \
  auto &vd = P.VU.elt<type_sew_t<x>::type>(rd_num, i, true);

#define VV_WITH_CARRY_PARAMS(x) \
  auto vs2 = P.VU.elt<type_sew_t<x>::type>(rs2_num, i); \
  auto vs1 = P.VU.elt<type_sew_t<x>::type>(rs1_num, i); \
  auto &vd = P.VU.elt<type_sew_t<x>::type>(rd_num, i, true);

//
// vector: integer and masking operation loop
//

// comparision result to masking register
#define VI_VV_LOOP_CMP(BODY) \
  VI_CHECK_MSS(true); \
  VI_LOOP_CMP_BASE \
  if (sew == e8){ \
    VV_CMP_PARAMS(e8); \
    BODY; \
  }else if(sew == e16){ \
    VV_CMP_PARAMS(e16); \
    BODY; \
  }else if(sew == e32){ \
    VV_CMP_PARAMS(e32); \
    BODY; \
  }else if(sew == e64){ \
    VV_CMP_PARAMS(e64); \
    BODY; \
  } \
  VI_LOOP_CMP_END

#define VI_VX_LOOP_CMP(BODY) \
  VI_CHECK_MSS(false); \
  VI_LOOP_CMP_BASE \
  if (sew == e8){ \
    VX_CMP_PARAMS(e8); \
    BODY; \
  }else if(sew == e16){ \
    VX_CMP_PARAMS(e16); \
    BODY; \
  }else if(sew == e32){ \
    VX_CMP_PARAMS(e32); \
    BODY; \
  }else if(sew == e64){ \
    VX_CMP_PARAMS(e64); \
    BODY; \
  } \
  VI_LOOP_CMP_END

#define VI_VI_LOOP_CMP(BODY) \
  VI_CHECK_MSS(false); \
  VI_LOOP_CMP_BASE \
  if (sew == e8){ \
    VI_CMP_PARAMS(e8); \
    BODY; \
  }else if(sew == e16){ \
    VI_CMP_PARAMS(e16); \
    BODY; \
  }else if(sew == e32){ \
    VI_CMP_PARAMS(e32); \
    BODY; \
  }else if(sew == e64){ \
    VI_CMP_PARAMS(e64); \
    BODY; \
  } \
  VI_LOOP_CMP_END

#define VI_VV_ULOOP_CMP(BODY) \
  VI_CHECK_MSS(true); \
  VI_LOOP_CMP_BASE \
  if (sew == e8){ \
    VV_UCMP_PARAMS(e8); \
    BODY; \
  }else if(sew == e16){ \
    VV_UCMP_PARAMS(e16); \
    BODY; \
  }else if(sew == e32){ \
    VV_UCMP_PARAMS(e32); \
    BODY; \
  }else if(sew == e64){ \
    VV_UCMP_PARAMS(e64); \
    BODY; \
  } \
  VI_LOOP_CMP_END

#define VI_VX_ULOOP_CMP(BODY) \
  VI_CHECK_MSS(false); \
  VI_LOOP_CMP_BASE \
  if (sew == e8){ \
    VX_UCMP_PARAMS(e8); \
    BODY; \
  }else if(sew == e16){ \
    VX_UCMP_PARAMS(e16); \
    BODY; \
  }else if(sew == e32){ \
    VX_UCMP_PARAMS(e32); \
    BODY; \
  }else if(sew == e64){ \
    VX_UCMP_PARAMS(e64); \
    BODY; \
  } \
  VI_LOOP_CMP_END

#define VI_VI_ULOOP_CMP(BODY) \
  VI_CHECK_MSS(false); \
  VI_LOOP_CMP_BASE \
  if (sew == e8){ \
    VI_UCMP_PARAMS(e8); \
    BODY; \
  }else if(sew == e16){ \
    VI_UCMP_PARAMS(e16); \
    BODY; \
  }else if(sew == e32){ \
    VI_UCMP_PARAMS(e32); \
    BODY; \
  }else if(sew == e64){ \
    VI_UCMP_PARAMS(e64); \
    BODY; \
  } \
  VI_LOOP_CMP_END

// merge and copy loop
#define VI_VVXI_MERGE_LOOP(BODY) \
  VI_GENERAL_LOOP_BASE \
  if (sew == e8){ \
    VXI_PARAMS(e8); \
    BODY; \
  }else if(sew == e16){ \
    VXI_PARAMS(e16); \
    BODY; \
  }else if(sew == e32){ \
    VXI_PARAMS(e32); \
    BODY; \
  }else if(sew == e64){ \
    VXI_PARAMS(e64); \
    BODY; \
  } \
  VI_LOOP_END 

// reduction loop - signed
#define VI_LOOP_REDUCTION_BASE(x) \
  require(x >= e8 && x <= e64); \
  reg_t vl = P.VU.vl; \
  reg_t rd_num = insn.rd(); \
  reg_t rs1_num = insn.rs1(); \
  reg_t rs2_num = insn.rs2(); \
  auto &vd_0_des = P.VU.elt<type_sew_t<x>::type>(rd_num, 0, true); \
  auto vd_0_res = P.VU.elt<type_sew_t<x>::type>(rs1_num, 0); \
  for (reg_t i=P.VU.vstart; i<vl; ++i){ \
    VI_LOOP_ELEMENT_SKIP(); \
    auto vs2 = P.VU.elt<type_sew_t<x>::type>(rs2_num, i); \

#define REDUCTION_LOOP(x, BODY) \
  VI_LOOP_REDUCTION_BASE(x) \
  BODY; \
  VI_LOOP_REDUCTION_END(x)

#define VI_VV_LOOP_REDUCTION(BODY) \
  VI_CHECK_REDUCTION(false); \
  reg_t sew = P.VU.vsew; \
  if (sew == e8) { \
    REDUCTION_LOOP(e8, BODY) \
  } else if(sew == e16) { \
    REDUCTION_LOOP(e16, BODY) \
  } else if(sew == e32) { \
    REDUCTION_LOOP(e32, BODY) \
  } else if(sew == e64) { \
    REDUCTION_LOOP(e64, BODY) \
  }

// reduction loop - unsgied
#define VI_ULOOP_REDUCTION_BASE(x) \
  require(x >= e8 && x <= e64); \
  reg_t vl = P.VU.vl; \
  reg_t rd_num = insn.rd(); \
  reg_t rs1_num = insn.rs1(); \
  reg_t rs2_num = insn.rs2(); \
  auto &vd_0_des = P.VU.elt<type_usew_t<x>::type>(rd_num, 0, true); \
  auto vd_0_res = P.VU.elt<type_usew_t<x>::type>(rs1_num, 0); \
  for (reg_t i=P.VU.vstart; i<vl; ++i){ \
    VI_LOOP_ELEMENT_SKIP(); \
    auto vs2 = P.VU.elt<type_usew_t<x>::type>(rs2_num, i);

#define REDUCTION_ULOOP(x, BODY) \
  VI_ULOOP_REDUCTION_BASE(x) \
  BODY; \
  VI_LOOP_REDUCTION_END(x)

#define VI_VV_ULOOP_REDUCTION(BODY) \
  VI_CHECK_REDUCTION(false); \
  reg_t sew = P.VU.vsew; \
  if (sew == e8){ \
    REDUCTION_ULOOP(e8, BODY) \
  } else if(sew == e16) { \
    REDUCTION_ULOOP(e16, BODY) \
  } else if(sew == e32) { \
    REDUCTION_ULOOP(e32, BODY) \
  } else if(sew == e64) { \
    REDUCTION_ULOOP(e64, BODY) \
  }


// genearl VXI signed/unsgied loop
#define VI_VV_ULOOP(BODY) \
  VI_CHECK_SSS(true) \
  VI_LOOP_BASE \
  if (sew == e8){ \
    VV_U_PARAMS(e8); \
    BODY; \
  }else if(sew == e16){ \
    VV_U_PARAMS(e16); \
    BODY; \
  }else if(sew == e32){ \
    VV_U_PARAMS(e32); \
    BODY; \
  }else if(sew == e64){ \
    VV_U_PARAMS(e64); \
    BODY; \
  } \
  VI_LOOP_END 

#define VI_VV_LOOP(BODY) \
  VI_CHECK_SSS(true) \
  VI_LOOP_BASE \
  if (sew == e8){ \
    VV_PARAMS(e8); \
    BODY; \
  }else if(sew == e16){ \
    VV_PARAMS(e16); \
    BODY; \
  }else if(sew == e32){ \
    VV_PARAMS(e32); \
    BODY; \
  }else if(sew == e64){ \
    VV_PARAMS(e64); \
    BODY; \
  } \
  VI_LOOP_END 

#define VI_VX_ULOOP(BODY) \
  VI_CHECK_SSS(false) \
  VI_LOOP_BASE \
  if (sew == e8){ \
    VX_U_PARAMS(e8); \
    BODY; \
  }else if(sew == e16){ \
    VX_U_PARAMS(e16); \
    BODY; \
  }else if(sew == e32){ \
    VX_U_PARAMS(e32); \
    BODY; \
  }else if(sew == e64){ \
    VX_U_PARAMS(e64); \
    BODY; \
  } \
  VI_LOOP_END 

#define VI_VX_LOOP(BODY) \
  VI_CHECK_SSS(false) \
  VI_LOOP_BASE \
  if (sew == e8){ \
    VX_PARAMS(e8); \
    BODY; \
  }else if(sew == e16){ \
    VX_PARAMS(e16); \
    BODY; \
  }else if(sew == e32){ \
    VX_PARAMS(e32); \
    BODY; \
  }else if(sew == e64){ \
    VX_PARAMS(e64); \
    BODY; \
  } \
  VI_LOOP_END 

#define VI_VI_ULOOP(BODY) \
  VI_CHECK_SSS(false) \
  VI_LOOP_BASE \
  if (sew == e8){ \
    VI_U_PARAMS(e8); \
    BODY; \
  }else if(sew == e16){ \
    VI_U_PARAMS(e16); \
    BODY; \
  }else if(sew == e32){ \
    VI_U_PARAMS(e32); \
    BODY; \
  }else if(sew == e64){ \
    VI_U_PARAMS(e64); \
    BODY; \
  } \
  VI_LOOP_END 

#define VI_VI_LOOP(BODY) \
  VI_CHECK_SSS(false) \
  VI_LOOP_BASE \
  if (sew == e8){ \
    VI_PARAMS(e8); \
    BODY; \
  }else if(sew == e16){ \
    VI_PARAMS(e16); \
    BODY; \
  }else if(sew == e32){ \
    VI_PARAMS(e32); \
    BODY; \
  }else if(sew == e64){ \
    VI_PARAMS(e64); \
    BODY; \
  } \
  VI_LOOP_END 

// narrow operation loop
#define VI_VV_LOOP_NARROW(BODY) \
VI_NARROW_CHECK_COMMON; \
VI_LOOP_BASE \
if (sew == e8){ \
  VI_NARROW_SHIFT(e8, e16) \
  BODY; \
}else if(sew == e16){ \
  VI_NARROW_SHIFT(e16, e32) \
  BODY; \
}else if(sew == e32){ \
  VI_NARROW_SHIFT(e32, e64) \
  BODY; \
} \
VI_LOOP_END 

#define VI_NARROW_SHIFT(sew1, sew2) \
  type_usew_t<sew1>::type &vd = P.VU.elt<type_usew_t<sew1>::type>(rd_num, i, true); \
  type_usew_t<sew2>::type vs2_u = P.VU.elt<type_usew_t<sew2>::type>(rs2_num, i); \
  type_usew_t<sew1>::type zimm5 = (type_usew_t<sew1>::type)insn.v_zimm5(); \
  type_sew_t<sew2>::type vs2 = P.VU.elt<type_sew_t<sew2>::type>(rs2_num, i); \
  type_sew_t<sew1>::type vs1 = P.VU.elt<type_sew_t<sew1>::type>(rs1_num, i); \
  type_sew_t<sew1>::type rs1 = (type_sew_t<sew1>::type)RS1; 

#define VI_VVXI_LOOP_NARROW(BODY, is_vs1) \
  VI_CHECK_SDS(is_vs1); \
  VI_LOOP_BASE \
  if (sew == e8){ \
    VI_NARROW_SHIFT(e8, e16) \
    BODY; \
  } else if (sew == e16) { \
    VI_NARROW_SHIFT(e16, e32) \
    BODY; \
  } else if (sew == e32) { \
    VI_NARROW_SHIFT(e32, e64) \
    BODY; \
  } \
  VI_LOOP_END

#define VI_VI_LOOP_NSHIFT(BODY, is_vs1) \
  VI_CHECK_SDS(is_vs1); \
  VI_LOOP_NSHIFT_BASE \
  if (sew == e8){ \
    VI_NSHIFT_PARAMS(e8, e16) \
    BODY; \
  } else if (sew == e16) { \
    VI_NSHIFT_PARAMS(e16, e32) \
    BODY; \
  } else if (sew == e32) { \
    VI_NSHIFT_PARAMS(e32, e64) \
    BODY; \
  } \
  VI_LOOP_END

#define VI_VX_LOOP_NSHIFT(BODY, is_vs1) \
  VI_CHECK_SDS(is_vs1); \
  VI_LOOP_NSHIFT_BASE \
  if (sew == e8){ \
    VX_NSHIFT_PARAMS(e8, e16) \
    BODY; \
  } else if (sew == e16) { \
    VX_NSHIFT_PARAMS(e16, e32) \
    BODY; \
  } else if (sew == e32) { \
    VX_NSHIFT_PARAMS(e32, e64) \
    BODY; \
  } \
  VI_LOOP_END

#define VI_VV_LOOP_NSHIFT(BODY, is_vs1) \
  VI_CHECK_SDS(is_vs1); \
  VI_LOOP_NSHIFT_BASE \
  if (sew == e8){ \
    VV_NSHIFT_PARAMS(e8, e16) \
    BODY; \
  } else if (sew == e16) { \
    VV_NSHIFT_PARAMS(e16, e32) \
    BODY; \
  } else if (sew == e32) { \
    VV_NSHIFT_PARAMS(e32, e64) \
    BODY; \
  } \
  VI_LOOP_END

// widen operation loop
#define VI_VV_LOOP_WIDEN(BODY) \
  VI_LOOP_BASE \
  if (sew == e8){ \
    VV_PARAMS(e8); \
    BODY; \
  }else if(sew == e16){ \
    VV_PARAMS(e16); \
    BODY; \
  }else if(sew == e32){ \
    VV_PARAMS(e32); \
    BODY; \
  } \
  VI_LOOP_END

#define VI_VX_LOOP_WIDEN(BODY) \
  VI_LOOP_BASE \
  if (sew == e8){ \
    VX_PARAMS(e8); \
    BODY; \
  }else if(sew == e16){ \
    VX_PARAMS(e16); \
    BODY; \
  }else if(sew == e32){ \
    VX_PARAMS(e32); \
    BODY; \
  } \
  VI_LOOP_END

#define VI_WIDE_OP_AND_ASSIGN(var0, var1, var2, op0, op1, sign) \
  switch(P.VU.vsew) { \
  case e8: { \
    sign##16_t vd_w = P.VU.elt<sign##16_t>(rd_num, i); \
    P.VU.elt<uint16_t>(rd_num, i, true) = \
      op1((sign##16_t)(sign##8_t)var0 op0 (sign##16_t)(sign##8_t)var1) + var2; \
    } \
    break; \
  case e16: { \
    sign##32_t vd_w = P.VU.elt<sign##32_t>(rd_num, i); \
    P.VU.elt<uint32_t>(rd_num, i, true) = \
      op1((sign##32_t)(sign##16_t)var0 op0 (sign##32_t)(sign##16_t)var1) + var2; \
    } \
    break; \
  default: { \
    sign##64_t vd_w = P.VU.elt<sign##64_t>(rd_num, i); \
    P.VU.elt<uint64_t>(rd_num, i, true) = \
      op1((sign##64_t)(sign##32_t)var0 op0 (sign##64_t)(sign##32_t)var1) + var2; \
    } \
    break; \
  }

#define VI_WIDE_OP_AND_ASSIGN_MIX(var0, var1, var2, op0, op1, sign_d, sign_1, sign_2) \
  switch(P.VU.vsew) { \
  case e8: { \
    sign_d##16_t vd_w = P.VU.elt<sign_d##16_t>(rd_num, i); \
    P.VU.elt<uint16_t>(rd_num, i, true) = \
      op1((sign_1##16_t)(sign_1##8_t)var0 op0 (sign_2##16_t)(sign_2##8_t)var1) + var2; \
    } \
    break; \
  case e16: { \
    sign_d##32_t vd_w = P.VU.elt<sign_d##32_t>(rd_num, i); \
    P.VU.elt<uint32_t>(rd_num, i, true) = \
      op1((sign_1##32_t)(sign_1##16_t)var0 op0 (sign_2##32_t)(sign_2##16_t)var1) + var2; \
    } \
    break; \
  default: { \
    sign_d##64_t vd_w = P.VU.elt<sign_d##64_t>(rd_num, i); \
    P.VU.elt<uint64_t>(rd_num, i, true) = \
      op1((sign_1##64_t)(sign_1##32_t)var0 op0 (sign_2##64_t)(sign_2##32_t)var1) + var2; \
    } \
    break; \
  }

#define VI_WIDE_WVX_OP(var0, op0, sign) \
  switch(P.VU.vsew) { \
  case e8: { \
    sign##16_t &vd_w = P.VU.elt<sign##16_t>(rd_num, i, true); \
    sign##16_t vs2_w = P.VU.elt<sign##16_t>(rs2_num, i); \
    vd_w = vs2_w op0 (sign##16_t)(sign##8_t)var0; \
    } \
    break; \
  case e16: { \
    sign##32_t &vd_w = P.VU.elt<sign##32_t>(rd_num, i, true); \
    sign##32_t vs2_w = P.VU.elt<sign##32_t>(rs2_num, i); \
    vd_w = vs2_w op0 (sign##32_t)(sign##16_t)var0; \
    } \
    break; \
  default: { \
    sign##64_t &vd_w = P.VU.elt<sign##64_t>(rd_num, i, true); \
    sign##64_t vs2_w = P.VU.elt<sign##64_t>(rs2_num, i); \
    vd_w = vs2_w op0 (sign##64_t)(sign##32_t)var0; \
    } \
    break; \
  }

// quad operation loop
#define VI_VV_LOOP_QUAD(BODY) \
  VI_CHECK_QSS(true); \
  VI_LOOP_BASE \
  if (sew == e8){ \
    VV_PARAMS(e8); \
    BODY; \
  }else if(sew == e16){ \
    VV_PARAMS(e16); \
    BODY; \
  } \
  VI_LOOP_END

#define VI_VX_LOOP_QUAD(BODY) \
  VI_CHECK_QSS(false); \
  VI_LOOP_BASE \
  if (sew == e8){ \
    VX_PARAMS(e8); \
    BODY; \
  }else if(sew == e16){ \
    VX_PARAMS(e16); \
    BODY; \
  } \
  VI_LOOP_END

#define VI_QUAD_OP_AND_ASSIGN(var0, var1, var2, op0, op1, sign) \
  switch(P.VU.vsew) { \
  case e8: { \
    sign##32_t vd_w = P.VU.elt<sign##32_t>(rd_num, i); \
    P.VU.elt<uint32_t>(rd_num, i, true) = \
      op1((sign##32_t)(sign##8_t)var0 op0 (sign##32_t)(sign##8_t)var1) + var2; \
    } \
    break; \
  default: { \
    sign##64_t vd_w = P.VU.elt<sign##64_t>(rd_num, i); \
    P.VU.elt<uint64_t>(rd_num, i, true) = \
      op1((sign##64_t)(sign##16_t)var0 op0 (sign##64_t)(sign##16_t)var1) + var2; \
    } \
    break; \
  }

#define VI_QUAD_OP_AND_ASSIGN_MIX(var0, var1, var2, op0, op1, sign_d, sign_1, sign_2) \
  switch(P.VU.vsew) { \
  case e8: { \
    sign_d##32_t vd_w = P.VU.elt<sign_d##32_t>(rd_num, i); \
    P.VU.elt<uint32_t>(rd_num, i, true) = \
      op1((sign_1##32_t)(sign_1##8_t)var0 op0 (sign_2##32_t)(sign_2##8_t)var1) + var2; \
    } \
    break; \
  default: { \
    sign_d##64_t vd_w = P.VU.elt<sign_d##64_t>(rd_num, i); \
    P.VU.elt<uint64_t>(rd_num, i, true) = \
      op1((sign_1##64_t)(sign_1##16_t)var0 op0 (sign_2##64_t)(sign_2##16_t)var1) + var2; \
    } \
    break; \
  }

// wide reduction loop - signed
#define VI_LOOP_WIDE_REDUCTION_BASE(sew1, sew2) \
  reg_t vl = P.VU.vl; \
  reg_t rd_num = insn.rd(); \
  reg_t rs1_num = insn.rs1(); \
  reg_t rs2_num = insn.rs2(); \
  auto &vd_0_des = P.VU.elt<type_sew_t<sew2>::type>(rd_num, 0, true); \
  auto vd_0_res = P.VU.elt<type_sew_t<sew2>::type>(rs1_num, 0); \
  for (reg_t i=P.VU.vstart; i<vl; ++i){ \
    VI_LOOP_ELEMENT_SKIP(); \
    auto vs2 = P.VU.elt<type_sew_t<sew1>::type>(rs2_num, i);

#define WIDE_REDUCTION_LOOP(sew1, sew2, BODY) \
  VI_LOOP_WIDE_REDUCTION_BASE(sew1, sew2) \
  BODY; \
  VI_LOOP_REDUCTION_END(sew2)

#define VI_VV_LOOP_WIDE_REDUCTION(BODY) \
  VI_CHECK_REDUCTION(true); \
  reg_t sew = P.VU.vsew; \
  if (sew == e8){ \
    WIDE_REDUCTION_LOOP(e8, e16, BODY) \
  } else if(sew == e16){ \
    WIDE_REDUCTION_LOOP(e16, e32, BODY) \
  } else if(sew == e32){ \
    WIDE_REDUCTION_LOOP(e32, e64, BODY) \
  }

// wide reduction loop - unsigned
#define VI_ULOOP_WIDE_REDUCTION_BASE(sew1, sew2) \
  reg_t vl = P.VU.vl; \
  reg_t rd_num = insn.rd(); \
  reg_t rs1_num = insn.rs1(); \
  reg_t rs2_num = insn.rs2(); \
  auto &vd_0_des = P.VU.elt<type_usew_t<sew2>::type>(rd_num, 0, true); \
  auto vd_0_res = P.VU.elt<type_usew_t<sew2>::type>(rs1_num, 0); \
  for (reg_t i=P.VU.vstart; i<vl; ++i) { \
    VI_LOOP_ELEMENT_SKIP(); \
    auto vs2 = P.VU.elt<type_usew_t<sew1>::type>(rs2_num, i);

#define WIDE_REDUCTION_ULOOP(sew1, sew2, BODY) \
  VI_ULOOP_WIDE_REDUCTION_BASE(sew1, sew2) \
  BODY; \
  VI_LOOP_REDUCTION_END(sew2)

#define VI_VV_ULOOP_WIDE_REDUCTION(BODY) \
  VI_CHECK_REDUCTION(true); \
  reg_t sew = P.VU.vsew; \
  if (sew == e8){ \
    WIDE_REDUCTION_ULOOP(e8, e16, BODY) \
  } else if(sew == e16){ \
    WIDE_REDUCTION_ULOOP(e16, e32, BODY) \
  } else if(sew == e32){ \
    WIDE_REDUCTION_ULOOP(e32, e64, BODY) \
  }

// carry/borrow bit loop
#define VI_VV_LOOP_CARRY(BODY) \
  VI_CHECK_MSS(true); \
  VI_GENERAL_LOOP_BASE \
  VI_MASK_VARS \
    if (sew == e8){ \
      VV_CARRY_PARAMS(e8) \
      BODY; \
    } else if (sew == e16) { \
      VV_CARRY_PARAMS(e16) \
      BODY; \
    } else if (sew == e32) { \
      VV_CARRY_PARAMS(e32) \
      BODY; \
    } else if (sew == e64) { \
      VV_CARRY_PARAMS(e64) \
      BODY; \
    } \
  VI_LOOP_END

#define VI_XI_LOOP_CARRY(BODY) \
  VI_CHECK_MSS(false); \
  VI_GENERAL_LOOP_BASE \
  VI_MASK_VARS \
    if (sew == e8){ \
      XI_CARRY_PARAMS(e8) \
      BODY; \
    } else if (sew == e16) { \
      XI_CARRY_PARAMS(e16) \
      BODY; \
    } else if (sew == e32) { \
      XI_CARRY_PARAMS(e32) \
      BODY; \
    } else if (sew == e64) { \
      XI_CARRY_PARAMS(e64) \
      BODY; \
    } \
  VI_LOOP_END

#define VI_VV_LOOP_WITH_CARRY(BODY) \
  require(insn.rd() != 0); \
  VI_CHECK_SSS(true); \
  VI_GENERAL_LOOP_BASE \
  VI_MASK_VARS \
    if (sew == e8){ \
      VV_WITH_CARRY_PARAMS(e8) \
      BODY; \
    } else if (sew == e16) { \
      VV_WITH_CARRY_PARAMS(e16) \
      BODY; \
    } else if (sew == e32) { \
      VV_WITH_CARRY_PARAMS(e32) \
      BODY; \
    } else if (sew == e64) { \
      VV_WITH_CARRY_PARAMS(e64) \
      BODY; \
    } \
  VI_LOOP_END

#define VI_XI_LOOP_WITH_CARRY(BODY) \
  require(insn.rd() != 0); \
  VI_CHECK_SSS(false); \
  VI_GENERAL_LOOP_BASE \
  VI_MASK_VARS \
    if (sew == e8){ \
      XI_WITH_CARRY_PARAMS(e8) \
      BODY; \
    } else if (sew == e16) { \
      XI_WITH_CARRY_PARAMS(e16) \
      BODY; \
    } else if (sew == e32) { \
      XI_WITH_CARRY_PARAMS(e32) \
      BODY; \
    } else if (sew == e64) { \
      XI_WITH_CARRY_PARAMS(e64) \
      BODY; \
    } \
  VI_LOOP_END

// average loop
#define VI_VVX_LOOP_AVG(opd, op, is_vs1) \
VI_CHECK_SSS(is_vs1); \
VRM xrm = p->VU.get_vround_mode(); \
VI_LOOP_BASE \
  switch(sew) { \
    case e8: { \
     VV_PARAMS(e8); \
     type_sew_t<e8>::type rs1 = RS1; \
     auto res = (int32_t)vs2 op opd; \
     INT_ROUNDING(res, xrm, 1); \
     vd = res >> 1; \
     break; \
    } \
    case e16: { \
     VV_PARAMS(e16); \
     type_sew_t<e16>::type rs1 = RS1; \
     auto res = (int32_t)vs2 op opd; \
     INT_ROUNDING(res, xrm, 1); \
     vd = res >> 1; \
     break; \
    } \
    case e32: { \
     VV_PARAMS(e32); \
     type_sew_t<e32>::type rs1 = RS1; \
     auto res = (int64_t)vs2 op opd; \
     INT_ROUNDING(res, xrm, 1); \
     vd = res >> 1; \
     break; \
    } \
    default: { \
     VV_PARAMS(e64); \
     type_sew_t<e64>::type rs1 = RS1; \
     auto res = (int128_t)vs2 op opd; \
     INT_ROUNDING(res, xrm, 1); \
     vd = res >> 1; \
     break; \
    } \
  } \
VI_LOOP_END

#define VI_VVX_ULOOP_AVG(opd, op, is_vs1) \
VI_CHECK_SSS(is_vs1); \
VRM xrm = p->VU.get_vround_mode(); \
VI_LOOP_BASE \
  switch(sew) { \
    case e8: { \
     VV_U_PARAMS(e8); \
     type_usew_t<e8>::type rs1 = RS1; \
     auto res = (uint16_t)vs2 op opd; \
     INT_ROUNDING(res, xrm, 1); \
     vd = res >> 1; \
     break; \
    } \
    case e16: { \
     VV_U_PARAMS(e16); \
     type_usew_t<e16>::type rs1 = RS1; \
     auto res = (uint32_t)vs2 op opd; \
     INT_ROUNDING(res, xrm, 1); \
     vd = res >> 1; \
     break; \
    } \
    case e32: { \
     VV_U_PARAMS(e32); \
     type_usew_t<e32>::type rs1 = RS1; \
     auto res = (uint64_t)vs2 op opd; \
     INT_ROUNDING(res, xrm, 1); \
     vd = res >> 1; \
     break; \
    } \
    default: { \
     VV_U_PARAMS(e64); \
     type_usew_t<e64>::type rs1 = RS1; \
     auto res = (uint128_t)vs2 op opd; \
     INT_ROUNDING(res, xrm, 1); \
     vd = res >> 1; \
     break; \
    } \
  } \
VI_LOOP_END

//
// vector: load/store helper 
//
#define VI_STRIP(inx) \
  reg_t vreg_inx = inx;

#define VI_DUPLICATE_VREG(reg_num, idx_sew) \
reg_t index[P.VU.vlmax]; \
for (reg_t i = 0; i < P.VU.vlmax && P.VU.vl != 0; ++i) { \
  switch(idx_sew) { \
    case e8: \
      index[i] = P.VU.elt<uint8_t>(reg_num, i); \
      break; \
    case e16: \
      index[i] = P.VU.elt<uint16_t>(reg_num, i); \
      break; \
    case e32: \
      index[i] = P.VU.elt<uint32_t>(reg_num, i); \
      break; \
    case e64: \
      index[i] = P.VU.elt<uint64_t>(reg_num, i); \
      break; \
  } \
}

#define VI_LD_INT4_EXT(stride, offset, elt_width, is_mask_ldst, is_zero) \
	const reg_t nf = insn.v_nf() + 1; \
	const reg_t vl = is_mask_ldst ? ((P.VU.vl + 7) / 8) : P.VU.vl; \
	const reg_t baseAddr = RS1; \
	const reg_t vd = insn.rd(); \
  const reg_t elt_width_bit_half = sizeof(elt_width##_t) * 4;   \
	const reg_t elt_width_bit_val = pow( 2, (sizeof(elt_width##_t) * 4 )) - 1; \
	VI_CHECK_LOAD(elt_width, is_mask_ldst); \
  for (reg_t i = 0; i < vl; ++i) { \
	  VI_ELEMENT_SKIP(i); \
	  VI_STRIP(i); \
	  P.VU.vstart = i; \
	  for (reg_t fn = 0; fn < nf; ++fn) { \
		  elt_width##_t val = MMU.load_##elt_width( \
		    baseAddr + (stride) + (offset) * sizeof(elt_width##_t)); \
		  if (i % 2 ) { \
			  val = (val >> elt_width_bit_half) & elt_width_bit_val ;\
		  } \
		  else { \
			  val = val & elt_width_bit_val;\
		  }\
      if (is_zero && val >> (elt_width_bit_half - 1)){ \
			  val = (elt_width_bit_val << elt_width_bit_half) | val; \
		  } \
		  P.VU.elt<elt_width##_t>(vd + fn * emul, vreg_inx, true) = val; \
	  } \
	} \
	P.VU.vstart = 0;

#define VI_LD(stride, offset, elt_width, is_mask_ldst) \
  const reg_t nf = insn.v_nf() + 1; \
  const reg_t vl = is_mask_ldst ? ((P.VU.vl + 7) / 8) : P.VU.vl; \
  const reg_t baseAddr = RS1; \
  const reg_t vd = insn.rd(); \
  VI_CHECK_LOAD(elt_width, is_mask_ldst); \
  for (reg_t i = 0; i < vl; ++i) { \
    VI_ELEMENT_SKIP(i); \
    VI_STRIP(i); \
    P.VU.vstart = i; \
    for (reg_t fn = 0; fn < nf; ++fn) { \
      elt_width##_t val = MMU.load_##elt_width( \
        baseAddr + (stride) + (offset) * sizeof(elt_width##_t)); \
      P.VU.elt<elt_width##_t>(vd + fn * emul, vreg_inx, true) = val; \
    } \
  } \
  P.VU.vstart = 0;

#define VI_LD_PI(stride, offset, elt_width, is_mask_ldst) \
  const reg_t nf = insn.v_nf() + 1; \
  const reg_t vl = is_mask_ldst ? ((P.VU.vl + 7) / 8) : P.VU.vl; \
  const reg_t baseAddr = RS1; \
  const reg_t vd = insn.rd(); \
  WRITE_RS1(RS1 + vl*sizeof(elt_width##_t));\
  VI_CHECK_LOAD(elt_width, is_mask_ldst); \
  WRITE_RS1(RS1 - vl*sizeof(elt_width##_t));\
  for (reg_t i = 0; i < vl; ++i) { \
    VI_ELEMENT_SKIP(i); \
    VI_STRIP(i); \
    P.VU.vstart = i; \
    for (reg_t fn = 0; fn < nf; ++fn) { \
      elt_width##_t val = MMU.load_##elt_width( \
        baseAddr + (stride) + (offset) * sizeof(elt_width##_t)); \
      P.VU.elt<elt_width##_t>(vd + fn * emul, vreg_inx, true) = val; \
    } \
  } \
  P.VU.vstart = 0;

#define VI_LD_INDEX(elt_width, is_seg) \
  const reg_t nf = insn.v_nf() + 1; \
  const reg_t vl = P.VU.vl; \
  const reg_t baseAddr = RS1; \
  const reg_t vd = insn.rd(); \
  if (!is_seg) \
    require(nf == 1); \
  VI_CHECK_LD_INDEX(elt_width); \
  VI_DUPLICATE_VREG(insn.rs2(), elt_width); \
  for (reg_t i = 0; i < vl; ++i) { \
    VI_ELEMENT_SKIP(i); \
    VI_STRIP(i); \
    P.VU.vstart = i; \
    for (reg_t fn = 0; fn < nf; ++fn) { \
      switch(P.VU.vsew){ \
        case e8: \
          P.VU.elt<uint8_t>(vd + fn * flmul, vreg_inx, true) = \
            MMU.load_uint8(baseAddr + index[i] + fn * 1); \
          break; \
        case e16: \
          P.VU.elt<uint16_t>(vd + fn * flmul, vreg_inx, true) = \
            MMU.load_uint16(baseAddr + index[i] + fn * 2); \
          break; \
        case e32: \
          P.VU.elt<uint32_t>(vd + fn * flmul, vreg_inx, true) = \
            MMU.load_uint32(baseAddr + index[i] + fn * 4); \
          break; \
        default: \
          P.VU.elt<uint64_t>(vd + fn * flmul, vreg_inx, true) = \
            MMU.load_uint64(baseAddr + index[i] + fn * 8); \
          break; \
      } \
    } \
  } \
  P.VU.vstart = 0;

#define VI_ST(stride, offset, elt_width, is_mask_ldst) \
  const reg_t nf = insn.v_nf() + 1; \
  const reg_t vl = is_mask_ldst ? ((P.VU.vl + 7) / 8) : P.VU.vl; \
  const reg_t baseAddr = RS1; \
  const reg_t vs3 = insn.rd(); \
  VI_CHECK_STORE(elt_width, is_mask_ldst); \
  for (reg_t i = 0; i < vl; ++i) { \
    VI_STRIP(i) \
    VI_ELEMENT_SKIP(i); \
    P.VU.vstart = i; \
    for (reg_t fn = 0; fn < nf; ++fn) { \
      elt_width##_t val = P.VU.elt<elt_width##_t>(vs3 + fn * emul, vreg_inx); \
      MMU.store_##elt_width( \
        baseAddr + (stride) + (offset) * sizeof(elt_width##_t), val); \
    } \
  } \
  P.VU.vstart = 0;

#define VI_ST_PI(stride, offset, elt_width, is_mask_ldst) \
  const reg_t nf = insn.v_nf() + 1; \
  const reg_t vl = is_mask_ldst ? ((P.VU.vl + 7) / 8) : P.VU.vl; \
  const reg_t baseAddr = RS1; \
  const reg_t vs3 = insn.rd(); \
  WRITE_RS1(RS1 + vl*sizeof(elt_width##_t));\
  VI_CHECK_STORE(elt_width, is_mask_ldst); \
  WRITE_RS1(RS1 - vl*sizeof(elt_width##_t));\
  for (reg_t i = 0; i < vl; ++i) { \
    VI_STRIP(i) \
    VI_ELEMENT_SKIP(i); \
    P.VU.vstart = i; \
    for (reg_t fn = 0; fn < nf; ++fn) { \
      elt_width##_t val = P.VU.elt<elt_width##_t>(vs3 + fn * emul, vreg_inx); \
      MMU.store_##elt_width( \
        baseAddr + (stride) + (offset) * sizeof(elt_width##_t), val); \
    } \
  } \
  P.VU.vstart = 0;

#define VI_ST_INDEX(elt_width, is_seg) \
  const reg_t nf = insn.v_nf() + 1; \
  const reg_t vl = P.VU.vl; \
  const reg_t baseAddr = RS1; \
  const reg_t vs3 = insn.rd(); \
  if (!is_seg) \
    require(nf == 1); \
  VI_CHECK_ST_INDEX(elt_width); \
  VI_DUPLICATE_VREG(insn.rs2(), elt_width);   \
  for (reg_t i = 0; i < vl; ++i) { \
    VI_STRIP(i) \
    VI_ELEMENT_SKIP(i); \
    P.VU.vstart = i; \
    for (reg_t fn = 0; fn < nf; ++fn) { \
      switch (P.VU.vsew) { \
      case e8: \
        MMU.store_uint8(baseAddr + index[i] + fn * 1, \
          P.VU.elt<uint8_t>(vs3 + fn * flmul, vreg_inx)); \
        break; \
      case e16: \
        MMU.store_uint16(baseAddr + index[i] + fn * 2, \
          P.VU.elt<uint16_t>(vs3 + fn * flmul, vreg_inx)); \
        break; \
      case e32: \
        MMU.store_uint32(baseAddr + index[i] + fn * 4, \
          P.VU.elt<uint32_t>(vs3 + fn * flmul, vreg_inx)); \
        break; \
      default: \
        MMU.store_uint64(baseAddr + index[i] + fn * 8, \
          P.VU.elt<uint64_t>(vs3 + fn * flmul, vreg_inx)); \
        break; \
      } \
    } \
  } \
  P.VU.vstart = 0;

#define VI_LDST_FF(elt_width) \
  const reg_t nf = insn.v_nf() + 1; \
  const reg_t sew = p->VU.vsew; \
  const reg_t vl = p->VU.vl; \
  const reg_t baseAddr = RS1; \
  const reg_t rd_num = insn.rd(); \
  VI_CHECK_LOAD(elt_width, false); \
  bool early_stop = false; \
  for (reg_t i = p->VU.vstart; i < vl; ++i) { \
    VI_STRIP(i); \
    VI_ELEMENT_SKIP(i); \
    \
    for (reg_t fn = 0; fn < nf; ++fn) { \
      uint64_t val; \
      try { \
        val = MMU.load_##elt_width( \
          baseAddr + (i * nf + fn) * sizeof(elt_width##_t)); \
      } catch (trap_t& t) { \
        if (i == 0) \
          throw; /* Only take exception on zeroth element */ \
        /* Reduce VL if an exception occurs on a later element */ \
        early_stop = true; \
        P.VU.vl = i; \
        break; \
      } \
      p->VU.elt<elt_width##_t>(rd_num + fn * emul, vreg_inx, true) = val; \
    } \
    \
    if (early_stop) { \
      break; \
    } \
  } \
  p->VU.vstart = 0;

#define VI_LD_WHOLE(elt_width) \
  require_vector_novtype(true, false); \
  const reg_t baseAddr = RS1; \
  const reg_t vd = insn.rd(); \
  const reg_t len = insn.v_nf() + 1; \
  require_align(vd, len); \
  const reg_t elt_per_reg = P.VU.vlenb / sizeof(elt_width ## _t); \
  const reg_t size = len * elt_per_reg; \
  if (P.VU.vstart < size) { \
    reg_t i = P.VU.vstart / elt_per_reg; \
    reg_t off = P.VU.vstart % elt_per_reg; \
    if (off) { \
      for (reg_t pos = off; pos < elt_per_reg; ++pos) { \
        auto val = MMU.load_## elt_width(baseAddr + \
          P.VU.vstart * sizeof(elt_width ## _t)); \
        P.VU.elt<elt_width ## _t>(vd + i, pos, true) = val; \
        P.VU.vstart++; \
      } \
      ++i; \
    } \
    for (; i < len; ++i) { \
      for (reg_t pos = 0; pos < elt_per_reg; ++pos) { \
        auto val = MMU.load_## elt_width(baseAddr + \
          P.VU.vstart * sizeof(elt_width ## _t)); \
        P.VU.elt<elt_width ## _t>(vd + i, pos, true) = val; \
        P.VU.vstart++; \
      } \
    } \
  } \
  P.VU.vstart = 0; \

#define VI_ST_WHOLE \
  require_vector_novtype(true, false); \
  const reg_t baseAddr = RS1; \
  const reg_t vs3 = insn.rd(); \
  const reg_t len = insn.v_nf() + 1; \
  require_align(vs3, len); \
  const reg_t size = len * P.VU.vlenb; \
   \
  if (P.VU.vstart < size) { \
    reg_t i = P.VU.vstart / P.VU.vlenb; \
    reg_t off = P.VU.vstart % P.VU.vlenb; \
    if (off) { \
      for (reg_t pos = off; pos < P.VU.vlenb; ++pos) { \
        auto val = P.VU.elt<uint8_t>(vs3 + i, pos); \
        MMU.store_uint8(baseAddr + P.VU.vstart, val); \
        P.VU.vstart++; \
      } \
      i++; \
    } \
    for (; i < len; ++i) { \
      for (reg_t pos = 0; pos < P.VU.vlenb; ++pos) { \
        auto val = P.VU.elt<uint8_t>(vs3 + i, pos); \
        MMU.store_uint8(baseAddr + P.VU.vstart, val); \
        P.VU.vstart++; \
      } \
    } \
  } \
  P.VU.vstart = 0;

//
// vector: amo 
//
#define VI_AMO(op, type, idx_type) \
  require_vector(false); \
  require_align(insn.rd(), P.VU.vflmul); \
  require(P.VU.vsew <= P.get_xlen() && P.VU.vsew >= 32); \
  require_align(insn.rd(), P.VU.vflmul); \
  float vemul = ((float)idx_type / P.VU.vsew * P.VU.vflmul); \
  require(vemul >= 0.125 && vemul <= 8); \
  require_align(insn.rs2(), vemul); \
  if (insn.v_wd()) {\
    require_vm; \
    if (idx_type > P.VU.vsew) { \
      if (insn.rd() != insn.rs2()) \
        require_noover(insn.rd(), P.VU.vflmul, insn.rs2(), vemul); \
    } else if (idx_type < P.VU.vsew) { \
      if (vemul < 1) {\
        require_noover(insn.rd(), P.VU.vflmul, insn.rs2(), vemul); \
      } else {\
        require_noover_widen(insn.rd(), P.VU.vflmul, insn.rs2(), vemul); \
      } \
    } \
  } \
  VI_DUPLICATE_VREG(insn.rs2(), idx_type); \
  const reg_t vl = P.VU.vl; \
  const reg_t baseAddr = RS1; \
  const reg_t vd = insn.rd(); \
  for (reg_t i = P.VU.vstart; i < vl; ++i) { \
    VI_ELEMENT_SKIP(i); \
    VI_STRIP(i); \
    P.VU.vstart = i; \
    switch (P.VU.vsew) { \
    case e32: {\
      auto vs3 = P.VU.elt< type ## 32_t>(vd, vreg_inx); \
      auto val = MMU.amo_uint32(baseAddr + index[i], [&]( type ## 32_t lhs) { op }); \
      if (insn.v_wd()) \
        P.VU.elt< type ## 32_t>(vd, vreg_inx, true) = val; \
      } \
      break; \
    case e64: {\
      auto vs3 = P.VU.elt< type ## 64_t>(vd, vreg_inx); \
      auto val = MMU.amo_uint64(baseAddr + index[i], [&]( type ## 64_t lhs) { op }); \
      if (insn.v_wd()) \
        P.VU.elt< type ## 64_t>(vd, vreg_inx, true) = val; \
      } \
      break; \
    default: \
      require(0); \
      break; \
    } \
  } \
  P.VU.vstart = 0;

// vector: sign/unsiged extension
#define VI_VV_EXT(div, type) \
  require(insn.rd() != insn.rs2()); \
  require_vm; \
  reg_t from = P.VU.vsew / div; \
  require(from >= e8 && from <= e64); \
  require(((float)P.VU.vflmul / div) >= 0.125 && ((float)P.VU.vflmul / div) <= 8 ); \
  require_align(insn.rd(), P.VU.vflmul); \
  require_align(insn.rs2(), P.VU.vflmul / div); \
  if ((P.VU.vflmul / div) < 1) { \
    require_noover(insn.rd(), P.VU.vflmul, insn.rs2(), P.VU.vflmul / div); \
  } else {\
    require_noover_widen(insn.rd(), P.VU.vflmul, insn.rs2(), P.VU.vflmul / div); \
  } \
  reg_t pat = (((P.VU.vsew >> 3) << 4) | from >> 3); \
  VI_GENERAL_LOOP_BASE \
  VI_LOOP_ELEMENT_SKIP(); \
    switch (pat) { \
      case 0x21: \
        P.VU.elt<type##16_t>(rd_num, i, true) = P.VU.elt<type##8_t>(rs2_num, i); \
        break; \
      case 0x41: \
        P.VU.elt<type##32_t>(rd_num, i, true) = P.VU.elt<type##8_t>(rs2_num, i); \
        break; \
      case 0x81: \
        P.VU.elt<type##64_t>(rd_num, i, true) = P.VU.elt<type##8_t>(rs2_num, i); \
        break; \
      case 0x42: \
        P.VU.elt<type##32_t>(rd_num, i, true) = P.VU.elt<type##16_t>(rs2_num, i); \
        break; \
      case 0x82: \
        P.VU.elt<type##64_t>(rd_num, i, true) = P.VU.elt<type##16_t>(rs2_num, i); \
        break; \
      case 0x84: \
        P.VU.elt<type##64_t>(rd_num, i, true) = P.VU.elt<type##32_t>(rs2_num, i); \
        break; \
      case 0x88: \
        P.VU.elt<type##64_t>(rd_num, i, true) = P.VU.elt<type##32_t>(rs2_num, i); \
        break; \
      default: \
        break; \
    } \
  VI_LOOP_END 

//
// vector: vfp helper
//
#define VI_VFP_COMMON \
  require_fp; \
  require((P.VU.vsew == e16 && p->supports_extension(EXT_ZFH)) || \
          (P.VU.vsew == e32 && p->supports_extension('F')) || \
          (P.VU.vsew == e64 && p->supports_extension('D'))); \
  require_vector(true);\
  require(STATE.frm < 0x5);\
  reg_t vl = P.VU.vl; \
  reg_t rd_num = insn.rd(); \
  reg_t rs1_num = insn.rs1(); \
  reg_t rs2_num = insn.rs2(); \
  softfloat_roundingMode = STATE.frm;

#define VI_VFP_LOOP_BASE \
  VI_VFP_COMMON \
  for (reg_t i=P.VU.vstart; i<vl; ++i){ \
    VI_LOOP_ELEMENT_SKIP();

#define VI_VFP_LOOP_CMP_BASE \
  VI_VFP_COMMON \
  for (reg_t i = P.VU.vstart; i < vl; ++i) { \
    VI_LOOP_ELEMENT_SKIP(); \
    uint64_t mmask = UINT64_C(1) << mpos; \
    uint64_t &vdi = P.VU.elt<uint64_t>(rd_num, midx, true); \
    uint64_t res = 0;

#define VI_VFP_LOOP_REDUCTION_BASE(width) \
  float##width##_t vd_0 = P.VU.elt<float##width##_t>(rd_num, 0); \
  float##width##_t vs1_0 = P.VU.elt<float##width##_t>(rs1_num, 0); \
  vd_0 = vs1_0; \
  bool is_active = false; \
  for (reg_t i=P.VU.vstart; i<vl; ++i){ \
    VI_LOOP_ELEMENT_SKIP(); \
    float##width##_t vs2 = P.VU.elt<float##width##_t>(rs2_num, i); \
    is_active = true; \

#define VI_VFP_LOOP_REDUCTION_BASE_BF(width) \
  bfloat##width##_t vd_0 = P.VU.elt<bfloat##width##_t>(rd_num, 0); \
  bfloat##width##_t vs1_0 = P.VU.elt<bfloat##width##_t>(rs1_num, 0); \
  vd_0 = vs1_0; \
  bool is_active = false; \
  for (reg_t i=P.VU.vstart; i<vl; ++i){ \
    VI_LOOP_ELEMENT_SKIP(); \
    bfloat##width##_t vs2 = P.VU.elt<bfloat##width##_t>(rs2_num, i); \
    is_active = true; \

#define VI_VFP_LOOP_WIDE_REDUCTION_BASE \
  VI_VFP_COMMON \
  float64_t vd_0 = f64(P.VU.elt<float64_t>(rs1_num, 0).v); \
  for (reg_t i=P.VU.vstart; i<vl; ++i) { \
    VI_LOOP_ELEMENT_SKIP();

#define VI_VFP_LOOP_END \
  } \
  P.VU.vstart = 0; \

#define VI_VFP_LOOP_REDUCTION_END(x) \
  } \
  P.VU.vstart = 0; \
  if (vl > 0) { \
    if (is_propagate && !is_active) { \
      switch (x) { \
        case e16: {\
            if (STATE.bf16) {\
              auto ret = bf16_classify(bf16(vd_0.v)); \
              if (ret & 0x300) { \
                if (ret & 0x100) { \
                  softfloat_exceptionFlags |= softfloat_flag_invalid; \
                  set_fp_exceptions; \
                } \
                P.VU.elt<uint16_t>(rd_num, 0, true) = defaultNaNBF16UI; \
              } else { \
                P.VU.elt<uint16_t>(rd_num, 0, true) = vd_0.v; \
              } \
            } else {\
              auto ret = f16_classify(f16(vd_0.v)); \
              if (ret & 0x300) { \
                if (ret & 0x100) { \
                  softfloat_exceptionFlags |= softfloat_flag_invalid; \
                  set_fp_exceptions; \
                } \
                P.VU.elt<uint16_t>(rd_num, 0, true) = defaultNaNF16UI; \
              } else { \
                P.VU.elt<uint16_t>(rd_num, 0, true) = vd_0.v; \
              } \
            }\
          } \
          break; \
        case e32: { \
            auto ret = f32_classify(f32(vd_0.v)); \
            if (ret & 0x300) { \
              if (ret & 0x100) { \
                softfloat_exceptionFlags |= softfloat_flag_invalid; \
                set_fp_exceptions; \
              } \
              P.VU.elt<uint32_t>(rd_num, 0, true) = defaultNaNF32UI; \
            } else { \
              P.VU.elt<uint32_t>(rd_num, 0, true) = vd_0.v; \
            } \
          } \
          break; \
        case e64: {\
            auto ret = f64_classify(f64(vd_0.v)); \
            if (ret & 0x300) { \
              if (ret & 0x100) { \
                softfloat_exceptionFlags |= softfloat_flag_invalid; \
                set_fp_exceptions; \
              } \
              P.VU.elt<uint64_t>(rd_num, 0, true) = defaultNaNF64UI; \
            } else { \
              P.VU.elt<uint64_t>(rd_num, 0, true) = vd_0.v; \
            } \
          } \
          break; \
      } \
    } else { \
      P.VU.elt<type_sew_t<x>::type>(rd_num, 0, true) = vd_0.v; \
    } \
  }

#define VI_VFP_LOOP_CMP_END \
  switch(P.VU.vsew) { \
    case e16: \
    case e32: \
    case e64: { \
      vdi = (vdi & ~mmask) | (((res) << mpos) & mmask); \
      break; \
    } \
    default: \
      require(0); \
      break; \
    }; \
  } \
  P.VU.vstart = 0;

#define VI_VFP_VV_LOOP(BODY16B, BODY16, BODY32, BODY64) \
  VI_CHECK_SSS(true); \
  VI_VFP_LOOP_BASE \
  switch(P.VU.vsew) { \
    case e16: {\
      if (STATE.bf16) {\
        bfloat16_t &vd = P.VU.elt<bfloat16_t>(rd_num, i, true); \
        bfloat16_t vs1 = P.VU.elt<bfloat16_t>(rs1_num, i); \
        bfloat16_t vs2 = P.VU.elt<bfloat16_t>(rs2_num, i); \
        BODY16B; \
      } else {\
        float16_t &vd = P.VU.elt<float16_t>(rd_num, i, true); \
        float16_t vs1 = P.VU.elt<float16_t>(rs1_num, i); \
        float16_t vs2 = P.VU.elt<float16_t>(rs2_num, i); \
        BODY16; \
      }\
      set_fp_exceptions; \
      break; \
    }\
    case e32: {\
      float32_t &vd = P.VU.elt<float32_t>(rd_num, i, true); \
      float32_t vs1 = P.VU.elt<float32_t>(rs1_num, i); \
      float32_t vs2 = P.VU.elt<float32_t>(rs2_num, i); \
      BODY32; \
      set_fp_exceptions; \
      break; \
    }\
    case e64: {\
      float64_t &vd = P.VU.elt<float64_t>(rd_num, i, true); \
      float64_t vs1 = P.VU.elt<float64_t>(rs1_num, i); \
      float64_t vs2 = P.VU.elt<float64_t>(rs2_num, i); \
      BODY64; \
      set_fp_exceptions; \
      break; \
    }\
    default: \
      require(0); \
      break; \
  }; \
  DEBUG_RVV_FP_VV; \
  VI_VFP_LOOP_END

#define VI_VFP_V_LOOP(BODY16B, BODY16, BODY32, BODY64) \
  VI_CHECK_SSS(false); \
  VI_VFP_LOOP_BASE \
  switch(P.VU.vsew) { \
    case e16: {\
      if (STATE.bf16) {\
        bfloat16_t &vd = P.VU.elt<bfloat16_t>(rd_num, i, true); \
        bfloat16_t vs2 = P.VU.elt<bfloat16_t>(rs2_num, i); \
        BODY16B; \
      } else {\
        float16_t &vd = P.VU.elt<float16_t>(rd_num, i, true); \
        float16_t vs2 = P.VU.elt<float16_t>(rs2_num, i); \
        BODY16; \
      }\
      break; \
    }\
    case e32: {\
      float32_t &vd = P.VU.elt<float32_t>(rd_num, i, true); \
      float32_t vs2 = P.VU.elt<float32_t>(rs2_num, i); \
      BODY32; \
      break; \
    }\
    case e64: {\
      float64_t &vd = P.VU.elt<float64_t>(rd_num, i, true); \
      float64_t vs2 = P.VU.elt<float64_t>(rs2_num, i); \
      BODY64; \
      break; \
    }\
    default: \
      require(0); \
      break; \
  }; \
  set_fp_exceptions; \
  VI_VFP_LOOP_END

#define VI_VFP_VV_LOOP_REDUCTION(BODY16B, BODY16, BODY32, BODY64) \
  VI_CHECK_REDUCTION(false) \
  VI_VFP_COMMON \
  switch(P.VU.vsew) { \
    case e16: {\
      if (STATE.bf16) {\
        VI_VFP_LOOP_REDUCTION_BASE_BF(16) \
          BODY16B; \
          set_fp_exceptions; \
        VI_VFP_LOOP_REDUCTION_END(e16) \
      } else {\
        VI_VFP_LOOP_REDUCTION_BASE(16) \
          BODY16; \
          set_fp_exceptions; \
        VI_VFP_LOOP_REDUCTION_END(e16) \
      }\
      break; \
    }\
    case e32: {\
      VI_VFP_LOOP_REDUCTION_BASE(32) \
        BODY32; \
        set_fp_exceptions; \
      VI_VFP_LOOP_REDUCTION_END(e32) \
      break; \
    }\
    case e64: {\
      VI_VFP_LOOP_REDUCTION_BASE(64) \
        BODY64; \
        set_fp_exceptions; \
      VI_VFP_LOOP_REDUCTION_END(e64) \
      break; \
    }\
    default: \
      require(0); \
      break; \
  }; \

#define VI_VFP_VV_LOOP_WIDE_REDUCTION(BODY16, BODY32) \
  VI_CHECK_REDUCTION(true) \
  VI_VFP_COMMON \
  require((P.VU.vsew == e16 && p->supports_extension('F')) || \
          (P.VU.vsew == e32 && p->supports_extension('D'))); \
  bool is_active = false; \
  switch(P.VU.vsew) { \
    case e16: {\
      float32_t vd_0 = P.VU.elt<float32_t>(rs1_num, 0); \
      for (reg_t i=P.VU.vstart; i<vl; ++i) { \
        VI_LOOP_ELEMENT_SKIP(); \
        is_active = true; \
        if (STATE.bf16) {\
          float32_t vs2 = bf16_to_f32(P.VU.elt<bfloat16_t>(rs2_num, i)); \
          BODY16; \
        } else {\
          float32_t vs2 = f16_to_f32(P.VU.elt<float16_t>(rs2_num, i)); \
          BODY16; \
        }\
        set_fp_exceptions; \
      VI_VFP_LOOP_REDUCTION_END(e32) \
      break; \
    }\
    case e32: {\
      float64_t vd_0 = P.VU.elt<float64_t>(rs1_num, 0); \
      for (reg_t i=P.VU.vstart; i<vl; ++i) { \
        VI_LOOP_ELEMENT_SKIP(); \
        is_active = true; \
        float64_t vs2 = f32_to_f64(P.VU.elt<float32_t>(rs2_num, i)); \
        BODY32; \
        set_fp_exceptions; \
      VI_VFP_LOOP_REDUCTION_END(e64) \
      break; \
    }\
    default: \
      require(0); \
      break; \
  }; \

#define VI_VFP_VF_LOOP(BODY16B, BODY16, BODY32, BODY64) \
  VI_CHECK_SSS(false); \
  VI_VFP_LOOP_BASE \
  switch(P.VU.vsew) { \
    case e16: {\
      if (STATE.bf16) {\
        bfloat16_t &vd = P.VU.elt<bfloat16_t>(rd_num, i, true); \
        bfloat16_t rs1 = bf16(READ_FREG(rs1_num)); \
        bfloat16_t vs2 = P.VU.elt<bfloat16_t>(rs2_num, i); \
        BODY16B; \
      } else {\
        float16_t &vd = P.VU.elt<float16_t>(rd_num, i, true); \
        float16_t rs1 = f16(READ_FREG(rs1_num)); \
        float16_t vs2 = P.VU.elt<float16_t>(rs2_num, i); \
        BODY16; \
      }\
      set_fp_exceptions; \
      break; \
    }\
    case e32: {\
      float32_t &vd = P.VU.elt<float32_t>(rd_num, i, true); \
      float32_t rs1 = f32(READ_FREG(rs1_num)); \
      float32_t vs2 = P.VU.elt<float32_t>(rs2_num, i); \
      BODY32; \
      set_fp_exceptions; \
      break; \
    }\
    case e64: {\
      float64_t &vd = P.VU.elt<float64_t>(rd_num, i, true); \
      float64_t rs1 = f64(READ_FREG(rs1_num)); \
      float64_t vs2 = P.VU.elt<float64_t>(rs2_num, i); \
      BODY64; \
      set_fp_exceptions; \
      break; \
    }\
    default: \
      require(0); \
      break; \
  }; \
  DEBUG_RVV_FP_VF; \
  VI_VFP_LOOP_END

#define VI_VFP_LOOP_CMP(BODY16B, BODY16, BODY32, BODY64, is_vs1) \
  VI_CHECK_MSS(is_vs1); \
  VI_VFP_LOOP_CMP_BASE \
  switch(P.VU.vsew) { \
    case e16: {\
      if (STATE.bf16) {\
        bfloat16_t vs2 = P.VU.elt<bfloat16_t>(rs2_num, i); \
        bfloat16_t vs1 = P.VU.elt<bfloat16_t>(rs1_num, i); \
        bfloat16_t rs1 = bf16(READ_FREG(rs1_num)); \
        BODY16B; \
      } else {\
        float16_t vs2 = P.VU.elt<float16_t>(rs2_num, i); \
        float16_t vs1 = P.VU.elt<float16_t>(rs1_num, i); \
        float16_t rs1 = f16(READ_FREG(rs1_num)); \
        BODY16; \
      }\
      set_fp_exceptions; \
      break; \
    }\
    case e32: {\
      float32_t vs2 = P.VU.elt<float32_t>(rs2_num, i); \
      float32_t vs1 = P.VU.elt<float32_t>(rs1_num, i); \
      float32_t rs1 = f32(READ_FREG(rs1_num)); \
      BODY32; \
      set_fp_exceptions; \
      break; \
    }\
    case e64: {\
      float64_t vs2 = P.VU.elt<float64_t>(rs2_num, i); \
      float64_t vs1 = P.VU.elt<float64_t>(rs1_num, i); \
      float64_t rs1 = f64(READ_FREG(rs1_num)); \
      BODY64; \
      set_fp_exceptions; \
      break; \
    }\
    default: \
      require(0); \
      break; \
  }; \
  VI_VFP_LOOP_CMP_END \

#define VI_VFP_VF_LOOP_WIDE(BODY16, BODY32) \
  VI_CHECK_DSS(false); \
  VI_VFP_LOOP_BASE \
  switch(P.VU.vsew) { \
    case e16: { \
      float32_t &vd = P.VU.elt<float32_t>(rd_num, i, true); \
      if (STATE.bf16) {\
        float32_t vs2 = bf16_to_f32(P.VU.elt<bfloat16_t>(rs2_num, i)); \
        float32_t rs1 = bf16_to_f32(bf16(READ_FREG(rs1_num))); \
        BODY16; \
      } else {\
        float32_t vs2 = f16_to_f32(P.VU.elt<float16_t>(rs2_num, i)); \
        float32_t rs1 = f16_to_f32(f16(READ_FREG(rs1_num))); \
        BODY16; \
      }\
      set_fp_exceptions; \
      break; \
    } \
    case e32: {\
      float64_t &vd = P.VU.elt<float64_t>(rd_num, i, true); \
      float64_t vs2 = f32_to_f64(P.VU.elt<float32_t>(rs2_num, i)); \
      float64_t rs1 = f32_to_f64(f32(READ_FREG(rs1_num))); \
      BODY32; \
      set_fp_exceptions; \
      break; \
    }\
    default: \
      require(0); \
      break; \
  }; \
  DEBUG_RVV_FP_VV; \
  VI_VFP_LOOP_END


#define VI_VFP_VV_LOOP_WIDE(BODY16, BODY32) \
  VI_CHECK_DSS(true); \
  VI_VFP_LOOP_BASE \
  switch(P.VU.vsew) { \
    case e16: {\
      float32_t &vd = P.VU.elt<float32_t>(rd_num, i, true); \
      if (STATE.bf16) {\
        float32_t vs2 = bf16_to_f32(P.VU.elt<bfloat16_t>(rs2_num, i)); \
        float32_t vs1 = bf16_to_f32(P.VU.elt<bfloat16_t>(rs1_num, i)); \
        BODY16; \
      } else {\
        float32_t vs2 = f16_to_f32(P.VU.elt<float16_t>(rs2_num, i)); \
        float32_t vs1 = f16_to_f32(P.VU.elt<float16_t>(rs1_num, i)); \
        BODY16; \
      }\
      set_fp_exceptions; \
      break; \
    }\
    case e32: {\
      float64_t &vd = P.VU.elt<float64_t>(rd_num, i, true); \
      float64_t vs2 = f32_to_f64(P.VU.elt<float32_t>(rs2_num, i)); \
      float64_t vs1 = f32_to_f64(P.VU.elt<float32_t>(rs1_num, i)); \
      BODY32; \
      set_fp_exceptions; \
      break; \
    }\
    default: \
      require(0); \
      break; \
  }; \
  DEBUG_RVV_FP_VV; \
  VI_VFP_LOOP_END

#define VI_VFP_WF_LOOP_WIDE(BODY16, BODY32) \
  VI_CHECK_DDS(false); \
  VI_VFP_LOOP_BASE \
  switch(P.VU.vsew) { \
    case e16: {\
      float32_t &vd = P.VU.elt<float32_t>(rd_num, i, true); \
      float32_t vs2 = P.VU.elt<float32_t>(rs2_num, i); \
      if (STATE.bf16) {\
        float32_t rs1 = bf16_to_f32(bf16(READ_FREG(rs1_num))); \
        BODY16; \
      } else {\
        float32_t rs1 = f16_to_f32(f16(READ_FREG(rs1_num))); \
        BODY16; \
      }\
      set_fp_exceptions; \
      break; \
    }\
    case e32: {\
      float64_t &vd = P.VU.elt<float64_t>(rd_num, i, true); \
      float64_t vs2 = P.VU.elt<float64_t>(rs2_num, i); \
      float64_t rs1 = f32_to_f64(f32(READ_FREG(rs1_num))); \
      BODY32; \
      set_fp_exceptions; \
      break; \
    }\
    default: \
      require(0); \
  }; \
  DEBUG_RVV_FP_VV; \
  VI_VFP_LOOP_END

#define VI_VFP_WV_LOOP_WIDE(BODY16, BODY32) \
  VI_CHECK_DDS(true); \
  VI_VFP_LOOP_BASE \
  switch(P.VU.vsew) { \
    case e16: {\
      float32_t &vd = P.VU.elt<float32_t>(rd_num, i, true); \
      float32_t vs2 = P.VU.elt<float32_t>(rs2_num, i); \
      if (STATE.bf16) {\
        float32_t vs1 = bf16_to_f32(P.VU.elt<bfloat16_t>(rs1_num, i)); \
        BODY16; \
      } else {\
        float32_t vs1 = f16_to_f32(P.VU.elt<float16_t>(rs1_num, i)); \
        BODY16; \
      }\
      set_fp_exceptions; \
      break; \
    }\
    case e32: {\
      float64_t &vd = P.VU.elt<float64_t>(rd_num, i, true); \
      float64_t vs2 = P.VU.elt<float64_t>(rs2_num, i); \
      float64_t vs1 = f32_to_f64(P.VU.elt<float32_t>(rs1_num, i)); \
      BODY32; \
      set_fp_exceptions; \
      break; \
    }\
    default: \
      require(0); \
  }; \
  DEBUG_RVV_FP_VV; \
  VI_VFP_LOOP_END

#define VI_VFP_LOOP_SCALE_BASE \
  require_fp; \
  require_vector(true);\
  require((P.VU.vsew == e8 && p->supports_extension(EXT_ZFH)) || \
          (P.VU.vsew == e16 && p->supports_extension('F')) || \
          (P.VU.vsew == e32 && p->supports_extension('D'))); \
  require(STATE.frm < 0x5);\
  reg_t vl = P.VU.vl; \
  reg_t rd_num = insn.rd(); \
  reg_t rs1_num = insn.rs1(); \
  reg_t rs2_num = insn.rs2(); \
  softfloat_roundingMode = STATE.frm; \
  for (reg_t i=P.VU.vstart; i<vl; ++i){ \
    VI_LOOP_ELEMENT_SKIP();

#define VI_VFP_CVT_SCALE(BODY8, BODY16B, BODY16, BODY32, \
                         CHECK8, CHECK16, CHECK32, \
                         is_widen, eew_check) \
  if (is_widen) { \
    VI_CHECK_DSS(false);\
  } else { \
    VI_CHECK_SDS(false); \
  } \
  require(eew_check); \
  switch(P.VU.vsew) { \
    case e8: {\
      CHECK8 \
      VI_VFP_LOOP_SCALE_BASE \
        BODY8 \
        set_fp_exceptions; \
      VI_VFP_LOOP_END \
      } \
      break; \
    case e16: {\
      CHECK16 \
      VI_VFP_LOOP_SCALE_BASE \
        if (STATE.bf16) {\
          BODY16B; \
        } else {\
          BODY16 \
        }\
        set_fp_exceptions; \
      VI_VFP_LOOP_END \
      } \
      break; \
    case e32: {\
      CHECK32 \
      VI_VFP_LOOP_SCALE_BASE \
        BODY32 \
        set_fp_exceptions; \
      VI_VFP_LOOP_END \
      } \
      break; \
    default: \
      require(0); \
      break; \
  }

#define DEBUG_START             (0x100)
#define DEBUG_END               (0xc0501000 - 1)

#define VME_DTYPE_DECODING_TO_TYPE(...) \
    bool relu = false; \
    using dtype_in = Float32; \
    switch (VME_DTYPE) { \
    case 0x0: { \
        using dtype_vd = half; \
        using dtype_lut = uint16_t; \
        __VA_ARGS__ \
    } \
        break; \
    case 0x01: { \
        using dtype_vd = Bfloat16; \
        using dtype_lut = uint16_t; \
        __VA_ARGS__ \
    } \
        break; \
    case 0x02: { \
        using dtype_vd = Float32; \
        using dtype_lut = uint32_t; \
        __VA_ARGS__ \
    } \
        break; \
    default: \
        break; \
    }

#define MTE_DTYPE_DECODING_TO_TYPE(...) \
  switch (MTE_DATA_TYPE) { \
    case 0x0: {\
      using dtype_vd = half; \
      using dtype_lut = uint16_t; \
       __VA_ARGS__ \
    } \
      break; \
    case 0x101: {\
      using dtype_vd = Bfloat16; \
      using dtype_lut = uint16_t; \
      __VA_ARGS__ \
    } \
      break; \
    case 0x202: {\
      using dtype_vd = Float32; \
      using dtype_lut = uint32_t; \
      __VA_ARGS__ \
    } \
      break; \
    case 0x303: {\
      using dtype_vd = int8_t; \
      using dtype_lut = int8_t; \
      __VA_ARGS__ \
    } \
      break; \
    default: \
        throw trap_tcp_illegal_encoding(); \
  }

#define DMAE_DTYPE_DECODING_TO_ESIZE() \
  uint8_t in_esize = 2; \
  uint8_t out_esize = 2; \
  switch (DMAE_DATA_TYPE) { \
    case 0x0: {\
      in_esize = 2; \
      out_esize = 2; \
    } \
      break; \
    case 0x101: {\
      in_esize = 2; \
      out_esize = 2; \
    } \
      break; \
    case 0x202: {\
      in_esize = 4; \
      out_esize = 4; \
    } \
      break; \
    case 0x303: {\
      in_esize = 1; \
      out_esize = 1; \
    } \
      break; \
    /*  \
    case 0x2: {\
      in_esize = 2; \
      out_esize = 4; \
    } \
      break; \
    case 0x102: {\
      in_esize = 2; \
      out_esize = 4; \
    } \
      break; \
    case 0x201: {\
      in_esize = 4; \
      out_esize = 2; \
    } \
      break; \
    case 0x200: {\
      in_esize = 4; \
      out_esize = 2; \
    } \
      break; \
    */ \
  }

#endif
