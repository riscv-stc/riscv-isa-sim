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
  int64_t shamt() { return x(20, 6); }
  int64_t s_imm() { return x(7, 5) + (xs(25, 7) << 5); }
  int64_t sb_imm() { return (x(8, 4) << 1) + (x(25,6) << 5) + (x(7,1) << 11) + (imm_sign() << 12); }
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
#define RS1 READ_REG(insn.rs1())
#define RS2 READ_REG(insn.rs2())
#define RS3 READ_REG(insn.rs3())
#define WRITE_RD(value) WRITE_REG(insn.rd(), value)
#define WRITE_RS1(value) WRITE_REG(insn.rs1(), value)
#define WRITE_RS2(value) WRITE_REG(insn.rs2(), value)

#ifndef RISCV_ENABLE_COMMITLOG
# define WRITE_REG(reg, value) STATE.XPR.write(reg, value)
# define WRITE_FREG(reg, value) DO_WRITE_FREG(reg, freg(value))
# define WRITE_VSTATUS
#else
   /* 0 : int
    * 1 : floating
    * 2 : vector reg
    * 3 : vector hint
    * 4 : csr
    */
# define WRITE_REG(reg, value) ({ \
    reg_t wdata = (value); /* value may have side effects */ \
    STATE.log_reg_write[(reg) << 4] = {wdata, 0}; \
    STATE.XPR.write(reg, wdata); \
  })
# define WRITE_FREG(reg, value) ({ \
    freg_t wdata = freg(value); /* value may have side effects */ \
    STATE.log_reg_write[((reg) << 4) | 1] = wdata; \
    DO_WRITE_FREG(reg, wdata); \
  })
# define WRITE_VSTATUS STATE.log_reg_write[3] = {0, 0};
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
#define VME_DTYPE (STATE.vme_data_type)
#define VME_DTYPE_VD (STATE.vme_data_type & 0xFF)
#define VME_DTYPE_VS1 ((STATE.vme_data_type & 0xFF00) >> 8)
#define VME_DTYPE_VS2 ((STATE.vme_data_type & 0xFF0000) >> 16)

#define VME_SHAPE1_COLUMN ((STATE.vme_FM_in & 0xFFFF0000) >> 16)
#define VME_SHAPE1_ROW (STATE.vme_FM_in & 0xFFFF)
#define VME_IFM_C_STRIDE ((STATE.vme_depth_in & 0xFFFF0000) >> 16)
#define VME_CIN (STATE.vme_depth_in & 0xFFFF)
#define VME_WOUT ((STATE.vme_FM_out & 0xFFFF0000) >> 16)
#define VME_HOUT (STATE.vme_FM_out & 0xFFFF)
#define VME_OFM_C_STRIDE ((STATE.vme_depth_stride & 0xFFFF0000) >> 16)
#define VME_K_C_STRIDE (STATE.vme_depth_stride & 0xFFFF)
#define VME_KW ((STATE.vme_kernel_params & 0xFF000000) >> 24)
#define VME_KH ((STATE.vme_kernel_params & 0xFF0000) >> 16)
#define VME_SW ((STATE.vme_kernel_params & 0xFF00) >> 8)
#define VME_SH (STATE.vme_kernel_params & 0xFF)
#define VME_N_PAD_U ((STATE.vme_FM_padding & 0xFF000000) >> 24)
#define VME_N_PAD_D ((STATE.vme_FM_padding & 0xFF0000) >> 16)
#define VME_N_PAD_L ((STATE.vme_FM_padding & 0xFF00) >> 8)
#define VME_N_PAD_R (STATE.vme_FM_padding & 0xFF)

// FPU macros
#define FRS1 READ_FREG(insn.rs1())
#define FRS2 READ_FREG(insn.rs2())
#define FRS3 READ_FREG(insn.rs3())
#define dirty_fp_state (STATE.mstatus |= MSTATUS_FS | (xlen == 64 ? MSTATUS64_SD : MSTATUS32_SD))
#define dirty_ext_state (STATE.mstatus |= MSTATUS_XS | (xlen == 64 ? MSTATUS64_SD : MSTATUS32_SD))
#define dirty_vs_state (STATE.mstatus |= MSTATUS_VS | (xlen == 64 ? MSTATUS64_SD : MSTATUS32_SD))
#define DO_WRITE_FREG(reg, value) (STATE.FPR.write(reg, value), dirty_fp_state)
#define WRITE_FRD(value) WRITE_FREG(insn.rd(), value)

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
#define SHAPE1_COLUMN ((STATE.shape_s1 & 0xFFFF0000) >> 16)
#define SHAPE1_ROW (STATE.shape_s1 & 0xFFFF)
#define SHAPE2_COLUMN ((STATE.shape_s2 & 0xFFFF0000) >> 16)
#define SHAPE2_ROW (STATE.shape_s2 & 0xFFFF)
#define STRIDE_RD (STATE.stride_d & 0xFFFF)
#define STRIDE_RS1 (STATE.stride_s & 0xFFFF)
#define STRIDE_RS2 ((STATE.stride_s & 0xFFFF0000) >> 16)
#define MME_SPARSE_BASE (STATE.mme_sparseidx_base)
#define MME_SPARSE_STRIDE (STATE.mme_sparseidx_stride & 0xFFFF)

#define BC_SHAPE1_COLUMN ((STATE.m_shape_s1 & 0xFFFF0000) >> 16)
#define BC_SHAPE1_ROW (STATE.m_shape_s1 & 0xFFFF)
#define BC_SHAPE2_COLUMN ((STATE.m_shape_s2 & 0xFFFF0000) >> 16)
#define BC_SHAPE2_ROW (STATE.m_shape_s2 & 0xFFFF)
#define BC_STRIDE_RD (STATE.m_stride_d & 0xFFFF)
#define BC_STRIDE_RS1 (STATE.m_stride_s & 0xFFFF)
#define BC_STRIDE_RS2 ((STATE.m_stride_s & 0xFFFF0000) >> 16)

#define MTE_SHAPE_COLUMN  ((STATE.mte_shape & 0xFFFF0000) >> 16)
#define MTE_SHAPE_ROW     (STATE.mte_shape & 0xFFFF)
#define STRIDE_LLB        (STATE.mte_stride_llb & 0xFFFF)
#define MTE_STRIDE_RS1        (STATE.mte_stride & 0xFFFF)
#define MTE_STRIDE_RD        ((STATE.mte_stride & 0xFFFF0000) >> 16)
#define MTE_DATA_TYPE_RD     (STATE.mte_data_type & 0xFF)
#define MTE_DATA_TYPE_RS1     ((STATE.mte_data_type & 0xFF00) >> 8)

#define SRC_CORE_ID     ((STATE.mte_icdest >> 16) & 0xFF)
#define DST_CORE_ID     (STATE.mte_icdest & 0xFF)

#define DMA_SHAPE_COLUMN  (STATE.dma_shape_col)
#define DMA_SHAPE_ROW     (STATE.dma_shape_row)
#define STRIDE_DDR        (STATE.dma_stride_ddr)

#define CONV_INFM_WH    (STATE.conv_FM_in)
#define CONV_DEPTH_IN   (STATE.conv_Depth_in)
#define CONV_OUTFM_WH   (STATE.conv_FM_out)
#define CONV_DEPTH_OUT  (STATE.conv_Depth_out)
#define CONV_KERNEL_PARAMS1   (STATE.conv_kernel_params1)
#define CONV_KERNEL_PARAMS2     (STATE.conv_kernel_params2)
#define CONV_PADDING    (STATE.conv_padding)
#define MME_QUANT_COEFF (STATE.mme_quant_coeff)
#define MME_DEQUANT_COEFF (STATE.mme_dequant_coeff)

