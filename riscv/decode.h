// See LICENSE for license details.

#ifndef _RISCV_DECODE_H
#define _RISCV_DECODE_H

#if (-1 != ~0) || ((-1 >> 1) != -1)
# error spike requires a two''s-complement c++ implementation
#endif

#ifdef WORDS_BIGENDIAN
# error spike requires a little-endian host
#endif

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

typedef int64_t sreg_t;
typedef uint64_t reg_t;

const int NXPR = 32;
const int NFPR = 32;
const int NVPR = 32;
const int NCSR = 4096;

#define X_RA 1
#define X_SP 2

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


#define VTYPE_EDIV_SHIFT 5
#define VTYPE_SEW_SHIFT  2
#define VTYPE_LMUL_SHIFT 0

#define VTYPE_EDIV 3
#define VTYPE_VSEW  7
#define VTYPE_VLMUL 3

typedef uint64_t insn_bits_t;
class insn_t
{
public:
  insn_t() = default;
  insn_t(insn_bits_t bits) : b(bits) {}
  insn_bits_t bits() { return b; }
  int length() { return insn_length(b); }
  int64_t i_imm() { return int64_t(b) >> 20; }
  int64_t shamt() { return x(20, 6); }
  int64_t s_imm() { return x(7, 5) + (xs(25, 7) << 5); }
  int64_t sb_imm() { return (x(8, 4) << 1) + (x(25,6) << 5) + (x(7,1) << 11) + (imm_sign() << 12); }
  int64_t u_imm() { return int64_t(b) >> 12 << 12; }
  int64_t uj_imm() { return (x(21, 10) << 1) + (x(20, 1) << 11) + (x(12, 8) << 12) + (imm_sign() << 20); }
  uint64_t v_uimm() { return x(15, 5); }
  uint64_t rd() { return x(7, 5); }
  uint64_t rs1() { return x(15, 5); }
  uint64_t rs2() { return x(20, 5); }
  uint64_t rs3() { return x(27, 5); }
  uint64_t rm() { return x(12, 3); }
  uint64_t csr() { return x(20, 12); }
  uint64_t dim() { return (x(25, 1) << 1) + x(14, 1); }
  uint64_t ts() { return x(14, 1); }
  uint64_t vm() { return x(25, 1); }
  uint64_t vlmul() {return x(20,2);}
  uint64_t vsew() { return x(22,3);}
  uint64_t vediv() { return x(25,2);}
  uint64_t nf() { return x(29, 3); }
  uint64_t mop() { return x(26, 3); }
  uint64_t width() { return x(12, 3); }
  uint64_t lumop() { return x(20, 5); }
  uint64_t sumop() { return x(20, 5); }

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
  void write_vh(size_t i, size_t v_idx, unsigned short value)
  {
	data[i].vh[v_idx] = value;
  }
  void write_vb(size_t i, size_t v_idx, unsigned char value)
  {
    data[i].vb[v_idx] = value;
  }
  const T& operator [] (size_t i) const
  {
    return data[i];
  }
private:
  T data[N];
};

// helpful macros, etc
#define MMU (*p->get_mmu())
#define STATE (*p->get_state())
#define READ_REG(reg) (unlikely((((0x40000800 <= pc) && (0x40000884 >= pc)) || ((0x40000360 <= pc) && (0x40000374 > pc))) && (reg == 0)) ? 0x40000000 : STATE.XPR[reg])

#define READ_FREG(reg) STATE.FPR[reg]
#define READ_VREG(reg) STATE.VPR[reg]
#define RS1 READ_REG(insn.rs1())
#define RS2 READ_REG(insn.rs2())
#define RD READ_REG(insn.rd())
#define WRITE_RD(value) WRITE_REG(insn.rd(), value)
#define WRITE_RS1(value) WRITE_REG(insn.rs1(), value)
#define WRITE_RS2(value) WRITE_REG(insn.rs2(), value)

#ifndef RISCV_ENABLE_COMMITLOG
# define WRITE_REG(reg, value) STATE.XPR.write(reg, value)
# define WRITE_FREG(reg, value) DO_WRITE_FREG(reg, freg(value))
//# define WRITE_VREG(reg, value) DO_WRITE_VREG(reg, vreg(value))
# define WRITE_VREG_H(reg_h, idx, value) STATE.VPR.write_vh(reg_h, idx, value)
# define WRITE_VREG_B(reg_b, idx, value) STATE.VPR.write_vb(reg_b, idx, value)
#else
# define WRITE_REG(reg, value) ({ \
    reg_t wdata = (value); /* value may have side effects */ \
    STATE.log_reg_write = (commit_log_reg_t){(reg) << 1, {wdata, 0}}; \
    STATE.XPR.write(reg, wdata); \
  })
# define WRITE_FREG(reg, value) ({ \
    freg_t wdata = freg(value); /* value may have side effects */ \
    STATE.log_reg_write = (commit_log_reg_t){((reg) << 1) | 1, wdata}; \
    DO_WRITE_FREG(reg, wdata); \
  })
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

// FPU macros
#define FRS1 READ_FREG(insn.rs1())
#define FRS2 READ_FREG(insn.rs2())
#define FRS3 READ_FREG(insn.rs3())
#define FLEN (*p->get_flen())
#define dirty_fp_state (STATE.mstatus |= MSTATUS_FS | (xlen == 64 ? MSTATUS64_SD : MSTATUS32_SD))
#define dirty_ext_state (STATE.mstatus |= MSTATUS_XS | (xlen == 64 ? MSTATUS64_SD : MSTATUS32_SD))
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

#define VLEN (p->get_vlen())
#define SLEN (p->get_slen())
#define ELEN (p->get_elen())
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