#define CONV_IN_COLUMN	((STATE.conv_FM_in & 0xFFFF0000) >> 16)
#define CONV_IN_ROW	(STATE.conv_FM_in & 0xFFFF)
#define CONV_OUT_COLUMN	((STATE.conv_FM_out & 0xFFFF0000) >> 16)
#define CONV_OUT_ROW	(STATE.conv_FM_out & 0xFFFF)
#define CONV_CIN	(STATE.conv_Depth_in & 0xFFFF)
#define CONV_COUT	(STATE.conv_Depth_out & 0xFFFF)
#define CONV_IN_STRIDE	((STATE.conv_Depth_in & 0xFFFF0000) >> 16)
#define CONV_W_STRIDE	(STATE.conv_kernel_params1 & 0xFF)
#define CONV_OUT_STRIDE	((STATE.conv_Depth_out & 0xFFFF0000) >> 16)
#define CONV_KW 	((STATE.conv_kernel_params2 & 0xFF000000) >> 24)
#define CONV_KH 	((STATE.conv_kernel_params2 & 0xFF0000) >> 16)
#define CONV_DL 	((STATE.conv_kernel_params2 & 0xFF00) >> 8)
#define CONV_SK 	(STATE.conv_kernel_params2 & 0xFF)
#define CONV_SH 	(STATE.conv_kernel_params2 & 0xFF)
#define CONV_SW 	((STATE.conv_kernel_params1 & 0xFF0000) >> 16)


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
					 (x).shape2_column = SHAPE2_COLUMN; \
					 (x).shape2_row = SHAPE2_ROW; \
					 (x).stride_rd = STRIDE_RD / esize_out; \
					 (x).stride_rs1 = STRIDE_RS1 ? STRIDE_RS1 / esize_in : SHAPE1_COLUMN; \
					 (x).stride_rs2 = STRIDE_RS2 ? STRIDE_RS2 / esize_in : SHAPE1_COLUMN;})

#define vme_ss_fill(ss, esize) do { \
    ss.row = VME_SHAPE1_ROW; \
    ss.column = VME_SHAPE1_COLUMN; \
    ss.ifm_c_stride = VME_IFM_C_STRIDE ? VME_IFM_C_STRIDE / esize : VME_CIN; \
    ss.cin = VME_CIN; \
    ss.wout = VME_WOUT; \
    ss.hout = VME_HOUT; \
    ss.ofm_c_stride = VME_OFM_C_STRIDE ? VME_OFM_C_STRIDE / esize : VME_CIN; \
    ss.k_c_stride = VME_K_C_STRIDE ? VME_K_C_STRIDE / esize : VME_CIN; \
    ss.kw = VME_KW; \
    ss.kh = VME_KH; \
    ss.sw = VME_SW; \
    ss.sh = VME_SH; \
    ss.n_pad_u = VME_N_PAD_U; \
    ss.n_pad_d = VME_N_PAD_D; \
    ss.n_pad_l = VME_N_PAD_L; \
    ss.n_pad_r = VME_N_PAD_R; \
} while (0);

#define bc_sst_fill(x, esize_in, esize_out) ({ \
           (x).shape1_column = BC_SHAPE1_COLUMN; \
					 (x).shape1_row = BC_SHAPE1_ROW; \
					 (x).shape2_column = BC_SHAPE2_COLUMN; \
					 (x).shape2_row = BC_SHAPE2_ROW; \
					 (x).stride_rd = BC_STRIDE_RD / esize_out; \
					 (x).stride_rs1 = BC_STRIDE_RS1 ? BC_STRIDE_RS1 / esize_in : BC_SHAPE1_COLUMN; \
					 (x).stride_rs2 = BC_STRIDE_RS2 ? BC_STRIDE_RS2 / esize_in : BC_SHAPE2_COLUMN;})


#define memul_sst_fill(x, esize_src1, esize_src2, esize_out) ({ \
           (x).shape1_column = BC_SHAPE1_COLUMN; \
					 (x).shape1_row = BC_SHAPE1_ROW; \
					 (x).shape2_column = BC_SHAPE2_COLUMN; \
					 (x).shape2_row = BC_SHAPE2_ROW; \
					 (x).stride_rd = BC_STRIDE_RD / esize_out; \
					 (x).stride_rs1 = BC_STRIDE_RS1 ? BC_STRIDE_RS1 / esize_src1 : BC_SHAPE1_COLUMN; \
					 (x).stride_rs2 = BC_STRIDE_RS2 ? BC_STRIDE_RS2 / esize_src2 : BC_SHAPE2_COLUMN; \
           (x).stride_idx = MME_SPARSE_STRIDE ? MME_SPARSE_STRIDE : BC_SHAPE2_COLUMN * 2; \
           (x).mme_quant_coeff.v = MME_QUANT_COEFF; \
           (x).mme_dequant_coeff.v = MME_DEQUANT_COEFF;})


#define conv_fill(x) ({(x).conv_fm_in = CONV_INFM_WH; \
					 (x).conv_depth_in = CONV_DEPTH_IN; \
					 (x).conv_fm_out = CONV_OUTFM_WH; \
					 (x).conv_depth_out = CONV_DEPTH_OUT; \
					 (x).conv_kernel_params1 = CONV_KERNEL_PARAMS1; \
					 (x).conv_kernel_params2 = CONV_KERNEL_PARAMS2; \
					 (x).conv_padding = CONV_PADDING; \
           (x).mme_quant_coeff.v = MME_QUANT_COEFF; \
           (x).mme_dequant_coeff.v = MME_DEQUANT_COEFF; \
           (x).stride_idx = MME_SPARSE_STRIDE;})

#define SHAMT (insn.i_imm() & 0x3F)
#define BRANCH_TARGET (pc + insn.sb_imm())
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
#define isBoxedF16(r) (isBoxedF32(r) && ((uint64_t)((r.v[0] >> 16) + 1) == ((uint64_t)1 << 48)))
#define unboxF16(r) (isBoxedF16(r) ? (uint16_t)r.v[0] : defaultNaNF16UI)
#define isBoxedF32(r) (isBoxedF64(r) && ((uint32_t)((r.v[0] >> 32) + 1) == 0))
#define unboxF32(r) (isBoxedF32(r) ? (uint32_t)r.v[0] : defaultNaNF32UI)
#define isBoxedF64(r) ((r.v[1] + 1) == 0)
#define unboxF64(r) (isBoxedF64(r) ? r.v[0] : defaultNaNF64UI)
typedef float128_t freg_t;
inline float16_t f16(uint16_t v) { return { v }; }
inline float32_t f32(uint32_t v) { return { v }; }
inline float64_t f64(uint64_t v) { return { v }; }
inline float16_t f16(freg_t r) { return f16(unboxF16(r)); }
inline float32_t f32(freg_t r) { return f32(unboxF32(r)); }
inline float64_t f64(freg_t r) { return f64(unboxF64(r)); }
inline float128_t f128(freg_t r) { return r; }
inline freg_t freg(float16_t f) { return { ((uint64_t)-1 << 16) | f.v, (uint64_t)-1 }; }
inline freg_t freg(float32_t f) { return { ((uint64_t)-1 << 32) | f.v, (uint64_t)-1 }; }
inline freg_t freg(float64_t f) { return { f.v, (uint64_t)-1 }; }
inline freg_t freg(float128_t f) { return f; }
#define F16_SIGN ((uint16_t)1 << 15)
#define F32_SIGN ((uint32_t)1 << 31)
#define F64_SIGN ((uint64_t)1 << 63)
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
  if (((write) && csr_read_only) || STATE.prv < csr_priv) \
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

#define VI_CHECK_STORE(elt_width) \
  require_vector(false); \
  reg_t veew = sizeof(elt_width##_t) * 8; \
  float vemul = ((float)veew / P.VU.vsew * P.VU.vflmul); \
  reg_t emul = vemul < 1 ? 1 : vemul; \
  require(vemul >= 0.125 && vemul <= 8); \
  require_align(insn.rd(), vemul); \
  require((nf * emul) <= (NVPR / 4) && \
          (insn.rd() + nf * emul) <= NVPR); \

#define VI_CHECK_LOAD(elt_width) \
  VI_CHECK_STORE(elt_width); \
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

// throw trap if cust inst access out of l1&im buffer
#define check_cust_access(x, len) \
        if (!(p->get_sim()->in_local_mem(zext_xlen(x), L1_BUFFER) && \
              p->get_sim()->in_local_mem(zext_xlen(x) + len - 1, L1_BUFFER)) && \
            !(p->get_sim()->in_local_mem(zext_xlen(x), IM_BUFFER) && \
              p->get_sim()->in_local_mem(zext_xlen(x) + len - 1, IM_BUFFER))) { \
            throw trap_ncp_cust_access(x, 0, 0); \
        }

// throw trap if cust inst access out of l1 buffer
#define check_cust_access_l1(x, len) \
        if (!(p->get_sim()->in_local_mem(zext_xlen(x), L1_BUFFER) && \
              p->get_sim()->in_local_mem(zext_xlen(x) + len - 1, L1_BUFFER))) { \
            throw trap_ncp_cust_access(x, 0, 0); \
        }

// throw trap if cust inst access out of im buffer
#define check_cust_access_im(x, len) \
        if (!(p->get_sim()->in_local_mem(zext_xlen(x), IM_BUFFER) && \
              p->get_sim()->in_local_mem(zext_xlen(x) + len - 1, IM_BUFFER))) { \
            throw trap_ncp_cust_access(x, 0, 0); \
        }

// throw trap if cust inst access out of sp_idx buffer
#define check_cust_access_sp(x, len) \
        if (!(p->get_sim()->in_local_mem(zext_xlen(x), SP_BUFFER) && \
              p->get_sim()->in_local_mem(zext_xlen(x) + len - 1, SP_BUFFER))) { \
            throw trap_ncp_cust_access(x, 0, 0); \
        }

// throw trap if cust inst access misaligned base address
#define check_cust_misaligned_base(x, type) \
        if (unlikely(x & (sizeof(type##_t)-1))) { \
            throw trap_ncp_cust_misaligned_base(x, 0, 0); \
        }

// throw trap if cust inst source address access with misaligned stride
#define check_cust_misaligned_stride_src(x, type, stride) \
        if (unlikely(stride && (stride & (sizeof(type##_t)-1)))) { \
            throw trap_ncp_cust_misaligned_stride(x, 0, 0); \
        }

// throw trap if cust inst dest access with misaligned stride, or stride < width
#define check_cust_misaligned_stride_dst(x, type, stride, col) \
        if (unlikely(stride && (stride & (sizeof(type##_t)-1) || stride < col*sizeof(type##_t)))) { \
            throw trap_ncp_cust_misaligned_stride(x, 0, 0); \
        }

// throw trap if cust inst use invalid shape, col=0 or row=0
#define check_cust_invalid_shape(col, row) \
        if (unlikely(col == 0 || row == 0)) { \
            throw trap_ncp_cust_invalid_param(); \
        }
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
  })

// check traps for ve***.mm instructions
#define check_traps_vexxx_mm ({ \
        check_cust_misaligned_base(RS1, int16); \
        check_cust_misaligned_base(RS2, int16); \
        check_cust_misaligned_base(RD, int16); \
        check_cust_invalid_shape(SHAPE1_COLUMN, SHAPE1_ROW); \
        if(SHAPE1_ROW > 1) { \
          check_cust_misaligned_stride_src(RS1, int16, STRIDE_RS1); \
          check_cust_misaligned_stride_src(RS2, int16, STRIDE_RS2); \
          check_cust_misaligned_stride_dst(RD, int16, STRIDE_RD, SHAPE1_COLUMN); \
        } \
        int rs1_size = (STRIDE_RS1 ? STRIDE_RS1 : (SHAPE1_COLUMN * sizeof(int16_t))) * SHAPE1_ROW; \
        int rs2_size = (STRIDE_RS2 ? STRIDE_RS2 : (SHAPE1_COLUMN * sizeof(int16_t))) * SHAPE1_ROW; \
        int rd_size = (STRIDE_RD ? STRIDE_RD : (SHAPE1_COLUMN * sizeof(int16_t))) * SHAPE1_ROW; \
        check_cust_access(RS1, rs1_size); \
        check_cust_access(RS2, rs2_size); \
        check_cust_access(RD, rd_size); \
    })

// check traps for ve***.mm reduce with float result instructions
#define check_traps_vexxx_mm_reduce_all ({ \
        check_cust_misaligned_base(RS1, int16); \
        check_cust_misaligned_base(RS2, int16); \
        check_cust_invalid_shape(SHAPE1_COLUMN, SHAPE1_ROW); \
        if (SHAPE1_ROW > 1) { \
          check_cust_misaligned_stride_src(RS1, int16, STRIDE_RS1); \
          check_cust_misaligned_stride_src(RS2, int16, STRIDE_RS2); \
        } \
        int rs1_size = (STRIDE_RS1 ? STRIDE_RS1 : (SHAPE1_COLUMN * sizeof(int16_t))) * SHAPE1_ROW; \
        int rs2_size = (STRIDE_RS2 ? STRIDE_RS2 : (SHAPE1_COLUMN * sizeof(int16_t))) * SHAPE1_ROW; \
        check_cust_access(RS1, rs1_size); \
        check_cust_access(RS2, rs2_size); \
    })

// check traps for ve***.mm reduce with vector result instructions
#define check_traps_vexxx_mm_reduce_vector ({ \
        check_cust_misaligned_base(RS1, int16); \
        check_cust_misaligned_base(RS2, int16); \
        check_cust_misaligned_base(RD, int16); \
        check_cust_invalid_shape(SHAPE1_COLUMN, SHAPE1_ROW); \
        if (SHAPE1_ROW > 1) { \
          check_cust_misaligned_stride_src(RS1, int16, STRIDE_RS1); \
          check_cust_misaligned_stride_src(RS2, int16, STRIDE_RS2); \
        } \
        if ((DIM_DM == 1) && (SHAPE1_ROW > 1)) { \
            check_cust_misaligned_stride_dst(RD, int16, STRIDE_RD, 1); \
        } \
        int rs1_size = (STRIDE_RS1 ? STRIDE_RS1 : (SHAPE1_COLUMN * sizeof(int16_t))) * SHAPE1_ROW; \
        int rs2_size = (STRIDE_RS2 ? STRIDE_RS2 : (SHAPE1_COLUMN * sizeof(int16_t))) * SHAPE1_ROW; \
        check_cust_access(RS1, rs1_size); \
        check_cust_access(RS2, rs2_size); \
        if (DIM_DM == 0) { \
            check_cust_access(RD, SHAPE1_COLUMN * sizeof(int16_t)); \
        } else { \
            check_cust_access(RD, SHAPE1_ROW * sizeof(int16_t)); \
        } \
    })

// check traps for ve***.mv reduce with vector result instructions
#define check_traps_vexxx_mv_reduce_vector ({ \
        check_cust_misaligned_base(RS1, int16); \
        check_cust_misaligned_base(RS2, int16); \
        check_cust_misaligned_base(RD, int16); \
        check_cust_invalid_shape(SHAPE1_COLUMN, SHAPE1_ROW); \
        if (SHAPE1_ROW > 1) \
          check_cust_misaligned_stride_src(RS1, int16, STRIDE_RS1); \
        if ((DIM_DM == 1) && (SHAPE1_ROW > 1)) { \
            check_cust_misaligned_stride_dst(RD, int16, STRIDE_RD, 1); \
        } \
        int rs1_size = (STRIDE_RS1 ? STRIDE_RS1 : (SHAPE1_COLUMN * sizeof(int16_t))) * SHAPE1_ROW; \
        check_cust_access(RS1, rs1_size); \
        if (DIM_DM == 0) { \
            check_cust_access(RS2, SHAPE1_ROW * sizeof(int16_t)); \
            check_cust_access(RD, SHAPE1_COLUMN * sizeof(int16_t)); \
        } else { \
            check_cust_access(RS2, SHAPE1_COLUMN * sizeof(int16_t)); \
            check_cust_access(RD, SHAPE1_ROW * sizeof(int16_t)); \
        } \
    })

// check traps for ve***.mf reduce with vector result instructions
#define check_traps_vexxx_mf_reduce_vector ({ \
        check_cust_misaligned_base(RS1, int16); \
        check_cust_misaligned_base(RD, int16); \
        check_cust_invalid_shape(SHAPE1_COLUMN, SHAPE1_ROW); \
        if(SHAPE1_ROW > 1) \
          check_cust_misaligned_stride_src(RS1, int16, STRIDE_RS1); \
        if ((DIM_DM == 1) && (SHAPE1_ROW > 1)) { \
            check_cust_misaligned_stride_dst(RD, int16, STRIDE_RD, 1); \
        } \
        int rs1_size = (STRIDE_RS1 ? STRIDE_RS1 : (SHAPE1_COLUMN * sizeof(int16_t))) * SHAPE1_ROW; \
        check_cust_access(RS1, rs1_size); \
        if (DIM_DM == 0) { \
            check_cust_access(RD, SHAPE1_COLUMN * sizeof(int16_t)); \
        } else { \
            check_cust_access(RD, SHAPE1_ROW * sizeof(int16_t)); \
        } \
  })