#define DST_CHIP_ID     ((STATE.mte_icdest >> 16) & 0xF)
#define DST_CORE_ID     (STATE.mte_icdest & 0x3F)
#define MTE_CORE_MAP    (STATE.mte_coremap)

#define CONV_INFM_WH    (STATE.conv_FM_in)
#define CONV_DEPTH_IN   (STATE.conv_Depth_in)
#define CONV_OUTFM_WH   (STATE.conv_FM_out)
#define CONV_DEPTH_OUT  (STATE.conv_Depth_out)
#define CONV_S_KERNEL   (STATE.conv_S_kernel)
#define CONV_KERNEL     (STATE.conv_kernel)
#define CONV_PADDING    (STATE.conv_padding)

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

#define vector_for_each(x) for(unsigned int (x) = VSTART; (x) < VLMAX; (x)++)
#define vector_for_each_from_zero(x) for(unsigned int (x) = 0; (x) < VLMAX; (x)++)
#define vector_for_each_no_lmlu(x) for(unsigned int (x) = 0; (x) < VLMAX_NO_LMUL; (x)++)

#define vdh_clear_exceed(idx) if(idx >= VL) {WRITE_VRD_H(0,idx); continue;} \
								else

#define sst_fill(x) ({(x).shape1_column = SHAPE1_COLUMN; \
					 (x).shape1_row = SHAPE1_ROW; \
					 (x).shape2_column = SHAPE2_COLUMN; \
					 (x).shape2_row = SHAPE2_ROW; \
					 (x).stride_rd = STRIDE_RD>>1; \
					 (x).stride_rs1 = STRIDE_RS1>>1; \
					 (x).stride_rs2 = STRIDE_RS2>>1;})

#define bc_sst_fill(x) ({(x).shape1_column = BC_SHAPE1_COLUMN; \
					 (x).shape1_row = BC_SHAPE1_ROW; \
					 (x).shape2_column = BC_SHAPE2_COLUMN; \
					 (x).shape2_row = BC_SHAPE2_ROW; \
					 (x).stride_rd = BC_STRIDE_RD>>1; \
					 (x).stride_rs1 = BC_STRIDE_RS1>>1; \
					 (x).stride_rs2 = BC_STRIDE_RS2>>1;})

#define conv_fill(x) ({(x).conv_fm_in = CONV_INFM_WH; \
					 (x).conv_depth_in = CONV_DEPTH_IN; \
					 (x).conv_fm_out = CONV_OUTFM_WH; \
					 (x).conv_depth_out = CONV_DEPTH_OUT; \
					 (x).conv_s_kernel = CONV_S_KERNEL; \
					 (x).conv_kernel = CONV_KERNEL; \
					 (x).conv_padding = CONV_PADDING;})

#define SHAMT (insn.i_imm() & 0x3F)
#define BRANCH_TARGET (pc + insn.sb_imm())
#define JUMP_TARGET (pc + insn.uj_imm())
#define RM ({ int rm = insn.rm(); \
              if(rm == 7) rm = STATE.frm; \
              if(rm > 4) throw trap_illegal_instruction(0); \
              rm; })

#define get_field(reg, mask) (((reg) & (decltype(reg))(mask)) / ((mask) & ~((mask) << 1)))
#define set_field(reg, mask, val) (((reg) & ~(decltype(reg))(mask)) | (((decltype(reg))(val) * ((mask) & ~((mask) << 1))) & (decltype(reg))(mask)))

#define require(x) if (unlikely(!(x))) throw trap_illegal_instruction(0)
#define require_privilege(p) require(STATE.prv >= (p))
#define require_rv64 require(xlen == 64)
#define require_rv32 require(xlen == 32)
#define require_extension(s) require(p->supports_extension(s))
#define require_fp require((STATE.mstatus & MSTATUS_FS) != 0)
#define require_accelerator require((STATE.mstatus & MSTATUS_XS) != 0)

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
#define isBoxedF32(r) (isBoxedF64(r) && ((uint32_t)((r.v[0] >> 32) + 1) == 0))
#define unboxF32(r) (isBoxedF32(r) ? (uint32_t)r.v[0] : defaultNaNF32UI)
#define isBoxedF64(r) ((r.v[1] + 1) == 0)
#define unboxF64(r) (isBoxedF64(r) ? r.v[0] : defaultNaNF64UI)
typedef float128_t freg_t;
inline float32_t f32(uint32_t v) { return { v }; }
inline float64_t f64(uint64_t v) { return { v }; }
inline float32_t f32(freg_t r) { return f32(unboxF32(r)); }
inline float64_t f64(freg_t r) { return f64(unboxF64(r)); }
inline float128_t f128(freg_t r) { return r; }
inline freg_t freg(float32_t f) { return { ((uint64_t)-1 << 32) | f.v, (uint64_t)-1 }; }
inline freg_t freg(float64_t f) { return { f.v, (uint64_t)-1 }; }
inline freg_t freg(float128_t f) { return f; }
#define F32_SIGN ((uint32_t)1 << 31)
#define F64_SIGN ((uint64_t)1 << 63)
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

// Seems that 0x0 doesn't work.
#define DEBUG_BASE              (0x40000000)
#define DEBUG_START             (0x100)
#define DEBUG_END               (0x40001000 - 1)

#define VREG_LENGTH (128)
/*Vector instruction support*/
union vreg_t{
	unsigned char vb[VREG_LENGTH];
	unsigned short vh[VREG_LENGTH / 2];
}; //vector reg length 256bit
//typedef int64_t vreg_t; //vector reg length 256bit


#endif