// check traps for ve***.mv instructions
#define check_traps_vexxx_mv ({ \
        check_cust_misaligned_base(RS1, int16); \
        check_cust_misaligned_base(RS2, int16); \
        check_cust_misaligned_base(RD, int16); \
        check_cust_invalid_shape(SHAPE1_COLUMN, SHAPE1_ROW); \
        if(SHAPE1_ROW > 1) { \
          check_cust_misaligned_stride_src(RS1, int16, STRIDE_RS1); \
          check_cust_misaligned_stride_dst(RD, int16, STRIDE_RD, SHAPE1_COLUMN); \
        } \
        int rs1_size = (STRIDE_RS1 ? STRIDE_RS1 : (SHAPE1_COLUMN * sizeof(int16_t))) * SHAPE1_ROW; \
        int rd_size = (STRIDE_RD ? STRIDE_RD : (SHAPE1_COLUMN * sizeof(int16_t))) * SHAPE1_ROW; \
        check_cust_access(RS1, rs1_size); \
        if (DIM_DM == 0) { \
          check_cust_access(RS2, SHAPE1_COLUMN * sizeof(int16_t)); \
        } else { \
          check_cust_access(RS2, SHAPE1_ROW * sizeof(int16_t)); \
        } \
        check_cust_access(RD, rd_size); \
  })

// check traps for ve***.mv instructions, x32 matrix
#define check_traps_vexxx_mv_x32 ({ \
        check_cust_misaligned_base(RS1, int32); \
        check_cust_misaligned_base(RS2, int16); \
        check_cust_misaligned_base(RD, int16); \
        check_cust_invalid_shape(SHAPE1_COLUMN, SHAPE1_ROW); \
        if(SHAPE1_ROW > 1) { \
          check_cust_misaligned_stride_src(RS1, int32, STRIDE_RS1); \
          check_cust_misaligned_stride_dst(RD, int16, STRIDE_RD, SHAPE1_COLUMN); \
        } \
        int rs1_size = (STRIDE_RS1 ? STRIDE_RS1 : (SHAPE1_COLUMN * sizeof(int32_t))) * SHAPE1_ROW; \
        int rd_size = (STRIDE_RD ? STRIDE_RD : (SHAPE1_COLUMN * sizeof(int16_t))) * SHAPE1_ROW; \
        check_cust_access(RS1, rs1_size); \
        if (DIM_DM == 0) { \
          check_cust_access(RS2, SHAPE1_COLUMN * sizeof(int16_t)); \
        } else { \
          check_cust_access(RS2, SHAPE1_ROW * sizeof(int16_t)); \
        } \
        check_cust_access(RD, rd_size); \
  })

// check traps for ve***.mf instructions
#define check_traps_vexxx_mf ({ \
        check_cust_misaligned_base(RS1, int16); \
        check_cust_misaligned_base(RD, int16); \
        check_cust_invalid_shape(SHAPE1_COLUMN, SHAPE1_ROW); \
        if(SHAPE1_ROW > 1) { \
          check_cust_misaligned_stride_src(RS1, int16, STRIDE_RS1); \
          check_cust_misaligned_stride_dst(RD, int16, STRIDE_RD, SHAPE1_COLUMN); \
        } \
        int rs1_size = (STRIDE_RS1 ? STRIDE_RS1 : (SHAPE1_COLUMN * sizeof(int16_t))) * SHAPE1_ROW; \
        int rd_size = (STRIDE_RD ? STRIDE_RD : (SHAPE1_COLUMN * sizeof(int16_t))) * SHAPE1_ROW; \
        check_cust_access(RS1, rs1_size); \
        check_cust_access(RD, rd_size); \
  })

// check traps for ve***.mf instructions, x32 matrix
#define check_traps_vexxx_mf_x32 ({ \
        check_cust_misaligned_base(RS1, int32); \
        check_cust_misaligned_base(RD, int16); \
        check_cust_invalid_shape(SHAPE1_COLUMN, SHAPE1_ROW); \
        if(SHAPE1_ROW > 1) { \
          check_cust_misaligned_stride_src(RS1, int32, STRIDE_RS1); \
          check_cust_misaligned_stride_dst(RD, int16, STRIDE_RD, SHAPE1_COLUMN); \
        } \
        int rs1_size = (STRIDE_RS1 ? STRIDE_RS1 : (SHAPE1_COLUMN * sizeof(int32_t))) * SHAPE1_ROW; \
        int rd_size = (STRIDE_RD ? STRIDE_RD : (SHAPE1_COLUMN * sizeof(int16_t))) * SHAPE1_ROW; \
        check_cust_access(RS1, rs1_size); \
        check_cust_access(RD, rd_size); \
  })

// check traps for ve***.mf instructions, .x8.hf.mf
#define check_traps_vexxx_mf_x8out ({ \
        check_cust_misaligned_base(RS1, int16); \
        check_cust_invalid_shape(SHAPE1_COLUMN, SHAPE1_ROW); \
        if(SHAPE1_ROW > 1) { \
          check_cust_misaligned_stride_src(RS1, int16, STRIDE_RS1); \
          check_cust_misaligned_stride_dst(RD, int8, STRIDE_RD, SHAPE1_COLUMN); \
        } \
        int rs1_size = (STRIDE_RS1 ? STRIDE_RS1 : (SHAPE1_COLUMN * sizeof(int16_t))) * SHAPE1_ROW; \
        int rd_size = (STRIDE_RD ? STRIDE_RD : (SHAPE1_COLUMN * sizeof(int8_t))) * SHAPE1_ROW; \
        check_cust_access(RS1, rs1_size); \
        check_cust_access(RD, rd_size); \
  })


// check traps for ve***.m instructions, element-wise
#define check_traps_vexxx_m_element_wise(esize) ({ \
        check_cust_misaligned_base(RS1, int16); \
        check_cust_misaligned_base(RD, int16); \
        check_cust_invalid_shape(SHAPE1_COLUMN, SHAPE1_ROW); \
        check_tcp_data_type \
        if(SHAPE1_ROW > 1) { \
          check_cust_misaligned_stride_src(RS1, int16, STRIDE_RS1); \
          check_cust_misaligned_stride_dst(RD, int16, STRIDE_RD, SHAPE1_COLUMN); \
        } \
        int rs1_size = (STRIDE_RS1 ? STRIDE_RS1 : (SHAPE1_COLUMN * esize)) * SHAPE1_ROW; \
        int rd_size = (STRIDE_RD ? STRIDE_RD : (SHAPE1_COLUMN * esize)) * SHAPE1_ROW; \
        check_cust_access(RS1, rs1_size); \
        check_cust_access(RD, rd_size); \
  })

// check traps for ve***.m instructions, reduce all
#define check_traps_vexxx_m_reduce_all ({ \
        check_cust_misaligned_base(RS1, int16); \
        check_cust_invalid_shape(SHAPE1_COLUMN, SHAPE1_ROW); \
        if(SHAPE1_ROW > 1) \
          check_cust_misaligned_stride_src(RS1, int16, STRIDE_RS1); \
        int rs1_size = (STRIDE_RS1 ? STRIDE_RS1 : (SHAPE1_COLUMN * sizeof(int16_t))) * SHAPE1_ROW; \
        check_cust_access(RS1, rs1_size); \
  })

// check traps for ve***.m instructions, reduce to vector
#define check_traps_vexxx_m_reduce_vector ({ \
        check_cust_misaligned_base(RS1, int16); \
        check_cust_misaligned_base(RD, int16); \
        check_cust_invalid_shape(SHAPE1_COLUMN, SHAPE1_ROW); \
        if(SHAPE1_ROW > 1) \
          check_cust_misaligned_stride_src(RS1, int16, STRIDE_RS1); \
        if ((DIM_DM == 1) && (SHAPE1_ROW > 1)) { \
            check_cust_misaligned_stride_dst(RD, int16, STRIDE_RD, 1); \
        } \
        int rs1_size = (STRIDE_RS1 ? STRIDE_RS1 : (SHAPE1_COLUMN * sizeof(int16_t))) * SHAPE1_ROW; \
        check_cust_access(RS1, rs1_size); \
        if (DIM_DM == 0) { \
          check_cust_access(RD, SHAPE1_COLUMN * sizeof(int16_t)); \
        } else { \
          check_cust_access(RD, SHAPE1_ROW * sizeof(int16_t)); \
        } \
  })


// check traps for ve***.m instructions, element-wise
#define check_traps_vexxx_m_convert(dtype, stype) ({ \
        check_cust_misaligned_base(RS1, stype); \
        check_cust_misaligned_base(RD, dtype); \
        check_cust_invalid_shape(SHAPE1_COLUMN, SHAPE1_ROW); \
        if(SHAPE1_ROW > 1) { \
          check_cust_misaligned_stride_src(RS1, stype, STRIDE_RS1); \
          check_cust_misaligned_stride_dst(RD, dtype, STRIDE_RD, SHAPE1_COLUMN); \
        } \
        int rs1_size = (STRIDE_RS1 ? STRIDE_RS1 : (SHAPE1_COLUMN * sizeof(stype##_t))) * SHAPE1_ROW; \
        int rd_size = (STRIDE_RD ? STRIDE_RD : (SHAPE1_COLUMN * sizeof(dtype##_t))) * SHAPE1_ROW; \
        check_cust_access(RS1, rs1_size); \
        check_cust_access(RD, rd_size); \
  })

// check traps for velut.m instructions
#define check_traps_velut_m ({ \
        check_cust_misaligned_base(RS1, int16); \
        check_cust_misaligned_base(RS2, int16); \
        check_cust_misaligned_base(RD, int16); \
        check_cust_invalid_shape(SHAPE1_COLUMN, SHAPE1_ROW); \
        if(SHAPE1_ROW > 1) { \
          check_cust_misaligned_stride_src(RS1, int16, STRIDE_RS1); \
          check_cust_misaligned_stride_dst(RD, int16, STRIDE_RD, SHAPE1_COLUMN); \
        } \
        int rs1_size = (STRIDE_RS1 ? STRIDE_RS1 : (SHAPE1_COLUMN * sizeof(int16_t))) * SHAPE1_ROW; \
        int rd_size = (STRIDE_RD ? STRIDE_RD : (SHAPE1_COLUMN * sizeof(int16_t))) * SHAPE1_ROW; \
        check_cust_access(RS1, rs1_size); \
        check_cust_access(RS2, 0x10000); \
        check_cust_access(RD, rd_size); \
  })

// check traps for mov.f instructions
#define check_traps_mov_f(esize) ({ \
        check_cust_misaligned_base(RD, int16); \
        check_cust_invalid_shape(SHAPE1_COLUMN, SHAPE1_ROW); \
        check_tcp_data_type \
        if (SHAPE1_ROW > 1) \
          check_cust_misaligned_stride_dst(RD, int16, STRIDE_RD, SHAPE1_COLUMN); \
        int rd_size = (STRIDE_RD ? STRIDE_RD : (SHAPE1_COLUMN * esize)) * SHAPE1_ROW; \
        check_cust_access(RD, rd_size); \
  })

// check traps for mov.v instructions
#define check_traps_mov_v(esize) ({ \
        check_cust_misaligned_base(RS1, int16); \
        check_cust_misaligned_base(RD, int16); \
        check_cust_invalid_shape(SHAPE1_COLUMN, SHAPE1_ROW); \
        check_tcp_data_type \
        if (SHAPE1_ROW > 1) \
          check_cust_misaligned_stride_dst(RD, int16, STRIDE_RD, SHAPE1_COLUMN); \
        int rd_size = (STRIDE_RD ? STRIDE_RD : (SHAPE1_COLUMN * esize)) * SHAPE1_ROW; \
        if (DIM_DM == 0) { \
          check_cust_access(RS1, SHAPE1_COLUMN * esize); \
        } else { \
          check_cust_access(RS1, SHAPE1_ROW * esize); \
        } \
        check_cust_access(RD, rd_size); \
  })

// check traps for memul.mm instructions
#define check_traps_memul_mm(in_type, out_type) ({ \
        check_cust_misaligned_base(RS1, in_type); \
        check_cust_misaligned_base(RS2, in_type); \
        check_cust_misaligned_base(RD, out_type); \
        check_cust_invalid_shape(BC_SHAPE1_ROW, BC_SHAPE1_COLUMN); \
        check_cust_invalid_shape(BC_SHAPE2_ROW, BC_SHAPE2_COLUMN); \
        if(BC_SHAPE1_ROW > 1)  { \
          check_cust_misaligned_stride_src(RS1, in_type, BC_STRIDE_RS1); \
          check_cust_misaligned_stride_dst(RD, out_type, BC_STRIDE_RD, BC_SHAPE2_COLUMN); \
        } \
        if(BC_SHAPE2_ROW > 1)  \
          check_cust_misaligned_stride_src(RS2, in_type, BC_STRIDE_RS2); \
        int rs1_size = (BC_STRIDE_RS1 ? BC_STRIDE_RS1 : (BC_SHAPE1_COLUMN * sizeof(in_type##_t))) * BC_SHAPE1_ROW; \
        int rs2_size = (BC_STRIDE_RS2 ? BC_STRIDE_RS2 : (BC_SHAPE2_COLUMN * sizeof(in_type##_t))) * BC_SHAPE2_ROW; \
        int rd_size = (BC_STRIDE_RD ? BC_STRIDE_RD : (BC_SHAPE2_COLUMN * sizeof(out_type##_t))) * BC_SHAPE1_ROW;\
        check_cust_access(RS1, rs1_size); \
        check_cust_access_l1(RS2, rs2_size); \
        check_cust_access_im(RD, rd_size); \
  })
// check traps for memul.tsx.mm instructions
#define check_traps_memul_ts_mm(in_type, out_type) ({ \
        check_cust_misaligned_base(RS1, in_type); \
        check_cust_misaligned_base(RS2, in_type); \
        check_cust_misaligned_base(RD, out_type); \
        check_cust_invalid_shape(BC_SHAPE1_ROW, BC_SHAPE1_COLUMN); \
        check_cust_invalid_shape(BC_SHAPE2_ROW, BC_SHAPE2_COLUMN); \
        if(BC_SHAPE1_ROW > 1)  { \
          check_cust_misaligned_stride_src(RS1, in_type, BC_STRIDE_RS1); \
          check_cust_misaligned_stride_dst(RD, out_type, BC_STRIDE_RD, BC_SHAPE2_COLUMN); \
        } \
        if(BC_SHAPE2_ROW > 1)  \
          check_cust_misaligned_stride_src(RS2, in_type, BC_STRIDE_RS2); \
        int rs1_size = (BC_STRIDE_RS1 ? BC_STRIDE_RS1 : (BC_SHAPE1_COLUMN * sizeof(in_type##_t))) * BC_SHAPE1_ROW; \
        int rs2_size = (BC_STRIDE_RS2 ? BC_STRIDE_RS2 : (BC_SHAPE2_COLUMN * sizeof(in_type##_t))) * BC_SHAPE2_ROW; \
        unsigned int rd_stride, rd_col; \
        if (TS == 0){ \
          rd_stride = BC_SHAPE2_COLUMN; rd_col = BC_SHAPE1_COLUMN; \
        } \
        int rd_size = (BC_STRIDE_RD ? BC_STRIDE_RD : (rd_stride * sizeof(out_type##_t))) * rd_col;\
        check_cust_access(RS1, rs1_size); \
        check_cust_access_l1(RS2, rs2_size); \
        check_cust_access_im(RD, rd_size); \
  })
// check traps for memul.sp.mm instructions
#define check_traps_memul_sp_mm(in_type, out_type) ({ \
        check_cust_misaligned_base(RS1, in_type); \
        check_cust_misaligned_base(RS2, in_type); \
        check_cust_misaligned_base(RD, out_type); \
        if (unlikely(MME_SPARSE_BASE & 1)) { \
            throw trap_ncp_cust_misaligned_base(MME_SPARSE_BASE, 0, 0); \
        } \
        check_sp_invalid_shape(BC_SHAPE1_ROW, BC_SHAPE1_COLUMN); \
        check_cust_invalid_shape(BC_SHAPE2_ROW, BC_SHAPE2_COLUMN); \
        if(BC_SHAPE1_ROW > 1)  { \
          check_cust_misaligned_stride_src(RS1, in_type, BC_STRIDE_RS1); \
          check_cust_misaligned_stride_dst(RD, out_type, BC_STRIDE_RD, BC_SHAPE2_COLUMN); \
        } \
        if(BC_SHAPE2_ROW > 1)  {\
          check_cust_misaligned_stride_src(RS2, in_type, BC_STRIDE_RS2); \
          if (unlikely(MME_SPARSE_STRIDE && (MME_SPARSE_STRIDE & 1))) { \
            throw trap_ncp_cust_misaligned_stride(MME_SPARSE_STRIDE, 0, 0); \
          } \
        } \
        int rs1_size = (BC_STRIDE_RS1 ? BC_STRIDE_RS1 : (BC_SHAPE1_COLUMN * sizeof(in_type##_t))) * BC_SHAPE1_ROW; \
        int rs2_size = (BC_STRIDE_RS2 ? BC_STRIDE_RS2 : (BC_SHAPE2_COLUMN * sizeof(in_type##_t))) * BC_SHAPE2_ROW; \
        int idx_size = (MME_SPARSE_STRIDE ? MME_SPARSE_STRIDE : (BC_SHAPE2_COLUMN * 2)) * BC_SHAPE2_ROW; \
        int rd_size = (BC_STRIDE_RD ? BC_STRIDE_RD : (BC_SHAPE2_COLUMN * sizeof(out_type##_t))) * BC_SHAPE1_ROW;\
        check_cust_access(RS1, rs1_size); \
        check_cust_access_l1(RS2, rs2_size); \
        check_cust_access_im(RD, rd_size); \
        check_cust_access_sp(MME_SPARSE_BASE, idx_size); \
  })

// check traps for memin.m/memax.m/meacc.m instructions, reduce all
#define check_traps_mexxx_m(in_type, out_type)({ \
        check_cust_misaligned_base(RS1, in_type); \
        check_cust_misaligned_base(RD, out_type); \
        check_cust_invalid_shape(BC_SHAPE1_ROW, BC_SHAPE1_COLUMN); \
        if(BC_SHAPE1_ROW > 1)  { \
          check_cust_misaligned_stride_src(RS1, in_type, BC_STRIDE_RS1); \
        } \
        int rs1_size = (BC_STRIDE_RS1 ? BC_STRIDE_RS1 : (BC_SHAPE1_COLUMN * sizeof(in_type##_t))) \
                                * BC_SHAPE1_ROW; \
        int rd_size = BC_SHAPE1_ROW * sizeof(out_type##_t); \
        check_cust_access(RS1, rs1_size); \
        check_cust_access_im(RD, rd_size); \
  })


// check traps for meconv.mm instructions
#define check_traps_meconv_mm(in_type, out_type) ({ \
        check_cust_misaligned_base(RS1, in_type); \
        check_cust_misaligned_base(RS2, in_type); \
        check_cust_misaligned_base(RD, out_type); \
        check_cust_invalid_shape(CONV_IN_ROW, CONV_IN_COLUMN); \
        check_cust_invalid_shape(CONV_OUT_ROW, CONV_OUT_COLUMN); \
        check_cust_invalid_shape(CONV_CIN, CONV_COUT); \
        check_cust_invalid_shape(CONV_KH, CONV_KW); \
        if (unlikely(CONV_SK == 0 || CONV_DL == 0)) { \
            throw trap_ncp_cust_invalid_param(); \
        } \
        check_cust_misaligned_stride_src(RS1, in_type, CONV_IN_STRIDE); \
        check_cust_misaligned_stride_src(RS2, in_type, CONV_W_STRIDE); \
        check_cust_misaligned_stride_dst(RD, out_type, CONV_OUT_STRIDE, CONV_COUT); \
        int rs1_size = (CONV_IN_STRIDE ? CONV_IN_STRIDE : (CONV_CIN * sizeof(in_type##_t))) * \
                       (CONV_IN_COLUMN * CONV_IN_ROW); \
        int rs2_size = (CONV_W_STRIDE ? CONV_W_STRIDE : (CONV_COUT * sizeof(in_type##_t))) * \
                       (CONV_KH * CONV_KW * CONV_CIN); \
        int rd_size = (CONV_OUT_STRIDE ? CONV_OUT_STRIDE : (CONV_COUT * sizeof(out_type##_t))) * \
                      (CONV_OUT_COLUMN * CONV_OUT_ROW); \
        check_cust_access(RS1, rs1_size); \
        check_cust_access_l1(RS2, rs2_size); \
        check_cust_access_im(RD, rd_size); \
  })

// check traps for meconv.mm instructions
#define check_traps_medeconv_mm(in_type, out_type) ({ \
        check_cust_misaligned_base(RS1, in_type); \
        check_cust_misaligned_base(RS2, in_type); \
        check_cust_misaligned_base(RD, out_type); \
        check_cust_invalid_shape(CONV_IN_ROW, CONV_IN_COLUMN); \
        check_cust_invalid_shape(CONV_OUT_ROW, CONV_OUT_COLUMN); \
        check_cust_invalid_shape(CONV_CIN, CONV_COUT); \
        check_cust_invalid_shape(CONV_KH, CONV_KW); \
        if (unlikely(CONV_SH == 0 || CONV_SW == 0)) { \
            throw trap_ncp_cust_invalid_param(); \
        } \
        check_cust_misaligned_stride_src(RS1, in_type, CONV_IN_STRIDE); \
        check_cust_misaligned_stride_src(RS2, in_type, CONV_W_STRIDE); \
        check_cust_misaligned_stride_dst(RD, out_type, CONV_OUT_STRIDE, CONV_COUT); \
        int rs1_size = (CONV_IN_STRIDE ? CONV_IN_STRIDE : (CONV_CIN * sizeof(in_type##_t))) * \
                       (CONV_IN_COLUMN * CONV_IN_ROW); \
        int rs2_size = (CONV_W_STRIDE ? CONV_W_STRIDE : (CONV_COUT * sizeof(in_type##_t))) * \
                       (CONV_KH * CONV_KW * CONV_CIN); \
        int rd_size = (CONV_OUT_STRIDE ? CONV_OUT_STRIDE : (CONV_COUT * sizeof(out_type##_t))) * \
                      (CONV_OUT_COLUMN * CONV_OUT_ROW); \
        check_cust_access(RS1, rs1_size); \
        check_cust_access_l1(RS2, rs2_size); \
        check_cust_access_im(RD, rd_size); \
  })

// check traps for meconv.mm instructions
#define check_traps_medeconv_sp_mm(in_type, out_type) ({ \
        check_cust_misaligned_base(RS1, in_type); \
        check_cust_misaligned_base(RS2, in_type); \
        check_cust_misaligned_base(RD, out_type); \
        if (unlikely(MME_SPARSE_BASE & 1)) { \
            throw trap_ncp_cust_misaligned_base(MME_SPARSE_BASE, 0, 0); \
        } \
        check_cust_invalid_shape(CONV_IN_ROW, CONV_IN_COLUMN); \
        check_cust_invalid_shape(CONV_OUT_ROW, CONV_OUT_COLUMN); \
        check_cust_invalid_shape(CONV_CIN, CONV_COUT); \
        check_cust_invalid_shape(CONV_KH, CONV_KW); \
        if (unlikely(CONV_SH == 0 || CONV_SW == 0)) { \
            throw trap_ncp_cust_invalid_param(); \
        } \
        check_cust_misaligned_stride_src(RS1, in_type, CONV_IN_STRIDE); \
        check_cust_misaligned_stride_src(RS2, in_type, CONV_W_STRIDE); \
        check_cust_misaligned_stride_dst(RD, out_type, CONV_OUT_STRIDE, CONV_COUT); \
        int rs1_size = (CONV_IN_STRIDE ? CONV_IN_STRIDE : (CONV_CIN * sizeof(in_type##_t))) * \
                       (CONV_IN_COLUMN * CONV_IN_ROW); \
        int rs2_size = (CONV_W_STRIDE ? CONV_W_STRIDE : (CONV_COUT * sizeof(in_type##_t))) * \
                       (CONV_KH * CONV_KW * CONV_CIN); \
        int idx_size = (MME_SPARSE_STRIDE ? MME_SPARSE_STRIDE : (CONV_COUT * 2)) *  \
                       (CONV_KH * CONV_KW * CONV_CIN); \
        int rd_size = (CONV_OUT_STRIDE ? CONV_OUT_STRIDE : (CONV_COUT * sizeof(out_type##_t))) * \
                      (CONV_OUT_COLUMN * CONV_OUT_ROW); \
        check_cust_access(RS1, rs1_size); \
        check_cust_access_l1(RS2, rs2_size); \
        check_cust_access_im(RD, rd_size); \
        check_cust_access_sp(MME_SPARSE_BASE, idx_size); \
  })

// check traps for meconv.sp.mm instructions
#define check_traps_meconv_sp_mm(in_type, out_type) ({ \
        check_cust_misaligned_base(RS1, in_type); \
        check_cust_misaligned_base(RS2, in_type); \
        check_cust_misaligned_base(RD, out_type); \
        if (unlikely(MME_SPARSE_BASE & 1)) { \
            throw trap_ncp_cust_misaligned_base(MME_SPARSE_BASE, 0, 0); \
        } \
        check_cust_invalid_shape(CONV_IN_ROW, CONV_IN_COLUMN); \
        check_cust_invalid_shape(CONV_OUT_ROW, CONV_OUT_COLUMN); \
        check_cust_invalid_shape(CONV_CIN, CONV_COUT); \
        check_cust_invalid_shape(CONV_KH, CONV_KW); \
        if (unlikely(CONV_SK == 0 || CONV_DL == 0)) { \
            throw trap_ncp_cust_invalid_param(); \
        } \
        check_cust_misaligned_stride_src(RS1, in_type, CONV_IN_STRIDE); \
        check_cust_misaligned_stride_src(RS2, in_type, CONV_W_STRIDE); \
        check_cust_misaligned_stride_dst(RD, out_type, CONV_OUT_STRIDE, CONV_COUT); \
        int rs1_size = (CONV_IN_STRIDE ? CONV_IN_STRIDE : (CONV_CIN * sizeof(in_type##_t))) * \
                       (CONV_IN_COLUMN * CONV_IN_ROW); \
        int rs2_size = (CONV_W_STRIDE ? CONV_W_STRIDE : (CONV_COUT * sizeof(in_type##_t))) * \
                       (CONV_KH * CONV_KW * CONV_CIN); \
        int idx_size = (MME_SPARSE_STRIDE ? MME_SPARSE_STRIDE : (CONV_COUT * 2)) *  \
                       (CONV_KH * CONV_KW * CONV_CIN); \
        int rd_size = (CONV_OUT_STRIDE ? CONV_OUT_STRIDE : (CONV_COUT * sizeof(out_type##_t))) * \
                      (CONV_OUT_COLUMN * CONV_OUT_ROW); \
        check_cust_access(RS1, rs1_size); \
        check_cust_access_l1(RS2, rs2_size); \
        check_cust_access_im(RD, rd_size); \
        check_cust_access_sp(MME_SPARSE_BASE, idx_size); \
  })
#define check_tcp_data_type \
  if (MTE_DATA_TYPE_RS1 != MTE_DATA_TYPE_RD) \
    trap_tcp_invalid_param();

// throw trap if tcp icmov's target core id not exist
#define check_tcp_icmov_invalid_core_id(core_id, max_id) \
        if (core_id > max_id) { \
            throw trap_tcp_icmov_invalid_core(); \
        }


// throw trap if tcp source start address in L1Buffer
#define check_tcp_access_start_l1(x) \
        if ((!(p->get_sim()->in_local_mem(zext_xlen(x), L1_BUFFER))) && \
            (!(p->get_sim()->in_local_mem(zext_xlen(x), SP_BUFFER)))) { \
            throw trap_tcp_access_start(x); \
        }

// throw trap if tcp source start address in L1Buffer
#define check_tcp_access_start_icmov(x) \
        if (!(p->get_sim()->in_local_mem(zext_xlen(x), L1_BUFFER))) { \
            throw trap_tcp_access_start_icmov(x); \
        }

// throw trap if tcp source end address in L1Buffer
#define check_tcp_access_end_l1(x) \
        if ((!(p->get_sim()->in_local_mem(zext_xlen(x), L1_BUFFER))) && \
            (!(p->get_sim()->in_local_mem(zext_xlen(x), SP_BUFFER)))) { \
            throw trap_tcp_access_end_l1(x); \
        }

// throw trap if tcp source start address in LLB for mov*
#define check_tcp_access_start_llb_mov(x) \
        if (zext_xlen(x) < LLB_AXI0_BUFFER_START) { \
            throw trap_tcp_access_start(x); \
        } \
        if (zext_xlen(x) >= LLB_AXI0_BUFFER_START+LLB_BUFFER_SIZE) { \
            throw trap_tcp_access_start(x); \
        }

// throw trap if tcp source start address in LLB for pld
#define check_tcp_access_start_llb_pld(x) \
        if (zext_xlen(x) < LLB_AXI0_BUFFER_START) { \
            throw trap_tcp_illegal_encoding(); \
        } \
        if (zext_xlen(x) >= LLB_AXI0_BUFFER_START+LLB_BUFFER_SIZE) { \
            throw trap_tcp_illegal_encoding(); \
        }

// throw trap if tcp source end address in L1Buffer
#define check_tcp_access_end_llb(x) \
        if (zext_xlen(x) < LLB_AXI0_BUFFER_START) { \
            throw trap_tcp_access_end_llb(x); \
        } \
        if (zext_xlen(x) >= LLB_AXI0_BUFFER_START+LLB_BUFFER_SIZE) { \
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
        check_tcp_access_start_l1(RS1) \
        check_tcp_access_start_icmov(RD) \
        check_tcp_access_end_l1(RS1 + RS2) \
        check_tcp_access_end_l1(RD + RS2) \
})

// check traps for icmov_m instruction
#define check_traps_icmov_m(esize) ({ \
        check_tcp_icmov_invalid_core_id(DST_CORE_ID, CORE_COUNT) \
        check_tcp_data_type \
        check_tcp_access_start_l1(RS1) \
        check_tcp_access_start_icmov(RD) \
        check_tcp_access_end_l1(RS1 + (MTE_STRIDE_RS1 ? MTE_STRIDE_RS1 * MTE_SHAPE_ROW : (MTE_SHAPE_COLUMN * MTE_SHAPE_ROW * esize))) \
        check_tcp_access_end_l1(RD + (MTE_STRIDE_RD ? MTE_STRIDE_RD * MTE_SHAPE_ROW : (MTE_SHAPE_COLUMN * MTE_SHAPE_ROW * esize))) \
})

// check traps for pld instruction
#define check_traps_pld(esize) ({ \
        check_tcp_invalid_param(MTE_SHAPE_COLUMN, MTE_SHAPE_ROW, MTE_STRIDE_RS1) \
        check_tcp_data_type \
        check_tcp_access_start_l1(RD) \
        check_tcp_access_start_llb_pld(RS1) \
        check_tcp_access_end_l1(RD + (MTE_STRIDE_RD ? MTE_STRIDE_RD * MTE_SHAPE_ROW : (MTE_SHAPE_COLUMN * MTE_SHAPE_ROW * esize))) \
        check_tcp_access_end_llb(RS1 + (MTE_STRIDE_RS1 ? MTE_STRIDE_RS1 * MTE_SHAPE_ROW : (MTE_SHAPE_COLUMN * MTE_SHAPE_ROW * esize))) \
})

// check traps for mov.l1.llb instruction
#define check_traps_mov_l1_llb(esize) ({ \
        check_tcp_invalid_param(MTE_SHAPE_COLUMN, MTE_SHAPE_ROW, MTE_STRIDE_RS1) \
        check_tcp_data_type \
        check_tcp_access_start_llb_mov(RS1) \
        check_tcp_access_start_l1(RD) \
        check_tcp_access_end_llb(RS1 + (MTE_STRIDE_RS1 ? MTE_STRIDE_RS1 * MTE_SHAPE_ROW : (MTE_SHAPE_COLUMN * MTE_SHAPE_ROW * esize))) \
        check_tcp_access_end_l1(RD + (MTE_STRIDE_RD ? MTE_STRIDE_RD * MTE_SHAPE_ROW : (MTE_SHAPE_COLUMN * MTE_SHAPE_ROW * esize))) \
})

// check traps for mov.llb.l instruction
#define check_traps_mov_llb_l1(esize) ({ \
        check_tcp_invalid_param(MTE_SHAPE_COLUMN, MTE_SHAPE_ROW, MTE_STRIDE_RS1) \
        check_tcp_data_type \
        check_tcp_access_start_l1(RS1) \
        check_tcp_access_start_llb_mov(RD) \
        check_tcp_access_end_l1(RS1 + (MTE_STRIDE_RS1 ? MTE_STRIDE_RS1 * MTE_SHAPE_ROW : (MTE_SHAPE_COLUMN * MTE_SHAPE_ROW * esize))) \
        check_tcp_access_end_llb(RD + (MTE_STRIDE_RD ? MTE_STRIDE_RD * MTE_SHAPE_ROW : (MTE_SHAPE_COLUMN * MTE_SHAPE_ROW * esize))) \
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

#define VI_LD(stride, offset, elt_width) \
  const reg_t nf = insn.v_nf() + 1; \
  const reg_t vl = P.VU.vl; \
  const reg_t baseAddr = RS1; \
  const reg_t vd = insn.rd(); \
  VI_CHECK_LOAD(elt_width); \
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

#define VI_ST(stride, offset, elt_width) \
  const reg_t nf = insn.v_nf() + 1; \
  const reg_t vl = P.VU.vl; \
  const reg_t baseAddr = RS1; \
  const reg_t vs3 = insn.rd(); \
  VI_CHECK_STORE(elt_width); \
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
  VI_CHECK_LOAD(elt_width); \
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

#define VI_VFP_VV_LOOP(BODY16, BODY32, BODY64) \
  VI_CHECK_SSS(true); \
  VI_VFP_LOOP_BASE \
  switch(P.VU.vsew) { \
    case e16: {\
      float16_t &vd = P.VU.elt<float16_t>(rd_num, i, true); \
      float16_t vs1 = P.VU.elt<float16_t>(rs1_num, i); \
      float16_t vs2 = P.VU.elt<float16_t>(rs2_num, i); \
      BODY16; \
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

#define VI_VFP_V_LOOP(BODY16, BODY32, BODY64) \
  VI_CHECK_SSS(false); \
  VI_VFP_LOOP_BASE \
  switch(P.VU.vsew) { \
    case e16: {\
      float16_t &vd = P.VU.elt<float16_t>(rd_num, i, true); \
      float16_t vs2 = P.VU.elt<float16_t>(rs2_num, i); \
      BODY16; \
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

#define VI_VFP_VV_LOOP_REDUCTION(BODY16, BODY32, BODY64) \
  VI_CHECK_REDUCTION(false) \
  VI_VFP_COMMON \
  switch(P.VU.vsew) { \
    case e16: {\
      VI_VFP_LOOP_REDUCTION_BASE(16) \
        BODY16; \
        set_fp_exceptions; \
      VI_VFP_LOOP_REDUCTION_END(e16) \
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
        float32_t vs2 = f16_to_f32(P.VU.elt<float16_t>(rs2_num, i)); \
        BODY16; \
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

#define VI_VFP_VF_LOOP(BODY16, BODY32, BODY64) \
  VI_CHECK_SSS(false); \
  VI_VFP_LOOP_BASE \
  switch(P.VU.vsew) { \
    case e16: {\
      float16_t &vd = P.VU.elt<float16_t>(rd_num, i, true); \
      float16_t rs1 = f16(READ_FREG(rs1_num)); \
      float16_t vs2 = P.VU.elt<float16_t>(rs2_num, i); \
      BODY16; \
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

#define VI_VFP_LOOP_CMP(BODY16, BODY32, BODY64, is_vs1) \
  VI_CHECK_MSS(is_vs1); \
  VI_VFP_LOOP_CMP_BASE \
  switch(P.VU.vsew) { \
    case e16: {\
      float16_t vs2 = P.VU.elt<float16_t>(rs2_num, i); \
      float16_t vs1 = P.VU.elt<float16_t>(rs1_num, i); \
      float16_t rs1 = f16(READ_FREG(rs1_num)); \
      BODY16; \
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
      float32_t vs2 = f16_to_f32(P.VU.elt<float16_t>(rs2_num, i)); \
      float32_t rs1 = f16_to_f32(f16(READ_FREG(rs1_num))); \
      BODY16; \
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
      float32_t vs2 = f16_to_f32(P.VU.elt<float16_t>(rs2_num, i)); \
      float32_t vs1 = f16_to_f32(P.VU.elt<float16_t>(rs1_num, i)); \
      BODY16; \
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
      float32_t rs1 = f16_to_f32(f16(READ_FREG(rs1_num))); \
      BODY16; \
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
      float32_t vs1 = f16_to_f32(P.VU.elt<float16_t>(rs1_num, i)); \
      BODY16; \
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

#define VI_VFP_CVT_SCALE(BODY8, BODY16, BODY32, \
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
        BODY16 \
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
    switch (VME_DTYPE) { \
    case 0x9: \
        relu = true; \
    case 0x0: { \
        using dtype_vd = half; \
        using dtype_vs1 = half; \
        using dtype_vs2 = half; \
        __VA_ARGS__ \
    } \
        break; \
    case 0x1010a: \
        relu = true; \
    case 0x10101: { \
        using dtype_vd = Bfloat16; \
        using dtype_vs1 = Bfloat16; \
        using dtype_vs2 = Bfloat16; \
        __VA_ARGS__ \
    } \
        break; \
    case 0x2020b: \
        relu = true; \
    case 0x20202: { \
        using dtype_vd = Float32; \
        using dtype_vs1 = Float32; \
        using dtype_vs2 = Float32; \
        __VA_ARGS__ \
    } \
        break; \
    default: \
        throw trap_illegal_instruction(insn.bits()); \
    }

#define MTE_DTYPE_DECODING_TO_TYPE(...) \
  switch (MTE_DATA_TYPE_RS1) { \
    case 0x0: {\
      using dtype_vd = half; \
      using dtype_vs1 = half; \
       __VA_ARGS__ \
    } \
      break; \
    case 0x1: {\
      using dtype_vd = Bfloat16; \
      using dtype_vs1 = Bfloat16; \
      __VA_ARGS__ \
    } \
      break; \
    case 0x2: {\
      using dtype_vd = Float32; \
      using dtype_vs1 = Float32; \
      __VA_ARGS__ \
    } \
      break; \
    case 0x3: {\
      using dtype_vd = int8_t; \
      using dtype_vs1 = int8_t; \
      __VA_ARGS__ \
    } \
      break; \
    case 0x4: {\
      using dtype_vd = uint8_t; \
      using dtype_vs1 = uint8_t; \
      __VA_ARGS__ \
    } \
      break; \
    case 0x5: {\
      using dtype_vd = int16_t; \
      using dtype_vs1 = int16_t; \
      __VA_ARGS__ \
    } \
      break; \
    case 0x6: {\
      using dtype_vd = uint16_t; \
      using dtype_vs1 = uint16_t; \
      __VA_ARGS__ \
    } \
      break; \
    case 0x7: {\
      using dtype_vd = int32_t; \
      using dtype_vs1 = int32_t; \
      __VA_ARGS__ \
    } \
      break; \
    case 0x8: {\
      using dtype_vd = uint32_t; \
      using dtype_vs1 = uint32_t; \
      __VA_ARGS__ \
    } \
      break; \
    default: \
        throw trap_illegal_instruction(insn.bits()); \
  }

#define VME_DTYPE_DECODING_TO_TYPE_ACC(...) \
    bool relu = false; \
    switch (VME_DTYPE) { \
    case 0x9: \
        relu = true; \
    case 0x0: { \
        using dtype_vd = half; \
        using dtype_in = Float32; \
        __VA_ARGS__ \
    } \
        break; \
    default: \
        throw trap_illegal_instruction(insn.bits()); \
    }

#endif
