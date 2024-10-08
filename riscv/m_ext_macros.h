#ifndef _RISCV_M_EXT_MACROS_H
#define _RISCV_M_EXT_MACROS_H

#include "vector_unit.h"
#include "v_ext_macros.h"
#include <math.h>

// #define LMUL_RESERVE 3

#define MTYPE_FP32 0x1
#define MTYPE_TFP32 0x2
#define MTYPE_FP16 0x1
#define MTYPE_BF16 0x2
#define MTYPE_FP8E4M3 0x1
#define MTYPE_FP8E5M2 0x2
#define MTYPE_FP8E3M4 0x3

// matrix 0.5 del mxsat
/* #define P_SET_OV(ov) \
   if (ov) P.MU.mxsat->write(1); */

#define M_FLOAT_TYPE_CHECK(sew) \
  require((sew == e8 && (p->extension_enabled(EXT_MATRIX_ZMF8E4M3) || \
          p->extension_enabled(EXT_MATRIX_ZMF8E5M2) || p->extension_enabled(EXT_MATRIX_ZMF8E3M4))) || \
          (sew == e16 && p->extension_enabled(EXT_ZFH)) || \
          (sew == e32 && p->extension_enabled('F')) || \
          (sew == e64 && p->extension_enabled('D'))); \

#define MX_PARAMS(x) \
  type_sew_t<x>::type &td  = P.MU.tr_elt<type_sew_t<x>::type>(td_num + m, 0, i, j, mmax, nmax, reg_rename, true); \
  type_sew_t<x>::type &ts1  = P.MU.tr_elt<type_sew_t<x>::type>(ts1_num + m, 0, i, k, mmax, nmax, false, false); \
  type_sew_t<x>::type &ts2  = P.MU.tr_elt<type_sew_t<x>::type>(ts2_num + m, 0, k, j, mmax, nmax, false, false); \

#define MX_ACCD_PARAMS(x) \
  type_sew_t<x>::type &accd  = P.MU.acc_elt<type_sew_t<x>::type>(td_num + m, 0, i, j, mmax, nmax * amul, false, true); \
  type_sew_t<x>::type ts1  = P.MU.tr_elt<type_sew_t<x>::type>(ts1_num + m, 0, i, k, mmax, nmax, false, false); \
  type_sew_t<x>::type ts2  = P.MU.tr_elt<type_sew_t<x>::type>(ts2_num + m, 0, k, j, mmax, nmax, false, false); \

#define MX2D_PARAMS(x) \
  type_sew_t<x>::type &td  = P.MU.acc_elt<type_sew_t<x>::type>(td_num + m, 0, i, j, mmax, nmax * amul, reg_rename, true); \
  type_sew_t<x>::type ts1  = P.MU.acc_elt<type_sew_t<x>::type>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false); \
  type_sew_t<x>::type ts2  = P.MU.acc_elt<type_sew_t<x>::type>(ts2_num + m, 0, i, j, mmax, nmax * amul, false, false); \

#define MXSU_PARAMS(x) \
  type_sew_t<x>::type &td  = P.MU.acc_elt<type_sew_t<x>::type>(td_num + m, 0, i, j, mmax, nmax * amul, reg_rename, true); \
  type_sew_t<x>::type ts1  = P.MU.acc_elt<type_sew_t<x>::type>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false); \
  type_usew_t<x>::type ts2  = P.MU.acc_elt<type_usew_t<x>::type>(ts2_num + m, 0, i, j, mmax, nmax * amul, false, false); \

#define MX_TD_PARAMS(x) \
  type_sew_t<x>::type &td  = P.MU.tr_elt<type_sew_t<x>::type>(td_num + m, 0, i, j, mmax, nmax, reg_rename, true); \

#define MX_TS1_PARAMS(x) \
  type_sew_t<x>::type ts1  = P.MU.tr_elt<type_sew_t<x>::type>(ts1_num + m, 0, i, j, mmax, nmax, false, false); \

#define MX_ACC1_PARAMS(x) \
  type_sew_t<x>::type &acc1  = P.MU.acc_elt<type_sew_t<x>::type>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false); \

#define MX_AD_PARAMS(x) \
  type_sew_t<x>::type &accd  = P.MU.acc_elt<type_sew_t<x>::type>(td_num + m, 0, i, j, mmax, nmax * amul, reg_rename, true); \

#define MX_TS1_TD_PARAMS(x) \
  MX_TD_PARAMS(x) \
  MX_TS1_PARAMS(x) \

#define MX_ACC1_AD_PARAMS(x) \
  MX_AD_PARAMS(x) \
  MX_ACC1_PARAMS(x) \

#define MXU_TS1_PARAMS(x) \
  type_usew_t<x>::type ts1  = P.MU.tr_elt<type_usew_t<x>::type>(ts1_num + m, 0, i, k, mmax, nmax, false, false); \

#define MX_TS2_PARAMS(x) \
  type_sew_t<x>::type ts2  = P.MU.tr_elt<type_sew_t<x>::type>(ts2_num + m, 0, i, k, mmax, nmax, false, false); \

#define MXU_TS2_PARAMS(x) \
  type_usew_t<x>::type ts2  = P.MU.tr_elt<type_usew_t<x>::type>(ts2_num + m, 0, i, j, mmax, nmax, false, false); \

#define MXU_ACC2_PARAMS(x) \
  type_usew_t<x>::type acc2  = P.MU.tr_elt<type_usew_t<x>::type>(ts2_num + m, 0, i, j, mmax, nmax * amul, false, false); \

#define MXU_PARAMS(x) \
  type_usew_t<x>::type &td  = P.MU.tr_elt<type_usew_t<x>::type>(td_num + m, 0, i, j, mmax, nmax, reg_rename, true); \
  type_usew_t<x>::type &ts1  = P.MU.tr_elt<type_usew_t<x>::type>(ts1_num + m, 0, i, k, mmax, nmax, false, false); \
  type_usew_t<x>::type &ts2  = P.MU.tr_elt<type_usew_t<x>::type>(ts2_num + m, 0, k, j, mmax, nmax, false, false); \

#define MXU_ACCD_PARAMS(x) \
  type_usew_t<x>::type &accd  = P.MU.acc_elt<type_usew_t<x>::type>(td_num + m, 0, i, j, mmax, nmax * amul, false, true); \
  type_usew_t<x>::type ts1  = P.MU.tr_elt<type_usew_t<x>::type>(ts1_num + m, 0, i, k, mmax, nmax, false, false); \
  type_usew_t<x>::type ts2  = P.MU.tr_elt<type_usew_t<x>::type>(ts2_num + m, 0, k, j, mmax, nmax, false, false); \

#define MXU2D_PARAMS(x) \
  type_usew_t<x>::type &td  = P.MU.acc_elt<type_usew_t<x>::type>(td_num + m, 0, i, j, mmax, nmax * amul, reg_rename, true); \
  type_usew_t<x>::type ts1  = P.MU.acc_elt<type_usew_t<x>::type>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false); \
  type_usew_t<x>::type ts2  = P.MU.acc_elt<type_usew_t<x>::type>(ts2_num + m, 0, i, j, mmax, nmax * amul, false, false); \

#define MX_PARAM_BASE(x) \
  type_sew_t<x>::type ts1  = P.MU.tr_elt<type_sew_t<x>::type>(ts1_num + m, 0, i, k, mmax, nmax, false, false); \
  type_sew_t<x>::type ts2  = P.MU.tr_elt<type_sew_t<x>::type>(ts2_num + m, 0, k, j, mmax, nmax, false, false); \

#define MX_PARAM2D_BASE(x) \
  type_sew_t<x>::type ts1  = P.MU.tr_elt<type_sew_t<x>::type>(ts1_num + m, 0, i, j, mmax, nmax, false, false); \
  type_sew_t<x>::type ts2  = P.MU.tr_elt<type_sew_t<x>::type>(ts2_num + m, 0, i, j, mmax, nmax, false, false); \

#define MX_ACC_PARAM2D_BASE(x) \
  type_sew_t<x>::type acc1  = P.MU.acc_elt<type_sew_t<x>::type>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false); \
  type_sew_t<x>::type acc2  = P.MU.acc_elt<type_sew_t<x>::type>(ts2_num + m, 0, i, j, mmax, nmax * amul, false, false); \

#define MXU_PARAM_BASE(x) \
  type_usew_t<x>::type ts1  = P.MU.tr_elt<type_usew_t<x>::type>(ts1_num + m, 0, i, k, mmax, nmax, false, false); \
  type_usew_t<x>::type ts2  = P.MU.tr_elt<type_usew_t<x>::type>(ts2_num + m, 0, k, j, mmax, nmax, false, false); \

#define MX_ACC1_SIGN_BASE(x) \
  type_sew_t<x>::type acc1  = P.MU.acc_elt<type_sew_t<x>::type>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false); \

#define MX_ACC1_USIGN_BASE(x) \
  type_usew_t<x>::type acc1  = P.MU.acc_elt<type_usew_t<x>::type>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false); \

#define MX_ACCD_USIGN_BASE(x) \
  type_usew_t<x>::type &accd  = P.MU.acc_elt<type_usew_t<x>::type>(td_num + m, 0, i, j, mmax, nmax * amul, false, true); \

#define MX_ACCD_SIGN_BASE(x) \
  type_sew_t<x>::type &accd  = P.MU.acc_elt<type_sew_t<x>::type>(td_num + m, 0, i, j, mmax, nmax * amul, false, true); \

#define MX_ACCD_USING_WIDE_BASE(x, mult) \
  type_usew_t<x * mult>::type &accd  = P.MU.acc_elt<type_usew_t<x * mult>::type>(td_num + m, \
        0, i, j, mmax, nmax / mult * amul, reg_rename, true); \

#define MX_ACCD_SING_WIDE_BASE(x, mult) \
  type_sew_t<x * mult>::type &accd  = P.MU.acc_elt<type_sew_t<x * mult>::type>(td_num + m, \
        0, i, j, mmax, nmax / mult * amul, reg_rename, true); \

#define MX_ACCD_USING_NARR_BASE(x, narr) \
  type_usew_t<x / narr>::type &accd  = P.MU.acc_elt<type_usew_t<x / narr>::type>(td_num + m, \
        0, i, j, mmax, nmax * narr * amul, reg_rename, true); \

#define MX_ACCD_SING_NARR_BASE(x, narr) \
  type_sew_t<x / narr>::type &accd  = P.MU.acc_elt<type_sew_t<x / narr>::type>(td_num + m, \
        0, i, j, mmax, nmax * narr * amul, reg_rename, true); \

#define MX_ACCD_USING_NARR_E4_BASE(x, narr) \
  type_usew_t<x / narr>::type &accd  = P.MU.acc_elt<type_usew_t<x / narr>::type>(td_num + m, \
        0, i, j / 2, mmax, nmax * narr * amul, reg_rename, true); \

#define MX_ACCD_SING_NARR_E4_BASE(x, narr) \
  type_sew_t<x / narr>::type &accd  = P.MU.acc_elt<type_sew_t<x / narr>::type>(td_num + m, \
        0, i, j / 2, mmax, nmax * narr * amul, reg_rename, true); \

#define MX_SUDS_PARAMS(x, mult) \
  MX_ACCD_SIGN_BASE(x) \
  MX_ACC1_USIGN_BASE(x) \
  
#define MX_SSDU_PARAMS(x, mult) \
  MX_ACCD_USIGN_BASE(x) \
  MX_ACC1_SIGN_BASE(x) \

#define MX_SSDS_WIDE_PARAMS(x, mult) \
  MX_ACCD_SING_WIDE_BASE(x, mult) \
  MX_ACC1_USIGN_BASE(x) \
  
#define MX_SUDU_WIDE_PARAMS(x, mult) \
  MX_ACCD_USING_WIDE_BASE(x, mult) \
  MX_ACC1_SIGN_BASE(x) \

#define MX_SSDS_NARR_PARAMS(x, narr) \
  MX_ACCD_SING_NARR_BASE(x, narr) \
  MX_ACC1_USIGN_BASE(x) \
  
#define MX_SUDU_NARR_PARAMS(x, narr) \
  MX_ACCD_USING_NARR_BASE(x, narr) \
  MX_ACC1_SIGN_BASE(x) \

#define MX_SSDS_NARR_E4_PARAMS(x, narr) \
  MX_ACCD_SING_NARR_E4_BASE(x, narr) \
  MX_ACC1_USIGN_BASE(x) \
  
#define MX_SUDU_NARR_E4_PARAMS(x, narr) \
  MX_ACCD_USING_NARR_E4_BASE(x, narr) \
  MX_ACC1_SIGN_BASE(x) \

#define MMULXU_PARAMS(x, mult) \
  type_usew_t<x * mult>::type &accd  = P.MU.acc_elt<type_usew_t<x * mult>::type>(td_num + m, \
        0, i, j, mmax, nmax / mult * amul, reg_rename, true); \
  MXU_PARAM_BASE(x) \

#define MMULXU2D_PARAMS(x, mult) \
  type_usew_t<x * mult>::type &accd  = P.MU.acc_elt<type_usew_t<x * mult>::type>(td_num + m, \
        0, i, j, mmax, nmax / mult * amul, reg_rename, true); \
  type_usew_t<x>::type acc1  = P.MU.acc_elt<type_usew_t<x>::type>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false); \
  type_usew_t<x>::type acc2  = P.MU.acc_elt<type_usew_t<x>::type>(ts2_num + m, 0, i, j, mmax, nmax * amul, false, false); \

#define MMULX_PARAMS(x, mult) \
  type_sew_t<x * mult>::type &accd  = P.MU.acc_elt<type_sew_t<x * mult>::type>(td_num + m, \
          0, i, j, mmax, nmax / mult * amul, reg_rename, true); \
  MX_PARAM_BASE(x) \

#define MMULX2D_PARAMS(x, mult) \
  MX_ACC_PARAM2D_BASE(x) \

#define MMULXXU_PARAMS(x, mult) \
  type_sew_t<x * mult>::type &accd  = P.MU.tr_elt<type_sew_t<x * mult>::type>(td_num + m, \
          0, i, j, mmax, nmax / mult * amul, reg_rename, true); \
  MX_ACC1_PARAMS(x) \
  MXU_ACC2_PARAMS(x) \

#define MXXU_PARAMS(x) \
  type_sew_t<x>::type &td  = P.MU.acc_elt<type_sew_t<x>::type>(td_num + m, 0, i, j, mmax, nmax * amul, reg_rename, true); \
  type_sew_t<x>::type ts1  = P.MU.acc_elt<type_sew_t<x>::type>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false); \
  type_usew_t<x>::type ts2  = P.MU.acc_elt<type_usew_t<x>::type>(ts2_num + m, 0, i, j, mmax, nmax * amul, false, false); \

#define MXUX_PARAMS(x) \
  type_sew_t<x>::type &td  = P.MU.tr_elt<type_sew_t<x>::type>(td_num + m, 0, i, j, mmax, nmax, reg_rename, true); \
  type_usew_t<x>::type &ts1  = P.MU.tr_elt<type_usew_t<x>::type>(ts1_num + m, 0, i, k, mmax, nmax, false, false); \
  type_sew_t<x>::type &ts2  = P.MU.tr_elt<type_sew_t<x>::type>(ts2_num + m, 0, k, j, mmax, nmax, false, false); \

#define MXDSU_PARAMS(x) \
  type_usew_t<x>::type ts1  = P.MU.tr_elt<type_usew_t<x>::type>(ts1_num + m, 0, i, k, mmax, nmax, false, false); \
  type_usew_t<x>::type ts2  = P.MU.tr_elt<type_usew_t<x>::type>(ts2_num + m, 0, k, j, mmax, nmax, false, false); \

#define MXDS_PARAMS(x) \
  type_sew_t<x>::type &ts1  = P.MU.tr_elt<type_sew_t<x>::type>(ts1_num + m, 0, i, k, mmax, nmax, false, false); \
  type_sew_t<x>::type &ts2  = P.MU.tr_elt<type_sew_t<x>::type>(ts2_num + m, 0, k, j, mmax, nmax, false, false); \

#define MXX_PARAMS_TR1(x) \
  type_sew_t<x>::type &ts1  = P.MU.tr_elt<type_sew_t<x>::type>(ts1_num + m, 0, i, j, mmax, nmax, false, false); \

#define MXX_PARAMS_ACC1(x) \
  type_sew_t<x>::type &acc1  = P.MU.acc_elt<type_sew_t<x>::type>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false); \

#define MFU_PARAMS_ACC1(width) \
  float##width##_t UNUSED &ts1  = P.MU.acc_elt<float##width##_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false); \

#define MFU_PARAMS_ACC2(width) \
  float##width##_t UNUSED &ts2  = P.MU.acc_elt<float##width##_t>(ts2_num + m, 0, i, j, mmax, nmax * amul, false, false); \

#define MBFU_PARAMS_ACC1(width) \
  bfloat##width##_t UNUSED &ts1  = P.MU.acc_elt<bfloat##width##_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false); \

#define MBFU_PARAMS_ACC2(width) \
  bfloat##width##_t UNUSED &ts2  = P.MU.acc_elt<bfloat##width##_t>(ts2_num + m, 0, i, j, mmax, nmax * amul, false, false); \

#define MTFU_PARAMS_ACC1(width) \
  tfloat##width##_t UNUSED &ts1  = P.MU.acc_elt<tfloat##width##_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false); \

#define MTFU_PARAMS_ACC2(width) \
  tfloat##width##_t UNUSED &ts2  = P.MU.acc_elt<tfloat##width##_t>(ts2_num + m, 0, i, j, mmax, nmax * amul, false, false); \

#define MFU_PARAMS_TR1(width) \
  float##width##_t UNUSED &ts1  = P.MU.tr_elt<float##width##_t>(ts1_num + m, 0, i, j, mmax, nmax, false, false); \

#define MFU_PARAMS_TR2(width) \
  float##width##_t UNUSED &ts2  = P.MU.tr_elt<float##width##_t>(ts2_num + m, 0, i, j, mmax, nmax, false, false); \

#define MBFU_PARAMS_TR1(width) \
  bfloat##width##_t UNUSED &ts1  = P.MU.tr_elt<float##width##_t>(ts1_num + m, 0, i, j, mmax, nmax, false, false); \

#define MBFU_PARAMS_TR2(width) \
  bfloat##width##_t UNUSED &ts2  = P.MU.tr_elt<float##width##_t>(ts2_num + m, 0, i, j, mmax, nmax, false, false); \

#define MFU_MM_PARAMS(width, wide) \
  float##width##_t &td = P.MU.acc_elt<float##width##_t>(td_num + m, 0, i, j, mmax, nmax * amul * wide, reg_rename, true); \
  MFU_PARAMS_ACC1(width) \
  MFU_PARAMS_ACC2(width) \

#define MBFU_MM_PARAMS(width, wide) \
  bfloat##width##_t &td = P.MU.acc_elt<bfloat##width##_t>(td_num + m, 0, i, j, mmax, nmax * amul * wide, reg_rename, true); \
  MBFU_PARAMS_ACC1(width) \
  MBFU_PARAMS_ACC2(width) \

#define MTFU_MM_PARAMS(width, wide) \
  tfloat##width##_t &td = P.MU.acc_elt<tfloat##width##_t>(td_num + m, 0, i, j, mmax, nmax * amul * wide, reg_rename, true); \
  MTFU_PARAMS_ACC1(width) \
  MTFU_PARAMS_ACC2(width) \

#define REGNAME_WRITE_BAKE(tr_mmax, tr_nmax, reg_sum, is_acc) \
  if (reg_rename) { \
    P.MU.reg_rename_write_back_elt(td_num, tr_mmax, tr_nmax, lmul, reg_sum, is_acc); \
  } \

#define MD2X_LOOP_BASE \
  for (reg_t i = 0; i < height; ++i) { \
    for (reg_t m = 0; m < lmul; m++) { \
      for (reg_t j = 0; j < width; ++j) { \

#define MD2X_LOOP(type, SRC, DST) \
  MX_##SRC##1_##DST##D_PARAMS(type) \
  MD2X_LOOP_BASE \

#define MD2X_LOOP_END \
      } \
    } \
  } \

#define MMV_TR_XPR(BODY, num, sew) \
  require(sew >= e8 && sew <= e64); \
  reg_t i = RS2 & 0xFF; \
  reg_t j = (RS2 >> 16); \
  reg_t ts1_num = num; \
  reg_t mmax = P.MU.mrows; \
  reg_t nmax = P.MU.mcols / sew; \
  reg_t m = 0; \
  reg_t amul = P.MU.mamul; \
  if (i >mmax || j > nmax) \
    require(0); \
  BODY \

#define MMV_ACC_XPR(BODY, num, sew) \
  require(sew >= e8 && sew <= e64); \
  reg_t i = RS2 & 0xFF; \
  reg_t j = (RS2 >> 16); \
  reg_t ts1_num = num; \
  reg_t mmax = P.MU.mrows; \
  reg_t nmax = P.MU.mcols / sew; \
  reg_t amul = P.MU.mamul; \
  reg_t m = 0; \
  if (i >mmax || j > nmax) \
    require(0); \
  BODY \

#define MMV_TR_AND_ACC(BODY, sew, is_acc) \
  require(sew >= e8 && sew <= e64); \
  reg_t ts1_num = insn.rs1(); \
  reg_t td_num = insn.rd(); \
  reg_t mmax = P.MU.mrows; \
  reg_t nmax = P.MU.mcols / sew; \
  reg_t height = P.MU.tile_m->read(); \
  reg_t amul = P.MU.mamul; \
  reg_t width = P.MU.tile_n->read(); \
  reg_t lmul = 1; \
  if (ts1_num == td_num) \
    return 0; \

#define MMV_TR_ACC_INTERTRANS_BASE(REG_VAL, sew) \
  require(sew >= e8 && sew <= e64); \
  reg_t mmax = P.MU.mrows; \
  reg_t nmax = P.MU.mcols / sew; \
  reg_t td_num = insn.rd(); \
  reg_t ts1_num = insn.rs1(); \
  reg_t amul = P.MU.mamul; \
  reg_t height = P.MU.tile_m->read(); \
  reg_t width = P.MU.tile_n->read(); \
  reg_t start_width = REG_VAL ; \
  reg_t lmul = 1; \
  bool reg_rename = false; \
  
#define MMV_TR_ACC_INTERTRANS(BODY, sew, REG_VAL) \
  MMV_TR_ACC_INTERTRANS_BASE(REG_VAL, sew) \
  MD2X_LOOP_BASE \
  BODY; \
  MD2X_LOOP_END \


#define MMV_TR_ACC_BASE(BODY, num, sew) \
  require(sew >= e8 && sew <= e64); \
  reg_t i = RS2 & 0xFF; \
  reg_t j = (RS2 >> 16); \
  reg_t ts1_num = num; \
  reg_t mmax = P.MU.mrows; \
  reg_t nmax = P.MU.mcols / sew; \
  reg_t amul = P.MU.mamul; \
  reg_t lmul = 1; \
  reg_t m = 0; \
  bool reg_rename = false; \
  BODY \

#define MMV_TR_ACC_FPR(BODY, num, sew) \
  MMV_TR_ACC_BASE(num, sew) \
  BODY; \


#define MTU_MV_LEN(trans, dim) \
  switch (dim) \
  { \
  case 'c' : \
    height = trans? P.MU.tile_n->read() : P.MU.tile_m->read(); \
    width = trans? P.MU.tile_m->read() : P.MU.tile_n->read(); \
    rmax = P.MU.mrows; \
    cmax = (P.MU.mcols / P.MU.msew); \
    break; \
  case 'a' : \
    height = trans? P.MU.tile_k->read() : P.MU.tile_m->read(); \
    width = trans? P.MU.tile_m->read() : P.MU.tile_k->read(); \
    rmax = P.MU.mrows; \
    cmax = (P.MU.mcols / P.MU.msew); \
    break; \
  case 'b' : \
    height = trans? P.MU.tile_n->read() : P.MU.tile_k->read(); \
    width = trans? P.MU.tile_k->read() : P.MU.tile_n->read(); \
    cmax = (P.MU.mcols / P.MU.msew); \
    rmax = P.MU.mrows; \
    break; \
  default : \
    break; \
  }; \


#define MMV_GENERAL_LOOP_BASE(is_trans, dim, sew, is_tr_to_v) \
  require(sew >= e8 && sew <= e64); \
  reg_t rd_num = is_tr_to_v ? insn.rd(): insn.td(); \
  reg_t rs1_num = is_tr_to_v ? insn.ts1(): insn.rs1(); \
  reg_t start_height = RS2; \
  reg_t height, width; \
  reg_t rmax = 0, cmax = 0; \
  reg_t amul = P.MU.mamul; \
  /* reg_t lmul = P.MU.mlmul; \ */ \
  reg_t lmul = 1; \
  require_align(insn.rs1(), lmul); \
  MTU_MV_LEN(is_trans, dim); \
  if (height < start_height) \
    require(0); \
  for (reg_t i = 0; i < height - start_height; i++){ \
    for (reg_t m = 0 ; m < lmul; m++) { \
      for (reg_t j = 0; j < width; j++) { \

#define MTU_VREG_TR_PARAMS(trans, x) \
  type_sew_t<x>::type &vd = P.VU.elt<type_sew_t<x>::type>(rd_num + m, i*width+j, true); \
  type_sew_t<x>::type ts1 = P.MU.tr_elt<type_sew_t<x>::type>(rs1_num + m, trans, i + start_height, j, rmax, cmax, false, false); \

#define MTU_VREG_ACC_PARAMS(trans, x) \
  type_sew_t<x>::type &vd = P.VU.elt<type_sew_t<x>::type>(rd_num, i*width+j, true); \
  type_sew_t<x>::type ts1 = P.MU.acc_elt<type_sew_t<x>::type>(rs1_num, trans, i+start_height, j, rmax, cmax * amul, false, false); \

#define MTU_TR_VREG_PARAMS(trans, x) \
  type_sew_t<x>::type vs1 = P.VU.elt<type_sew_t<x>::type>(rs1_num + m, i*width+j); \
  type_sew_t<x>::type &td = P.MU.tr_elt<type_sew_t<x>::type>(rd_num + m, trans, i + start_height, j, rmax, cmax, false, true); \

#define MTU_ACC_VREG_PARAMS(trans, x) \
  type_sew_t<x>::type vs1 = P.VU.elt<type_sew_t<x>::type>(rs1_num, i*width+j); \
  type_sew_t<x>::type &td = P.MU.acc_elt<type_sew_t<x>::type>(rd_num, trans, i+start_height, j, rmax, cmax * amul, false, true); \

#define MTU_VM_LOOP_END \
    } \
  } \
  } \

// vreg <-- tr
#define MMV_VREG_FROM_TR(is_trans, dim, sew, REG) \
  /* require(P.MU.msew == P.VU.vsew); */ \
  MMV_GENERAL_LOOP_BASE(is_trans, dim, sew, true) \
  if (sew == e8){ \
    MTU_VREG_##REG##_PARAMS(is_trans, e8); \
    vd = ts1; \
  }else if(sew == e16){ \
    MTU_VREG_##REG##_PARAMS(is_trans, e16); \
    vd = ts1; \
  }else if(sew == e32){ \
    MTU_VREG_##REG##_PARAMS(is_trans, e32); \
    vd = ts1; \
  }else if(sew == e64){ \
    MTU_VREG_##REG##_PARAMS(is_trans, e64); \
    vd = ts1; \
  } \
  MTU_VM_LOOP_END 

// tr <-- vreg
#define MMV_TR_FROM_VREG(is_trans, dim, sew, REG) \
  /* require(P.MU.msew == P.VU.vsew); */ \
  MMV_GENERAL_LOOP_BASE(is_trans, dim, sew, false) \
  if (sew == e8){ \
    MTU_##REG##_VREG_PARAMS(is_trans, e8); \
    td = vs1; \
  }else if(sew == e16){ \
    MTU_##REG##_VREG_PARAMS(is_trans, e16); \
    td = vs1; \
  }else if(sew == e32){ \
    MTU_##REG##_VREG_PARAMS(is_trans, e32); \
    td = vs1; \
  }else if(sew == e64){ \
    MTU_##REG##_VREG_PARAMS(is_trans, e64); \
    td = vs1; \
  } \
  MTU_VM_LOOP_END 

#define MXU_GENERAL_LOOP_BASE(ins, sew) \
  require(sew >= e8 && sew <= e64); \
  reg_t tile_m = P.MU.tile_m->read();\
  reg_t tile_k = P.MU.tile_k->read();\
  reg_t tile_n = P.MU.tile_n->read();\
  reg_t mmax = P.MU.mrows;\
  reg_t nmax = P.MU.mcols / sew;\
  reg_t kmax = std::min(nmax, P.MU.mrows);\
  reg_t td_num = insn.td(); \
  reg_t ts1_num = insn.ts1(); \
  reg_t ts2_num = insn.ts2(); \
  reg_t amul = P.MU.mamul; \
  /* reg_t lmul = P.MU.mlmul; \ */ \
  reg_t lmul = 1; \
  reg_t reg_sum = 1; \
  /* if (ins && insn.mlmul() != LMUL_RESERVE) \
    lmul = (1 << insn.mlmul()); */ \
  bool reg_rename = false; \
  if (td_num == ts1_num || td_num == ts2_num){ \
    reg_rename = true; \
  } \
  bool only_one_fix_reg_sum = false; \
  for (reg_t i = 0; i < tile_m; ++i) { \
    for (reg_t m = 0 ; m < lmul; m++) { \
      for (reg_t j = 0; j < tile_n; ++j) { \
        for (reg_t k = 0; k < tile_k; ++k) { \

#define MXU_LOOP_ELEMENT_SKIP(BODY)

#define MXU_LOOP_BASE(sew) \
    MXU_GENERAL_LOOP_BASE(false, sew) \
    MXU_LOOP_ELEMENT_SKIP();

#define MXU_GENERAL_LOOP_BASE_WIDE(ins, wide, sew) \
  require(sew >= e8 && sew <= e64); \
  reg_t tile_m = P.MU.tile_m->read();\
  reg_t tile_k = P.MU.tile_k->read();\
  reg_t tile_n = P.MU.tile_n->read();\
  reg_t mmax = P.MU.mrows;\
  reg_t nmax = P.MU.mcols / sew;\
  reg_t kmax = std::min(nmax, P.MU.mrows);\
  reg_t td_num = insn.td(); \
  reg_t ts1_num = insn.ts1(); \
  reg_t ts2_num = insn.ts2(); \
  reg_t amul = P.MU.mamul; \
  bool reg_rename = false; \
  /* reg_t lmul = P.MU.mlmul; \ */ \
  reg_t lmul = 1; \
  /* if (ins && insn.mlmul() != LMUL_RESERVE) \
    lmul = (1 << insn.mlmul()); */ \
  reg_t des_nmax = nmax / wide; \
  for (reg_t i = 0; i < tile_m; ++i) { \
    for (reg_t m = 0 ; m < lmul; m++) { \
      for (reg_t j = 0; j < tile_n; ++j) { \
        for (reg_t k = 0; k < tile_k; ++k) { \

#define MXU_LOOP_BASE_WIDE(wide, sew) \
    MXU_GENERAL_LOOP_BASE_WIDE(false, wide, sew) \
    MXU_LOOP_ELEMENT_SKIP();

#define MXU_LOOP_END \
        } \
      } \
    } \
  } \

#define MXU_VFP_LOOP_END \
  MXU_LOOP_END \


#define MXU_MM_LOOP(BODY, TYPE, sew) \
  MXU_LOOP_BASE(sew) \
  if (sew == e8){ \
    M##TYPE##_ACCD_PARAMS(e8); \
    BODY; \
  }else if(sew == e16){ \
    M##TYPE##_ACCD_PARAMS(e16); \
    BODY; \
  }else if(sew == e32){ \
    M##TYPE##_ACCD_PARAMS(e32); \
    BODY; \
  }else if(sew == e64){ \
    M##TYPE##_ACCD_PARAMS(e64); \
    BODY; \
  } \
  MXU_LOOP_END \

#define MXU_MM_LOOP_WIDE(BODY ,PARAMS, wide, sew) \
  MXU_GENERAL_LOOP_BASE_WIDE(false, wide, sew) \
    MXU_LOOP_ELEMENT_SKIP(); \
  if (sew == e8) { \
    M##PARAMS##_PARAMS(e8, 2); \
    BODY \
  } else if (sew == e16) { \
    M##PARAMS##_PARAMS(e16, 2); \
    BODY \
  } else if (sew == e32) { \
    M##PARAMS##_PARAMS(e32, 2); \
    BODY \
  } else { \
    require(0); \
  } \
  MXU_LOOP_END \

#define MXU_CHECK_OVERFLOW(eew) \
  res = res > std::numeric_limits<eew>::max()? std::numeric_limits<eew>::max() : res; \
  res = res < std::numeric_limits<eew>::min()? std::numeric_limits<eew>::min() : res; \
  

#define MX_2D_GENERAL_LOOP_BASE(ins, el_sew) \
  require(P.MU.msew >= e8 && P.MU.msew <= e64); \
  reg_t sew = el_sew; \
  reg_t tile_m = P.MU.tile_m->read();\
  reg_t tile_n = P.MU.tile_n->read();\
  reg_t mmax = P.MU.mrows;\
  reg_t nmax = P.MU.mcols / sew;\
  reg_t td_num = insn.td(); \
  reg_t ts1_num = insn.ts1(); \
  reg_t ts2_num = insn.ts2(); \
  /* reg_t lmul = P.MU.mlmul; \*/ \
  reg_t reg_sum = 1; \
  reg_t lmul = 1; \
  /* if (ins && insn.mlmul() != LMUL_RESERVE) \
    lmul = (1 << insn.mlmul()); */ \
  bool reg_rename = false; \
  if (td_num == ts1_num || td_num == ts2_num){ \
    reg_rename = true; \
  } \
  reg_t amul = P.MU.mamul; \
  for (reg_t i = 0; i < tile_m; ++i) { \
    for (reg_t m = 0 ; m < lmul; m++) { \
      for (reg_t j = 0; j < tile_n; ++j) { \

#define MX_2D_LOOP_ELEMENT_SKIP(BODY)

#define MX_2D_LOOP_BASE(ins, sew) \
    MX_2D_GENERAL_LOOP_BASE(ins, sew) \
    MX_2D_LOOP_ELEMENT_SKIP();

#define MX_2D_LOOP_END \
      } \
    } \
  } \

#define MX_2D_GENERAL_LOOP_BASE_WIDE(ins, sew) \
  require(sew >= e8 && sew <= e64); \
  reg_t tile_m = P.MU.tile_m->read();\
  reg_t tile_n = P.MU.tile_n->read();\
  reg_t mmax = P.MU.mrows;\
  reg_t nmax = P.MU.mcols / sew;\
  reg_t td_num = insn.td(); \
  reg_t ts1_num = insn.ts1(); \
  reg_t ts2_num = insn.ts2(); \
  /* reg_t lmul = P.MU.mlmul; \ */ \
  reg_t reg_sum = 1; \
  reg_t lmul = 1; \
  reg_t amul = P.MU.mamul; \
  /* if (ins && insn.mlmul() != LMUL_RESERVE) \
    lmul = (1 << insn.mlmul()); */ \
  bool reg_rename = false; \
  if (td_num == ts1_num || td_num == ts2_num){ \
    reg_rename = true; \
  } \
  reg_t des_nmax = nmax / 2; \
  for (reg_t i = 0; i < tile_m; ++i) { \
    for (reg_t m = 0 ; m < lmul; m++) { \
      for (reg_t j = 0; j < tile_n; ++j) { \

#define MX_2D_LOOP_BASE_WIDEN(ins, sew) \
    MX_2D_GENERAL_LOOP_BASE_WIDE(ins, sew) \
    MX_2D_LOOP_ELEMENT_SKIP(); \

#define BIT4_SIGN_EXTEND_INT8(T, vale) \
  if (std::is_signed<T>::value && (vale & 0x8)) { \
    vale |= 0xF0; \
  } \

#define MXU_MM_ADD(opd, op0, sign, type_t, td_type, saturated, sew) \
  require(sew >= e4 && sew <= e64); \
  /* reg_t lmul = insn.mlmul() != LMUL_RESERVE ? (1 << insn.mlmul()) : P.MU.mlmul; */ \
  reg_t lmul = 1; \
  reg_t tile_m = P.MU.tile_m->read() * lmul;\
  reg_t tile_n = P.MU.tile_n->read() * lmul;\
  reg_t mmax = P.MU.mrows;\
  reg_t nmax = P.MU.mcols / sew;\
  reg_t amul = P.MU.mamul; \
  reg_t td_num = insn.td(); \
  reg_t ts1_num = insn.ts1(); \
  reg_t ts2_num = insn.ts2(); \
  type_t res; \
  bool reg_rename = false; \
  if (td_num == ts1_num || td_num == ts2_num){ \
    reg_rename = true; \
  } \
  for (reg_t i = 0; i < tile_m; ++i) { \
    for (reg_t m = 0; m < lmul; m++) {\
      for (reg_t j = 0; j < tile_n; ++j) { \
        if (sew == e4) { \
          auto &td = P.MU.acc_elt<sign<e8>::type>(td_num + m , 0, i, j, mmax, nmax * amul / 2, reg_rename, true); \
          auto ts1  = P.MU.acc_elt<sign<e8>::type>(ts1_num + m, 0, i, j, mmax, nmax * amul / 2, false, false); \
          auto ts2  = P.MU.acc_elt<sign<e8>::type>(ts2_num + m, 0, i, j, mmax, nmax * amul / 2, false, false); \
          bit4_pair_t<sign<e8>::type> bit4_pair_ts1; \
          bit4_pair_t<sign<e8>::type> bit4_pair_ts2; \
          if (saturated) { \
            bit4_pair_ts1 = bit4_pair_t<sign<e8>::type>(ts1, true); \
            bit4_pair_ts2 = bit4_pair_t<sign<e8>::type>(ts2, true); \
          } else { \
            bit4_pair_ts1 = ts1; \
            bit4_pair_ts2 = ts2; \
          } \
          auto result = bit4_pair_ts1 op0 bit4_pair_ts2; \
          td = static_cast<td_type##8_t>(result); \
        } else if (sew == e8){ \
          auto &td = P.MU.acc_elt<sign<e8>::type>(td_num + m , 0, i, j, mmax, nmax * amul, reg_rename, true); \
          auto ts1  = P.MU.acc_elt<sign<e8>::type>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false); \
          auto ts2  = P.MU.acc_elt<sign<e8>::type>(ts2_num + m, 0, i, j, mmax, nmax * amul, false, false); \
          res = (type_t)ts1 op0 (type_t)ts2; \
          if (saturated) { \
            MXU_CHECK_OVERFLOW(td_type##8_t) \
          } \
          td = (td_type##8_t)res; \
        }else if(sew == e16){ \
          auto &td = P.MU.acc_elt<sign<e16>::type>(td_num + m, 0, i, j, mmax, nmax * amul, reg_rename, true); \
          auto ts1  = P.MU.acc_elt<sign<e16>::type>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false); \
          auto ts2  = P.MU.acc_elt<sign<e16>::type>(ts2_num + m, 0, i, j, mmax, nmax * amul, false, false); \
          res = (type_t)ts1 op0 (type_t)ts2; \
          if (saturated) { \
            MXU_CHECK_OVERFLOW(td_type##16_t) \
          } \
          td = (td_type##16_t)res; \
        }else if(sew == e32){ \
          auto &td = P.MU.acc_elt<sign<e32>::type>(td_num + m, 0, i, j, mmax, nmax * amul, reg_rename, true); \
          auto ts1  = P.MU.acc_elt<sign<e32>::type>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false); \
          auto ts2  = P.MU.acc_elt<sign<e32>::type>(ts2_num + m, 0, i, j, mmax, nmax * amul, false, false); \
          res = (type_t)ts1 op0 (type_t)ts2; \
          if (saturated) { \
            MXU_CHECK_OVERFLOW(td_type##32_t) \
          } \
          td = (td_type##32_t)res; \
        }else if(sew == e64){ \
          auto &td = P.MU.acc_elt<sign<e64>::type>(td_num + m, 0, i, j, mmax, nmax * amul, reg_rename, true); \
          auto ts1  = P.MU.acc_elt<sign<e64>::type>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false); \
          auto ts2  = P.MU.acc_elt<sign<e64>::type>(ts2_num + m, 0, i, j, mmax, nmax * amul, false, false); \
          res = (type_t)ts1 op0 (type_t)ts2; \
          if (saturated) { \
            MXU_CHECK_OVERFLOW(td_type##64_t) \
          } \
          td = (td_type##64_t)res; \
        } \
      } \
    } \
  } \
  REGNAME_WRITE_BAKE(mmax, sew == e4 ? nmax * amul / 2: nmax * amul, 1, true); \

#define MXU_W_MM_ADD(opd, op0, sign, type_t, td_type, saturated, sew) \
  require(sew >= e4 && sew <= e32); \
  /* reg_t lmul = insn.mlmul() != LMUL_RESERVE ? (1 << insn.mlmul()) : P.MU.mlmul; */ \
  reg_t lmul = 1; \
  reg_t tile_m = P.MU.tile_m->read();\
  reg_t tile_n = P.MU.tile_n->read();\
  reg_t td_num = insn.td(); \
  reg_t ts1_num = insn.ts1(); \
  reg_t ts2_num = insn.ts2(); \
  reg_t mmax = P.MU.mrows;\
  reg_t nmax = P.MU.mcols / sew;\
  type_t res; \
  reg_t amul = P.MU.mamul; \
  bool reg_rename = false; \
  if (td_num == ts1_num || td_num == ts2_num){ \
    reg_rename = true; \
  } \
  reg_t des_nmax = nmax/2; \
  for (reg_t i = 0; i < tile_m; ++i) { \
    for (reg_t m = 0; m < lmul; m++) { \
      for (reg_t j = 0; j < tile_n; ++j) { \
        if (sew == e4) { \
          auto &td = P.MU.acc_elt<sign<e8>::type>(td_num + m, 0, i, j * 2, mmax, des_nmax * amul, reg_rename, true); \
          auto ts1  = P.MU.acc_elt<sign<e8>::type>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false); \
          auto ts2  = P.MU.acc_elt<sign<e8>::type>(ts2_num + m, 0, i, j, mmax, nmax * amul, false, false); \
          auto ts1_low = ts1 & 0xF; \
          auto ts2_low = ts2 & 0xF; \
          auto ts1_height = ts1 >> 0x4; \
          auto ts2_height = ts2 >> 0x4; \
          BIT4_SIGN_EXTEND_INT8(sign<e8>::type, ts1_low); \
          BIT4_SIGN_EXTEND_INT8(sign<e8>::type, ts2_low); \
          BIT4_SIGN_EXTEND_INT8(sign<e8>::type, ts1_height); \
          BIT4_SIGN_EXTEND_INT8(sign<e8>::type, ts2_height); \
          res = (type_t)ts1_low op0 (type_t)ts2_low; \
          if (saturated) { \
            MXU_CHECK_OVERFLOW(td_type##8_t) \
          } \
          td = (td_type##16_t)res; \
          auto &td2 = P.MU.acc_elt<sign<e8>::type>(td_num + m, 0, i, j * 2 + 1, mmax, des_nmax * amul, reg_rename, true); \
          res = (type_t)ts1_height op0 (type_t)ts2_height; \
          if (saturated) { \
            MXU_CHECK_OVERFLOW(td_type##8_t) \
          } \
          td2 = (td_type##16_t)res; \
        } else if (sew == e8){ \
          auto &td = P.MU.acc_elt<sign<e16>::type>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true); \
          auto ts1  = P.MU.acc_elt<sign<e8>::type>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false); \
          auto ts2  = P.MU.acc_elt<sign<e8>::type>(ts2_num + m, 0, i, j, mmax, nmax * amul, false, false); \
          res = (type_t)ts1 op0 (type_t)ts2; \
          if (saturated) { \
            MXU_CHECK_OVERFLOW(td_type##16_t) \
          } \
          td = (td_type##16_t)res; \
        }else if(sew == e16){ \
          auto &td = P.MU.acc_elt<sign<e32>::type>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true); \
          auto ts1  = P.MU.acc_elt<sign<e16>::type>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false); \
          auto ts2  = P.MU.acc_elt<sign<e16>::type>(ts2_num + m, 0, i, j, mmax, nmax * amul, false, false); \
          res = (type_t)ts1 op0 (type_t)ts2; \
          if (saturated) { \
            MXU_CHECK_OVERFLOW(td_type##32_t) \
          } \
          td = (td_type##32_t)res; \
        }else if(sew == e32){ \
          auto &td = P.MU.acc_elt<sign<e64>::type>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true); \
          auto ts1  = P.MU.acc_elt<sign<e32>::type>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false); \
          auto ts2  = P.MU.acc_elt<sign<e32>::type>(ts2_num + m, 0, i, j, mmax, nmax * amul, false, false); \
          res = (type_t)ts1 op0 (type_t)ts2; \
          if (saturated) { \
            MXU_CHECK_OVERFLOW(td_type##64_t) \
          } \
          td = (td_type##64_t)res; \
        }\
      } \
    } \
  } \
  REGNAME_WRITE_BAKE(mmax, sew == e4 ? nmax * amul / 2: nmax * amul , 1, true); \

#define MXU_Q_MM_ADD(opd, op0, sign, type, td_type) \
  require(P.MU.msew >= e8 && P.MU.msew <= e16); \
  reg_t tile_m = P.MU.tile_m->read();\
  reg_t tile_n = P.MU.tile_n->read();\
  reg_t sew = P.MU.msew; \
  reg_t td_num = insn.td(); \
  reg_t ts1_num = insn.ts1(); \
  reg_t ts2_num = insn.rs2(); \
  type res; \
  bool reg_rename = false; \
  if (td_num == ts1_num || td_num == ts2_num){ \
    reg_rename = true; \
  } \
  reg_t des_nmax = nmax / 4; \
  reg_t reg_sum = 1 + (tile_n - 1) / des_nmax; \
  reg_t td_num_lmul = 0; \
  for (reg_t i = 0; i < tile_m; ++i) { \
    td_num_lmul = 0; \
    for (reg_t m = 0; m < lmul; m++) { \
      td_num_lmul += (m != 0) ? ((tile_n - 1) / des_nmax) : 0; \
      for (reg_t j = 0; j < tile_n; ++j) { \
      if (sew == e8){ \
        auto &td = P.MU.tr_elt<sign<e32>::type>(td_num + m + td_num_lmul + j / des_nmax, 0, i, j % des_nmax, mmax, des_nmax, reg_rename, true); \
        auto ts1  = P.MU.tr_elt<sign<e8>::type>(ts1_num + m, 0, i, j, mmax, nmax, false, false); \
        auto ts2  = P.MU.tr_elt<sign<e8>::type>(ts2_num + m, 0, i, j, mmax, nmax, false, false); \
        res = (type)ts1 op0 (type)ts2; \
        MXU_CHECK_OVERFLOW(td_type##32_t) \
        td = (td_type##32_t)res; \
      }else if(sew == e16){ \
        auto &td = P.MU.tr_elt<sign<e64>::type>(td_num + m + td_num_lmul + j / des_nmax, 0, i, j % des_nmax, mmax, des_nmax, reg_rename, true); \
        auto ts1  = P.MU.tr_elt<sign<e16>::type>(ts1_num + m, 0, i, j, mmax, nmax, false, false); \
        auto ts2  = P.MU.tr_elt<sign<e16>::type>(ts2_num + m, 0, i, j, mmax, nmax, false, false); \
        res = (type)ts1 op0 (type)ts2; \
        MXU_CHECK_OVERFLOW(td_type##64_t) \
        td = (td_type##64_t)res; \
      }\
    } \
  } \
  } \
  REGNAME_WRITE_BAKE(mmax, nmax, reg_sum, true); \



#define MXU_WIDE_OP_AND_ASSIGN(var0, var1, var2, op0, op1, sign, sew) \
  switch(sew) { \
  case e4: { \
    sign##8_t td_w = P.MU.acc_elt<sign##8_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, false); \
    bit4_pair_t <sign##8_t> var0_bit4_pair = var0; \
    bit4_pair_t <sign##8_t> var1_bit4_pair = var1; \
    P.MU.acc_elt<sign##8_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true) = \
      op1((sign##8_t)var0_bit4_pair op0 (sign##8_t)var1_bit4_pair) + var2; \
    } \
    break; \
  case e8: { \
    sign##16_t td_w = P.MU.acc_elt<sign##16_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, false); \
    P.MU.acc_elt<sign##16_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true) = \
      op1((sign##16_t)(sign##8_t)var0 op0 (sign##16_t)(sign##8_t)var1) + var2; \
    } \
    break; \
  case e16: { \
    sign##32_t td_w = P.MU.acc_elt<sign##32_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, false); \
    P.MU.acc_elt<sign##32_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true) = \
      op1((sign##32_t)(sign##16_t)var0 op0 (sign##32_t)(sign##16_t)var1) + var2; \
    } \
    break; \
  default: { \
    sign##64_t td_w = P.MU.acc_elt<sign##64_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, false); \
    P.MU.acc_elt<sign##64_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true) = \
      op1((sign##64_t)(sign##32_t) var0 op0 (sign##64_t)(sign##32_t)var1) + var2; \
    } \
    break; \
  }

#define MXU_WIDE_OP_AND_ASSIGN_MIX(var0, var1, var2, op0, op1, signd, sign_1, sign_2, sew) \
  switch(sew) { \
  case e8: { \
    signd##16_t UNUSED td_w = P.MU.acc_elt<signd##16_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, false); \
    P.MU.acc_elt<signd##16_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true) = \
      op1((sign_1##16_t)(sign_1##8_t)var0 op0 (sign_2##16_t)(sign_2##8_t)var1) + var2; \
    } \
    break; \
  case e16: { \
    signd##32_t td_w = P.MU.acc_elt<signd##32_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, false); \
    P.MU.acc_elt<signd##32_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true) = \
      op1((sign_1##32_t)(sign_1##16_t)var0 op0 (sign_2##32_t)(sign_2##16_t)var1) + var2; \
    } \
    break; \
  default: { \
    signd##64_t td_w = P.MU.acc_elt<signd##64_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, false); \
    P.MU.acc_elt<signd##64_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true) = \
      op1((sign_1##64_t)(sign_1##32_t) var0 op0 (sign_2##64_t)(sign_2##32_t)var1) + var2; \
    } \
    break; \
  }

#define MXU_QUAD_OP_AND_ASSIGN(var0, var1, var2, op0, op1, sign, sew) \
  switch(sew) { \
  case e8: { \
    sign##32_t td_q = P.MU.acc_elt<sign##32_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, false, false); \
    P.MU.acc_elt<sign##32_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, false, true) = \
      op1((sign##32_t)(sign##8_t)var0 op0 (sign##32_t)(sign##8_t)var1) + var2; \
    } \
    break; \
  case e16: { \
    sign##64_t td_q = P.MU.acc_elt<sign##64_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, false, false); \
    P.MU.acc_elt<sign##64_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, false, true) = \
      op1((sign##64_t)(sign##16_t)var0 op0 (sign##64_t)(sign##16_t)var1) + var2; \
    } \
    break; \
  }

#define MXU_OCT_OP_AND_ASSIGN(var0, var1, var2, op0, op1, sign, sew) \
  switch(sew) { \
  case e4: { \
    sign##32_t td_o = P.MU.acc_elt<sign##32_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, false, false); \
    P.MU.acc_elt<sign##32_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, false, true) = \
      op1((sign##32_t)(sign##8_t)(var0 & 0xFF) op0 (sign##32_t)(sign##8_t)(var1 & 0xFF) op1 \
          (sign##32_t)(sign##8_t)(var0 >> 4) op0 (sign##32_t)(sign##8_t)(var1 >> 4)) + var2; \
    } \
    break; \
  case e8: { \
    sign##64_t td_o = P.MU.acc_elt<sign##64_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, false, false); \
    P.MU.acc_elt<sign##64_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, false, true) = \
      op1((sign##64_t)(sign##8_t)var0 op0 (sign##64_t)(sign##8_t)var1) + var2; \
    } \
    break; \
  }


#define VM_WIDE_CHECK_COMMON \
  require_matrix(false);\

#define VM_CHECK_DSS(is_vs1) \
  VM_WIDE_CHECK_COMMON; \

// widen operation loop
#define MXU_VV_LOOP_WIDEN(BODY, wide, sew) \
  MXU_LOOP_BASE_WIDE(wide, sew) \
  if (sew == e8 || sew == e4){ \
    MX_ACCD_PARAMS(e8); \
    BODY; \
  } else if(sew == e16){ \
    MX_ACCD_PARAMS(e16); \
    BODY; \
  } else if(sew == e32){ \
    MX_ACCD_PARAMS(e32); \
    BODY; \
  } else { /* type not support*/ \
    require(0); \
  } \
  MXU_LOOP_END
  

#define MXU_GENERAL_LOOP_BASE_OCT(wide, sew) \
  require(sew >= e8 && sew <= e64); \
  reg_t tile_m = P.MU.tile_m->read();\
  reg_t tile_k = P.MU.tile_k->read();\
  reg_t tile_n = P.MU.tile_n->read();\
  reg_t mmax = P.MU.mrows;\
  reg_t nmax = P.MU.mcols / sew; \
  reg_t kmax = std::min(nmax, P.MU.mrows); \
  reg_t td_num = insn.td(); \
  reg_t ts1_num = insn.ts1(); \
  reg_t ts2_num = insn.rs2(); \
  reg_t amul = P.MU.mamul; \
  /* reg_t lmul = P.MU.mlmul; \ */ \
  reg_t lmul = 1; \
  /* if (ins && insn.mlmul() != LMUL_RESERVE) \
    lmul = (1 << insn.mlmul()); */ \
  reg_t des_nmax = nmax / wide; \
  for (reg_t i = 0; i < tile_m; ++i) { \
    for (reg_t m = 0 ; m < lmul; m++) { \
      for (reg_t j = 0; j < tile_n; ++j) { \
        for (reg_t k = 0; k < tile_k; ++k) { \

// widen operation loop
#define MXU_VV_LOOP_OCT(BODY, wide, sew) \
  MXU_GENERAL_LOOP_BASE_OCT(wide, sew) \
  if (sew == e8){ \
    MX_ACCD_PARAMS(e8); \
    BODY; \
  }else if(sew == e4){ \
    MX_ACCD_PARAMS(e8); \
    BODY; \
  } else if(sew == e32){ \
    MX_ACCD_PARAMS(e32); \
    BODY; \
  } else { /* type not support*/ \
    require(0); \
  } \
  MXU_LOOP_END

#define MXU_VFP_COMMON(wide, sew) \
  require_fp; \
  require((sew == e16 && p->extension_enabled(EXT_ZFH)) || \
          (sew == e32 && p->extension_enabled('F')) || \
          (sew == e64 && p->extension_enabled('D'))); \
  require_matrix(true);\
  require(STATE.frm->read() < 0x5);\
  reg_t tile_m = P.MU.tile_m->read();\
  reg_t tile_k = P.MU.tile_k->read();\
  reg_t tile_n = P.MU.tile_n->read();\
  reg_t td_num = insn.td(); \
  reg_t ts1_num = insn.ts1(); \
  reg_t ts2_num = insn.ts2(); \
  reg_t mmax = P.MU.mrows;\
  reg_t nmax = P.MU.mcols / sew;\
  reg_t kmax = std::min(nmax, P.MU.mrows);\
  /* reg_t lmul = P.MU.mlmul; */ \
  reg_t lmul = 1; \
  reg_t amul = P.MU.mamul; \
  require_align(td_num, lmul); \
  require_align(ts1_num, lmul); \
  require_align(ts2_num, lmul); \
  softfloat_roundingMode = STATE.frm->read(); \
  bool reg_rename = false; \
  if (td_num == ts1_num || td_num == ts2_num){ \
    reg_rename = true; \
  } \
  reg_t reg_sum = 1; \
  bool only_one_fix_reg_sum = false; \
  reg_t des_nmax = nmax / wide; \
  reg_t td_num_lmul = 0;

#define MXU_VFP_LOOP_BASE(wide, sew) \
  MXU_VFP_COMMON(wide, sew) \
  /*printf("m,k,n = %d, %d, %d\n", tile_m, tile_k, tile_n);*/ \
  for (reg_t i=0; i<tile_m; ++i) { \
    for (reg_t m = 0; m < lmul; m++) { \
      for (reg_t j=0; j<tile_n; ++j) { \
        for (reg_t k = 0; k < tile_k; ++k) { \

#define MXU_VFP_VV_LOOP(wide, sew) \
  MXU_VFP_LOOP_BASE(wide, sew) \
  switch(sew) { \
    case e16: { \
      if ( P.MU.mfp16 == MTYPE_FP16 ) { /* fp16 */ \
        float16_t &td = P.MU.acc_elt<float16_t>(td_num + m, 0, i, j, mmax, nmax * amul, false, true); \
        float16_t ts1 = P.MU.tr_elt<float16_t>(ts1_num + m, 0, i, k, mmax, nmax, false, false); \
        float16_t ts2 = P.MU.tr_elt<float16_t>(ts2_num + m, 0, k, j, mmax, nmax, false, false); \
        td = f16_mulAdd(ts1, ts2, td); \
      } else if (P.MU.mfp16 == MTYPE_BF16 ) { /* bf16*/ \
        bfloat16_t &td = P.MU.acc_elt<bfloat16_t>(td_num + m, 0, i, j, mmax, nmax * amul, false, true); \
        float16_t ts1 = P.MU.tr_elt<float16_t>(ts1_num + m, 0, i, k, mmax, nmax, false, false); \
        float16_t ts2 = P.MU.tr_elt<float16_t>(ts2_num + m, 0, k, j, mmax, nmax, false, false); \
        td = bf16_mulAdd(ts1, ts2, td); \
      } else {\
        require(0); /* type not supoort*/ \
      } \
      set_fp_exceptions; \
      break; \
    }\
    case e32: {\
      if ( P.MU.mfp32 == MTYPE_FP32 ) { /* fp32 */ \
        float32_t &td = P.MU.acc_elt<float32_t>(td_num + m, 0, i, j, mmax, nmax * amul, false, true); \
        float32_t ts1 = P.MU.tr_elt<float32_t>(ts1_num + m, 0, i, k, mmax, nmax, false, false); \
        float32_t ts2 = P.MU.tr_elt<float32_t>(ts2_num + m, 0, k, j, mmax, nmax, false, false); \
        td = f32_mulAdd(ts1, ts2, td); \
      } else if ( P.MU.mfp32 == MTYPE_TFP32 ) { /*tfp32*/ \
        tfloat32_t &td = P.MU.acc_elt<tfloat32_t>(td_num + m, 0, i, j, mmax, nmax * amul, false, true); \
        tfloat32_t ts1 = P.MU.tr_elt<tfloat32_t>(ts1_num + m, 0, i, k, mmax, nmax, false, false); \
        tfloat32_t ts2 = P.MU.tr_elt<tfloat32_t>(ts2_num + m, 0, k, j, mmax, nmax, false, false); \
        td = tf32_mulAdd(ts1, ts2, td); \
      } \
      set_fp_exceptions; \
      break; \
    }\
    case e64: {\
      float64_t &td = P.MU.acc_elt<float64_t>(td_num + m, 0, i, j, mmax, nmax * amul, false, true); \
      float64_t ts1 = P.MU.tr_elt<float64_t>(ts1_num + m, 0, i, k, mmax, nmax, false, false); \
      float64_t ts2 = P.MU.tr_elt<float64_t>(ts2_num + m, 0, k, j, mmax, nmax, false, false); \
      td = f64_mulAdd(ts1, ts2, td); \
      set_fp_exceptions; \
      break; \
    }\
    default: \
      require(0); \
      break; \
  }; \
  DEBUG_MVV_FP_VV; \
  MXU_VFP_LOOP_END \

#define MXU_VFP_MM_ADD(BODY_E4M3, BODY_E5M2, BODY_E3M4, BODY16, BODYB16, BODY32, BODYT32, BODY64, sew) \
  require_fp; \
  M_FLOAT_TYPE_CHECK(sew) \
  require_matrix(true);\
  require(STATE.frm->read() < 0x5);\
  reg_t tile_m = P.MU.tile_m->read();\
  reg_t tile_n = P.MU.tile_n->read();\
  reg_t td_num = insn.td(); \
  reg_t ts1_num = insn.ts1(); \
  reg_t ts2_num = insn.ts2(); \
  reg_t mmax = P.MU.mrows;\
  reg_t nmax = P.MU.mcols / sew; \
  /* reg_t lmul = insn.mlmul() != LMUL_RESERVE ? (1 << insn.mlmul()) : P.MU.mlmul; */ \
  reg_t lmul = 1; \
  reg_t amul = P.MU.mamul; \
  require_align(insn.td(), lmul); \
  require_align(insn.ts1(), lmul); \
  require_align(insn.rs2(), lmul); \
  softfloat_roundingMode = STATE.frm->read(); \
  bool reg_rename = false; \
  if (td_num == ts1_num || td_num == ts2_num){ \
    reg_rename = true; \
  } \
  reg_t reg_sum = 1; \
  for (reg_t i=0; i<tile_m; ++i) { \
    for (reg_t m = 0; m < lmul; m++) { \
      for (reg_t j=0; j<tile_n; ++j) { \
        switch(sew) { \
        case e8: { \
          switch (P.MU.mfp8) { \
            case MTYPE_FP8E4M3: { \
              auto &td = P.MU.acc_elt<float8_e4m3_t>(td_num + m, 0, i, j, mmax, nmax * amul, reg_rename, true); \
              auto ts1  = P.MU.acc_elt<float8_e4m3_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false); \
              auto ts2  = P.MU.acc_elt<float8_e4m3_t>(ts2_num + m, 0, i, j, mmax, nmax * amul, false, false); \
              BODY_E4M3; \
              break; \
            } \
            case MTYPE_FP8E5M2: { \
              auto &td = P.MU.acc_elt<float8_e5m2_t>(td_num + m, 0, i, j, mmax, nmax * amul, reg_rename, true); \
              auto ts1  = P.MU.acc_elt<float8_e5m2_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false); \
              auto ts2  = P.MU.acc_elt<float8_e5m2_t>(ts2_num + m, 0, i, j, mmax, nmax * amul, false, false); \
              BODY_E5M2; \
              break; \
            } \
            case MTYPE_FP8E3M4: { \
              auto &td = P.MU.acc_elt<float8_e3m4_t>(td_num + m, 0, i, j, mmax, nmax * amul, reg_rename, true); \
              auto ts1  = P.MU.acc_elt<float8_e3m4_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false); \
              auto ts2  = P.MU.acc_elt<float8_e3m4_t>(ts2_num + m, 0, i, j, mmax, nmax * amul, false, false); \
              BODY_E3M4; \
              break; \
            } \
            default: \
              require(0); \
              break; \
          } \
          set_fp_exceptions; \
          break; \
        } \
        case e16: { \
          if (P.MU.mfp16 == MTYPE_FP16) { \
            float16_t &td = P.MU.acc_elt<float16_t>(td_num + m, 0, i, j, mmax, nmax * amul, reg_rename, true); \
            float16_t ts1  = P.MU.acc_elt<float16_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false); \
            float16_t ts2  = P.MU.acc_elt<float16_t>(ts2_num + m, 0, i, j, mmax, nmax * amul, false, false); \
            BODY16; \
          } \
          else if (P.MU.mfp16 == MTYPE_BF16) { \
            bfloat16_t &td = P.MU.acc_elt<bfloat16_t>(td_num + m, 0, i, j, mmax, nmax * amul, reg_rename, true); \
            bfloat16_t ts1  = P.MU.acc_elt<bfloat16_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false); \
            bfloat16_t ts2  = P.MU.acc_elt<bfloat16_t>(ts2_num + m, 0, i, j, mmax, nmax * amul, false, false); \
            BODYB16; \
          } else {\
            require(0); \
          }\
          set_fp_exceptions; \
          break; \
        }\
        case e32: {\
          if (P.MU.mfp32 == MTYPE_FP32) { \
            float32_t &td = P.MU.acc_elt<float32_t>(td_num + m, 0, i, j, mmax, nmax * amul, reg_rename, true); \
            float32_t ts1  = P.MU.acc_elt<float32_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false); \
            float32_t ts2  = P.MU.acc_elt<float32_t>(ts2_num + m, 0, i, j, mmax, nmax * amul, false, false); \
            BODY32; \
          } else if (P.MU.mfp32 == MTYPE_TFP32) { \
            tfloat32_t &td = P.MU.acc_elt<tfloat32_t>(td_num + m, 0, i, j, mmax, nmax * amul, reg_rename, true); \
            tfloat32_t ts1  = P.MU.acc_elt<tfloat32_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false); \
            tfloat32_t ts2  = P.MU.acc_elt<tfloat32_t>(ts2_num + m, 0, i, j, mmax, nmax * amul, false, false); \
            BODYT32; \
          } else { \
            require(0); \
          } \
          set_fp_exceptions; \
          break; \
        }\
        case e64: {\
          if (P.MU.mfp64) { \
            float64_t &td = P.MU.acc_elt<float64_t>(td_num + m, 0, i, j, mmax, nmax * amul, reg_rename, true); \
            float64_t ts1  = P.MU.acc_elt<float64_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false); \
            float64_t ts2  = P.MU.acc_elt<float64_t>(ts2_num + m, 0, i, j, mmax, nmax * amul, false, false); \
            BODY64; \
            set_fp_exceptions; \
          } else {\
            require(0); \
          } \
          break; \
        }\
        default: \
          require(0); \
          break; \
        }; \
      } \
    } \
  } \
  REGNAME_WRITE_BAKE(mmax, nmax * amul, 1, true) \

#define MXU_VFP_W_MM_ADD(BODY16, BODYB16, BODY32, BODYT32, BODY64, sew) \
  require_fp; \
  require((sew == e16 && p->extension_enabled(EXT_ZFH)) || \
          (sew == e32 && p->extension_enabled('F')) || \
          (sew == e64 && p->extension_enabled('D'))); \
  require_matrix(true);\
  require(STATE.frm->read() < 0x5);\
  reg_t tile_m = P.MU.tile_m->read();\
  reg_t tile_n = P.MU.tile_n->read();\
  reg_t mmax = P.MU.mrows;\
  reg_t nmax = P.MU.mcols / sew;\
  reg_t td_num = insn.td(); \
  reg_t ts1_num = insn.ts1(); \
  reg_t ts2_num = insn.ts2(); \
  reg_t amul = P.MU.mamul; \
  /* reg_t lmul = insn.mlmul() != LMUL_RESERVE ? (1 << insn.mlmul()) : P.MU.mlmul; */ \
  reg_t lmul = 1; \
  require_align(insn.td(), lmul); \
  require_align(insn.ts1(), lmul); \
  require_align(insn.rs2(), lmul); \
  softfloat_roundingMode = STATE.frm->read(); \
  bool reg_rename = false; \
  if (td_num == ts1_num || td_num == ts2_num){ \
    reg_rename = true; \
  } \
  reg_t des_nmax = nmax / 2; \
  for (reg_t i=0; i<tile_m; ++i) { \
    for (reg_t m = 0; m < lmul; m++) { \
      for (reg_t j=0; j<tile_n; ++j) { \
        switch(sew) { \
        case e8: { \
          switch (P.MU.mfp8) \
          { \
          case MTYPE_FP8E4M3: { \
            if (P.MU.mfp16 == MTYPE_FP16) { \
              float16_t &td = P.MU.acc_elt<float16_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true); \
              float16_t ts1  = f8e4m3_to_f16(P.MU.acc_elt<float8_e4m3_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false)); \
              float16_t ts2  = f8e4m3_to_f16(P.MU.acc_elt<float8_e4m3_t>(ts2_num + m, 0, i, j, mmax, nmax * amul, false, false)); \
              BODY16; \
            } else if (P.MU.mfp16 == MTYPE_BF16) {\
              bfloat16_t &td = P.MU.acc_elt<bfloat16_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true); \
              bfloat16_t ts1  = f8e4m3_to_bf16(P.MU.acc_elt<float8_e4m3_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false)); \
              bfloat16_t ts2  = f8e4m3_to_bf16(P.MU.acc_elt<float8_e4m3_t>(ts2_num + m, 0, i, j, mmax, nmax * amul, false, false)); \
              BODYB16; \
            } else {\
              require(0); \
            } \
            break; \
          } \
          case MTYPE_FP8E5M2: { \
            if (P.MU.mfp16 == MTYPE_FP16) { \
              float16_t &td = P.MU.acc_elt<float16_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true); \
              float16_t ts1  = f8e5m2_to_f16(P.MU.acc_elt<float8_e5m2_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false)); \
              float16_t ts2  = f8e5m2_to_f16(P.MU.acc_elt<float8_e5m2_t>(ts2_num + m, 0, i, j, mmax, nmax * amul, false, false)); \
              BODY16; \
            } else if (P.MU.mfp16 == MTYPE_BF16) {\
              bfloat16_t &td = P.MU.acc_elt<bfloat16_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true); \
              bfloat16_t ts1  = f8e5m2_to_f16(P.MU.acc_elt<float8_e5m2_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false)); \
              bfloat16_t ts2  = f8e5m2_to_f16(P.MU.acc_elt<float8_e5m2_t>(ts2_num + m, 0, i, j, mmax, nmax * amul, false, false)); \
              BODYB16; \
            } else {\
              require(0); \
            } \
            break; \
          } \
          case MTYPE_FP8E3M4: { \
            if (P.MU.mfp16 == MTYPE_FP16) { \
              float16_t &td = P.MU.acc_elt<float16_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true); \
              float16_t ts1  = f8e3m4_to_f16(P.MU.acc_elt<float8_e3m4_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false)); \
              float16_t ts2  = f8e3m4_to_f16(P.MU.acc_elt<float8_e3m4_t>(ts2_num + m, 0, i, j, mmax, nmax * amul, false, false)); \
              BODY16; \
            } else if (P.MU.mfp16 == MTYPE_BF16) {\
              bfloat16_t &td = P.MU.acc_elt<bfloat16_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true); \
              bfloat16_t ts1  = f8e3m4_to_bf16(P.MU.acc_elt<float8_e3m4_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false)); \
              bfloat16_t ts2  = f8e3m4_to_bf16(P.MU.acc_elt<float8_e3m4_t>(ts2_num + m, 0, i, j, mmax, nmax * amul, false, false)); \
              BODYB16; \
            } else {\
              require(0); \
            } \
            break; \
          } \
          default: \
            require(0); \
            break; \
          }\
          set_fp_exceptions; \
          break; \
        }\
        case e16: {\
          if (P.MU.mfp16 == MTYPE_FP16 && P.MU.mfp32 == MTYPE_FP32) { \
            float32_t &td = P.MU.acc_elt<float32_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true); \
            float32_t ts1  = f16_to_f32(P.MU.acc_elt<float16_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false)); \
            float32_t ts2  = f16_to_f32(P.MU.acc_elt<float16_t>(ts2_num + m, 0, i, j, mmax, nmax * amul, false, false)); \
            BODY32; \
          } else if (P.MU.mfp16 == MTYPE_FP16 && P.MU.mfp32 == MTYPE_TFP32) {\
            tfloat32_t &td = P.MU.acc_elt<tfloat32_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true); \
            tfloat32_t ts1  = f16_to_tf32(P.MU.acc_elt<float16_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false)); \
            tfloat32_t ts2  = f16_to_tf32(P.MU.acc_elt<float16_t>(ts2_num + m, 0, i, j, mmax, nmax * amul, false, false)); \
            BODYT32; \
          } else if (P.MU.mfp16 == MTYPE_BF16 && P.MU.mfp32 == MTYPE_TFP32) {\
            tfloat32_t &td = P.MU.acc_elt<tfloat32_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true); \
            tfloat32_t ts1  = bf16_to_tf32(P.MU.acc_elt<bfloat16_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false)); \
            tfloat32_t ts2  = bf16_to_tf32(P.MU.acc_elt<bfloat16_t>(ts2_num + m, 0, i, j, mmax, nmax * amul, false, false)); \
            BODYT32; \
          } else if (P.MU.mfp16 == MTYPE_BF16 && P.MU.mfp32 == MTYPE_FP32) {\
            float32_t &td = P.MU.acc_elt<float32_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true); \
            float32_t ts1  = bf16_to_f32(P.MU.acc_elt<bfloat16_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false)); \
            float32_t ts2  = bf16_to_f32(P.MU.acc_elt<bfloat16_t>(ts2_num + m, 0, i, j, mmax, nmax * amul, false, false)); \
            BODY32; \
          } else {\
            require(0); \
          } \
          set_fp_exceptions; \
          break; \
        }\
        case e32: {\
          if (P.MU.mfp32 == MTYPE_FP32) {\
            float64_t &td = P.MU.acc_elt<float64_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true); \
            float64_t ts1  = f32_to_f64(P.MU.acc_elt<float32_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false)); \
            float64_t ts2  = f32_to_f64(P.MU.acc_elt<float32_t>(ts2_num + m, 0, i, j, mmax, nmax * amul, false, false)); \
            BODY64; \
          } else if (P.MU.mfp32 == MTYPE_TFP32) {\
            float64_t &td = P.MU.acc_elt<float64_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true); \
            float64_t ts1  = tf32_to_f64(P.MU.acc_elt<tfloat32_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false)); \
            float64_t ts2  = tf32_to_f64(P.MU.acc_elt<tfloat32_t>(ts2_num + m, 0, i, j, mmax, nmax * amul, false, false)); \
            BODY64; \
          } else {\
            require(0); \
          } \
          set_fp_exceptions; \
          break; \
        }\
        default: \
          require(0); \
          break; \
        }; \
      } \
    } \
  } \
  REGNAME_WRITE_BAKE(mmax, nmax * amul, 1, true) \

#define MXU_VFP_VV_LOOP_WIDE(wide, sew) \
  MXU_VFP_LOOP_BASE(wide, sew) \
  switch(sew) { \
    case e8: { \
      switch (P.MU.mfp8) { /*e4m3*/ \
        case MTYPE_FP8E4M3: { \
          if (P.MU.mfp16 == MTYPE_FP16) {  /* 0x1 fp16 */ \
            float16_t &td_w = P.MU.acc_elt<float16_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, false, true); \
            float16_t ts1 = f8e4m3_to_f16(P.MU.tr_elt<float8_e4m3_t>(ts1_num + m, 0, i, k, mmax, nmax, false, false)); \
            float16_t ts2 = f8e4m3_to_f16(P.MU.tr_elt<float8_e4m3_t>(ts2_num + m, 0, k, j, mmax, nmax, false, false)); \
            td_w = f16_mulAdd(ts1, ts2, td_w); \
          } else if (P.MU.mfp16 == MTYPE_BF16) { /* 0x2 bf16*/ \
            bfloat16_t &td_w = P.MU.acc_elt<bfloat16_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, false, true); \
            bfloat16_t ts1 = f8e4m3_to_bf16(P.MU.tr_elt<float8_e4m3_t>(ts1_num + m, 0, i, k, mmax, nmax, false, false)); \
            bfloat16_t ts2 = f8e4m3_to_bf16(P.MU.tr_elt<float8_e4m3_t>(ts2_num + m, 0, k, j, mmax, nmax, false, false)); \
            td_w = bf16_mulAdd(ts1, ts2, td_w); \
          } else { \
            require(0); /*mfp16*/ \
          } \
          break; \
        } \
        case MTYPE_FP8E5M2: { /*m5e2*/ \
          if (P.MU.mfp16 == MTYPE_FP16) {  /* 0x1 fp16 */ \
              float16_t &td_w = P.MU.acc_elt<float16_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, false, true); \
              float16_t ts1 = f8e5m2_to_f16(P.MU.tr_elt<float8_e5m2_t>(ts1_num + m, 0, i, k, mmax, nmax, false, false)); \
              float16_t ts2 = f8e5m2_to_f16(P.MU.tr_elt<float8_e5m2_t>(ts2_num + m, 0, k, j, mmax, nmax, false, false)); \
              td_w = f16_mulAdd(ts1, ts2, td_w); \
            } else if (P.MU.mfp16 == MTYPE_BF16) { /* 0x2 bf16*/ \
              bfloat16_t &td_w = P.MU.acc_elt<bfloat16_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, false, true); \
              bfloat16_t ts1 = f8e5m2_to_bf16(P.MU.tr_elt<float8_e5m2_t>(ts1_num + m, 0, i, k, mmax, nmax, false, false)); \
              bfloat16_t ts2 = f8e5m2_to_bf16(P.MU.tr_elt<float8_e5m2_t>(ts2_num + m, 0, k, j, mmax, nmax, false, false)); \
              td_w = bf16_mulAdd(ts1, ts2, td_w); \
            } else { \
              require(0); /*mfp16*/ \
            } \
            break; \
        } \
        case MTYPE_FP8E3M4: { /* e3m4*/ \
          if (P.MU.mfp16 == MTYPE_FP16) {  /* 0x1 fp16 */ \
              float16_t &td_w = P.MU.acc_elt<float16_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, false, true); \
              float16_t ts1 = f8e3m4_to_f16(P.MU.tr_elt<float8_e3m4_t>(ts1_num + m, 0, i, k, mmax, nmax, false, false)); \
              float16_t ts2 = f8e3m4_to_f16(P.MU.tr_elt<float8_e3m4_t>(ts2_num + m, 0, k, j, mmax, nmax, false, false)); \
              td_w = f16_mulAdd(ts1, ts2, td_w); \
            } else if (P.MU.mfp16 == MTYPE_BF16) { /* 0x2 bf16*/ \
              bfloat16_t &td_w = P.MU.acc_elt<bfloat16_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, false, true); \
              bfloat16_t ts1 = f8e3m4_to_bf16(P.MU.tr_elt<float8_e3m4_t>(ts1_num + m, 0, i, k, mmax, nmax, false, false)); \
              bfloat16_t ts2 = f8e3m4_to_bf16(P.MU.tr_elt<float8_e3m4_t>(ts2_num + m, 0, k, j, mmax, nmax, false, false)); \
              td_w = bf16_mulAdd(ts1, ts2, td_w); \
            } else { \
              require(0); /*mfp16*/ \
            } \
            break; \
        } \
      } \
      set_fp_exceptions; \
    } \
    case e16: {\
      switch(P.MU.mfp16){ \
        case MTYPE_FP16: { /*fp16*/ \
          if (P.MU.mfp32 == MTYPE_FP32) { /*fp32*/ \
            float32_t &td_w = P.MU.acc_elt<float32_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, false, true); \
            float32_t ts1 = f16_to_f32(P.MU.tr_elt<float16_t>(ts1_num + m, 0, i, k, mmax, nmax, false, false)); \
            float32_t ts2 = f16_to_f32(P.MU.tr_elt<float16_t>(ts2_num + m, 0, k, j, mmax, nmax, false, false)); \
            td_w = f32_mulAdd(ts1, ts2, td_w); \
          } else if (P.MU.mfp32 == MTYPE_TFP32 ) { /*tf32*/ \
            tfloat32_t &td_w = P.MU.acc_elt<tfloat32_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, false, true); \
            tfloat32_t ts1 = f16_to_tf32(P.MU.tr_elt<float16_t>(ts1_num + m, 0, i, k, mmax, nmax, false, false)); \
            tfloat32_t ts2 = f16_to_tf32(P.MU.tr_elt<float16_t>(ts2_num + m, 0, k, j, mmax, nmax, false, false)); \
            td_w = tf32_mulAdd(ts1, ts2, td_w); \
          } else { /* type not support */ \
            require(0); \
          } \
          break; \
        } \
        case MTYPE_BF16: { /*bf16*/ \
          if (P.MU.mfp32 == MTYPE_FP32) { /*fp32*/ \
            float32_t &td_w = P.MU.acc_elt<float32_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, false, true); \
            float32_t ts1 = bf16_to_f32(P.MU.tr_elt<bfloat16_t>(ts1_num + m, 0, i, k, mmax, nmax, false, false)); \
            float32_t ts2 = bf16_to_f32(P.MU.tr_elt<bfloat16_t>(ts2_num + m, 0, k, j, mmax, nmax, false, false)); \
            td_w = f32_mulAdd(ts1, ts2, td_w); \
          } else if (P.MU.mfp32 == MTYPE_TFP32 ) { /*tf32*/ \
            tfloat32_t &td_w = P.MU.acc_elt<tfloat32_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, false, true); \
            tfloat32_t ts1 = bf16_to_tf32(P.MU.tr_elt<bfloat16_t>(ts1_num + m, 0, i, k, mmax, nmax, false, false)); \
            tfloat32_t ts2 = bf16_to_tf32(P.MU.tr_elt<bfloat16_t>(ts2_num + m, 0, k, j, mmax, nmax, false, false)); \
            td_w = tf32_mulAdd(ts1, ts2, td_w); \
          } else { /* type not support */ \
            require(0); \
          } \
          break; \
        } \
      } \
      set_fp_exceptions; \
      break; \
    }\
    case e32: {\
      if (P.MU.mfp32 == MTYPE_FP32) { /*fp32*/ \
        float64_t &td_w = P.MU.acc_elt<float64_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, false, true); \
        float64_t ts1 = f32_to_f64(P.MU.tr_elt<float32_t>(ts1_num + m, 0, i, k, mmax, nmax, false, false)); \
        float64_t ts2 = f32_to_f64(P.MU.tr_elt<float32_t>(ts2_num + m, 0, k, j, mmax, nmax, false, false)); \
        td_w = f64_mulAdd(ts1, ts2, td_w); \
      } else if (P.MU.mfp32 == MTYPE_TFP32) { /*tf32*/ \
        float64_t &td_w = P.MU.acc_elt<float64_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, false, true); \
        float64_t ts1 = tf32_to_f64(P.MU.tr_elt<tfloat32_t>(ts1_num + m, 0, i, k, mmax, nmax, false, false)); \
        float64_t ts2 = tf32_to_f64(P.MU.tr_elt<tfloat32_t>(ts2_num + m, 0, k, j, mmax, nmax, false, false)); \
        td_w = f64_mulAdd(ts1, ts2, td_w); \
      } else { /* type not support*/ \
        require(0); \
      } \
      set_fp_exceptions; \
      break; \
    }\
    default: \
      require(0); \
      break; \
  }; \
  DEBUG_RVV_FP_VV; \
  MXU_VFP_LOOP_END \

#define MXU_VFP_VV_LOOP_QUAD(wide, sew) \
  MXU_VFP_LOOP_BASE(wide, sew) \
  switch(sew) { \
    case e8: {\
      switch (P.MU.mfp8) { \
        case MTYPE_FP8E4M3: { /* fp8e4m3*/ \
          if (P.MU.mfp32 == MTYPE_FP32) { \
            float32_t &td_w = P.MU.acc_elt<float32_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true); \
            float32_t ts1 = f8e4m3_to_f32(P.MU.tr_elt<float8_e4m3_t>(ts1_num + m, 0, i, k, mmax, nmax, false, false)); \
            float32_t ts2 = f8e4m3_to_f32(P.MU.tr_elt<float8_e4m3_t>(ts2_num + m, 0, k, j, mmax, nmax, false, false)); \
            td_w = f32_mulAdd(ts1, ts2, td_w); \
          } else if (P.MU.mfp32 == MTYPE_TFP32) { \
            tfloat32_t &td_w = P.MU.acc_elt<tfloat32_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true); \
            tfloat32_t ts1 = f8e4m3_to_tf32(P.MU.tr_elt<float8_e4m3_t>(ts1_num + m, 0, i, k, mmax, nmax, false, false)); \
            tfloat32_t ts2 = f8e4m3_to_tf32(P.MU.tr_elt<float8_e4m3_t>(ts2_num + m, 0, k, j, mmax, nmax, false, false)); \
            td_w = tf32_mulAdd(ts1, ts2, td_w); \
          } \
          break; \
        } \
        case MTYPE_FP8E5M2: { /* fp8e5m2 */ \
          if (P.MU.mfp32 == MTYPE_FP32) { \
            float32_t &td_w = P.MU.acc_elt<float32_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true); \
            float32_t ts1 = f8e5m2_to_f32(P.MU.tr_elt<float8_e5m2_t>(ts1_num + m, 0, i, k, mmax, nmax, false, false)); \
            float32_t ts2 = f8e5m2_to_f32(P.MU.tr_elt<float8_e5m2_t>(ts2_num + m, 0, k, j, mmax, nmax, false, false)); \
            td_w = f32_mulAdd(ts1, ts2, td_w); \
          } else if (P.MU.mfp32 == MTYPE_TFP32) { \
            tfloat32_t &td_w = P.MU.acc_elt<tfloat32_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true); \
            tfloat32_t ts1 = f8e5m2_to_tf32(P.MU.tr_elt<float8_e5m2_t>(ts1_num + m, 0, i, k, mmax, nmax, false, false)); \
            tfloat32_t ts2 = f8e5m2_to_tf32(P.MU.tr_elt<float8_e5m2_t>(ts2_num + m, 0, k, j, mmax, nmax, false, false)); \
            td_w = tf32_mulAdd(ts1, ts2, td_w); \
          } \
          break; \
        } \
        case MTYPE_FP8E3M4: { /* fp8e3m4 */ \
          if (P.MU.mfp32 == MTYPE_FP32) { \
            float32_t &td_w = P.MU.acc_elt<float32_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true); \
            float32_t ts1 = f8e3m4_to_f32(P.MU.tr_elt<float8_e3m4_t>(ts1_num + m, 0, i, k, mmax, nmax, false, false)); \
            float32_t ts2 = f8e3m4_to_f32(P.MU.tr_elt<float8_e3m4_t>(ts2_num + m, 0, k, j, mmax, nmax, false, false)); \
            td_w = f32_mulAdd(ts1, ts2, td_w); \
          } else if (P.MU.mfp32 == MTYPE_TFP32) { \
            tfloat32_t &td_w = P.MU.acc_elt<tfloat32_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true); \
            tfloat32_t ts1 = f8e3m4_to_tf32(P.MU.tr_elt<float8_e3m4_t>(ts1_num + m, 0, i, k, mmax, nmax, false, false)); \
            tfloat32_t ts2 = f8e3m4_to_tf32(P.MU.tr_elt<float8_e3m4_t>(ts2_num + m, 0, k, j, mmax, nmax, false, false)); \
            td_w = tf32_mulAdd(ts1, ts2, td_w); \
          } else { \
            require(0); \
          } \
          break; \
        } \
      } \
      break; \
    }\
    case e16: {\
      if (P.MU.mfp16 == MTYPE_FP16) { \
        float64_t &td_w = P.MU.tr_elt<float64_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true); \
        float64_t ts1 = f16_to_f64(P.MU.tr_elt<float16_t>(ts1_num + m, 0, i, k, mmax, nmax, false, false)); \
        float64_t ts2 = f16_to_f64(P.MU.tr_elt<float16_t>(ts2_num + m, 0, k, j, mmax, nmax, false, false)); \
        td_w = f64_mulAdd(ts1, ts2, td_w); \
      } else if (P.MU.mfp16 == MTYPE_BF16) { \
        float64_t &td_w = P.MU.tr_elt<float64_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true); \
        float64_t ts1 = bf16_to_f64(P.MU.tr_elt<bfloat16_t>(ts1_num + m, 0, i, k, mmax, nmax, false, false)); \
        float64_t ts2 = bf16_to_f64(P.MU.tr_elt<bfloat16_t>(ts2_num + m, 0, k, j, mmax, nmax, false, false)); \
        td_w = f64_mulAdd(ts1, ts2, td_w); \
      } else {\
        require(0); \
      } \
      set_fp_exceptions; \
      break; \
    }\
    default: \
      require(0); \
      break; \
  }; \
  DEBUG_RVV_FP_VV; \
  MXU_VFP_LOOP_END \

#define VM_CHECK_STORE(elt_width, rows, cols)

#define VM_CHECK_LOAD(elt_width, rows, cols) \
  VM_CHECK_STORE(elt_width, rows, cols); \

// #define MTI_CHECK_LOAD(let_width, is_mask_ldst)

#define MTU_LS_LEN(trans, ch, type_size) \
  require_matrix(false) \
  switch (ch) \
  { \
  case 'c' : \
    height = trans? P.MU.tile_n->read() : P.MU.tile_m->read(); \
    width = trans? P.MU.tile_m->read() : P.MU.tile_n->read(); \
    rmax = P.MU.mrows; \
    cmax = (P.MU.mrlenb / type_size); \
    break; \
  case 'a' : \
    height = trans? P.MU.tile_k->read() : P.MU.tile_m->read(); \
    width = trans? P.MU.tile_m->read() : P.MU.tile_k->read(); \
    rmax = P.MU.mrows; \
    cmax = (P.MU.mrlenb / type_size); \
    break; \
  case 'b' : \
    height = trans? P.MU.tile_n->read() : P.MU.tile_k->read(); \
    width = trans? P.MU.tile_k->read() : P.MU.tile_n->read(); \
    cmax = (P.MU.mrlenb / type_size); \
    rmax = P.MU.mrows; \
    break; \
  default : \
    break; \
  }; \

#define CLEAR_TILE(td) \
for (reg_t m = 0; m < lmul; m++) {\
  for (reg_t i = 0; i < height; i++) { \
    for (reg_t j = 0; j < P.MU.mcols / 8; j++) { \
      P.MU.tr_elt<int8_t>(td + m, 0, i, j, P.MU.mrows, P.MU.mcols >> 3, false, true) = 0; \
    } \
  } \
}\

#define CLEAR_ACC(accd) \
  for (reg_t i = 0; i < height; i++) { \
    for (reg_t j = 0; j < P.MU.mcols * P.MU.mamul / 8; j++) { \
      P.MU.acc_elt<int8_t>(accd, 0, i, j, P.MU.mrows, (P.MU.mcols >> 3) * P.MU.mamul, false, true) = 0; \
    } \
  } \

#define PAD_TILE(td, elt_width, val) \
for (reg_t m = 0; m < lmul; m++) {\
  for (reg_t i = 0; i < height; i++) { \
    for (reg_t j = 0; j < width; j++) { \
      P.MU.tr_elt<elt_width##_t>(td + m, 0, i, j, rmax, cmax, false, true) = (elt_width##_t)val; \
    } \
  } \
}\

#define PAD_ACC(td, elt_width, val) \
for (reg_t m = 0; m < lmul; m++) {\
  for (reg_t i = 0; i < height; i++) { \
    for (reg_t j = 0; j < width; j++) { \
      P.MU.acc_elt<elt_width##_t>(td + m, 0, i, j, rmax, cmax * P.MU.mamul, false, true) = (elt_width##_t)val; \
    } \
  } \
}\

#define WHOLE_MATRIX(is_trans, is_true) \
  if (is_true) { \
    if (is_trans) { \
      height = cmax ; \
      width = rmax ; \
    } else { \
      height = rmax ; \
      width = cmax ; \
    } \
  } \

#define MTU_TR_LD(is_trans, dim, elt_width, is_max) \
  const reg_t baseAddr = RS1; \
  const reg_t stride2 = RS2; \
  const reg_t td = insn.td(); \
  /* reg_t lmul = insn.mlmul() != LMUL_RESERVE ? (1 << insn.mlmul()) : P.MU.mlmul; */ \
  reg_t lmul = 1; \
  require_align(td, lmul); \
  reg_t height, width; \
  reg_t rmax = 0, cmax = 0;\
  MTU_LS_LEN(is_trans, dim, sizeof(elt_width##_t)); \
  WHOLE_MATRIX(is_trans, is_max) \
  CLEAR_TILE(td); \
  for (reg_t i = 0; i < height; ++i) { \
    for (reg_t m = 0; m < lmul; m++) {\
      for (reg_t j = 0; j < width; ++j) { \
          elt_width##_t val = MMU.load<elt_width##_t>( \
                    baseAddr + i * stride2 + j * sizeof(elt_width##_t) + \
                      m * (width * sizeof(elt_width##_t))); \
          P.MU.tr_elt<elt_width##_t>(td + m, is_trans, i, j, rmax, cmax, false, true) = val; \
      } \
    } \
  } \

  #define MTU_ACC_LD(is_trans, dim, elt_width, is_max) \
  const reg_t baseAddr = RS1; \
  const reg_t stride2 = RS2; \
  const reg_t accd = insn.td(); \
  reg_t height, width; \
  reg_t amul = P.MU.mamul; \
  reg_t rmax = 0, cmax = 0;\
  MTU_LS_LEN(is_trans, dim, sizeof(elt_width##_t)); \
  WHOLE_MATRIX(is_trans, is_max) \
  CLEAR_ACC(accd); \
  for (reg_t i = 0; i < height; ++i) { \
    for (reg_t j = 0; j < width; ++j) { \
        elt_width##_t val = MMU.load<elt_width##_t>( \
                  baseAddr + i * stride2 + j * sizeof(elt_width##_t)); \
        P.MU.acc_elt<elt_width##_t>(accd, is_trans, i, j, rmax, cmax * amul, false, true) = val; \
    } \
  } \

#define MTU_TR_ST(is_trans, dim, elt_width, is_max) \
  const reg_t baseAddr = RS1; \
  const reg_t stride2 = RS2; \
  const reg_t td = insn.td(); \
  /*reg_t lmul = insn.mlmul() != LMUL_RESERVE ? (1 << insn.mlmul()) : P.MU.mlmul; \*/ \
  reg_t lmul = 1; \
  require_align(td, lmul); \
  reg_t height = 0, width = 0; \
  reg_t rmax = 0, cmax = 0; \
  MTU_LS_LEN(is_trans, dim, sizeof(elt_width##_t)); \
  WHOLE_MATRIX(is_trans, is_max) \
  for (reg_t i = 0; i < height; ++i) { \
    for (reg_t m = 0; m < lmul; m++) {\
      for (reg_t j = 0; j < width; ++j) { \
          elt_width##_t val = P.MU.tr_elt<elt_width##_t>(td + m, is_trans, i, j, rmax, cmax, false, true); \
          MMU.store<elt_width##_t>( \
                    baseAddr + i * stride2 + j * sizeof(elt_width##_t) + \
                    m * (width * sizeof(elt_width##_t)), val); \
      } \
    } \
  } \


#define MTU_ACC_ST(is_trans, dim, elt_width, is_max) \
  const reg_t baseAddr = RS1; \
  const reg_t stride2 = RS2; \
  const reg_t accd = insn.td(); \
  reg_t height = 0, width = 0; \
  reg_t amul = P.MU.mamul; \
  reg_t rmax = 0, cmax = 0;\
  MTU_LS_LEN(is_trans, dim, sizeof(elt_width##_t)); \
  WHOLE_MATRIX(is_trans, is_max) \
  for (reg_t i = 0; i < height; ++i) { \
    for (reg_t j = 0; j < width; ++j) { \
        elt_width##_t val = P.MU.acc_elt<elt_width##_t>(accd, is_trans, i, j, rmax, cmax * amul, false, false); \
        MMU.store<elt_width##_t>( \
                  baseAddr + i * stride2 + j * sizeof(elt_width##_t), val); \
    } \
  } \

#define MTU_VECTOR_LD(is_trans, dim, elt_width) \
  const reg_t baseAddr = RS1; \
  const reg_t stride2 = RS2; \
  const reg_t vd = insn.rd(); \
  reg_t height, width; \
  reg_t cmax = 0, rmax = 0; \
  MTU_LS_LEN(is_trans, dim, sizeof(elt_width##_t)); \
  for (reg_t i = 0; i < height; i++) { \
    for (reg_t j = 0; j < width; j++) { \
      elt_width##_t val = MMU.load<elt_width##_t>( \
              baseAddr + i * stride2 + j * sizeof(elt_width##_t)); \
      P.VU.elt<elt_width##_t>(vd, i * width + j, true) = val; \
    } \
  } \

#define MTU_VECTOR_ST(is_trans, dim, elt_width) \
  const reg_t baseAddr = RS1; \
  const reg_t stride2 = RS2; \
  const reg_t vd = insn.rd(); \
  reg_t height, width; \
  reg_t cmax = 0, rmax = 0; \
  MTU_LS_LEN(is_trans, dim, sizeof(elt_width##_t)); \
  for (reg_t i = 0; i < height; i++) { \
    for (reg_t j = 0; j < width; j++) { \
      elt_width##_t val = P.VU.elt<elt_width##_t>(vd, i * width + j); \
      MMU.store<elt_width##_t>( \
        baseAddr + i * stride2 + j * sizeof(elt_width##_t), val); \
    } \
  } \


#define MTU_UF_TR_ACC_LD(is_trans, elt_width, dim) \
  reg_t baseAddr = RS1; \
  const reg_t stride2 = RS2; \
  const reg_t td = insn.td(); \
  reg_t sh = P.MU.mstr_h; \
  reg_t sw = P.MU.mstr_w; \
  reg_t dh = P.MU.mdil_h; \
  reg_t dw = P.MU.mdil_w; \
  reg_t inh = P.MU.inshape[1]; \
  reg_t inw = P.MU.inshape[0]; \
  reg_t pt = P.MU.mpad_top; \
  reg_t pb = P.MU.mpad_bottom; \
  reg_t pl = P.MU.mpad_left; \
  reg_t pr = P.MU.mpad_right; \
  reg_t outh = P.MU.outshape[1]; \
  reg_t outw = P.MU.outshape[0]; \
  sreg_t inposh = P.MU.mskin[1]; \
  sreg_t inposw = P.MU.mskin[0]; \
  reg_t krposw = P.MU.mskout[1]; \
  reg_t outposw = P.MU.mskout[0]; \
  reg_t mpadv = P.MU.mpadval->read(); \
  reg_t height, width; \
  reg_t rmax = 0, cmax = 0; \
  reg_t lmul = 1; \
  reg_t amul = P.MU.mamul; \
  MTU_LS_LEN(is_trans, dim, sizeof(elt_width##_t)); \
  if (dim == 'c') {\
    CLEAR_ACC(td); \
    PAD_ACC(td, elt_width, mpadv); \
  } else {\
    CLEAR_TILE(td); \
    PAD_TILE(td, elt_width, mpadv); \
  } \
  for (reg_t i = 0; i < height; ++i) { \
    if (inposh >= 0 && (reg_t)inposh < inh && inposw >= 0 && (reg_t)inposw < inw) { \
      for (reg_t j = 0; j < width; ++j) { \
        elt_width##_t val = MMU.load<elt_width##_t>( \
                      baseAddr + j * sizeof(elt_width##_t)); \
        if (dim == 'c') \
          P.MU.acc_elt<elt_width##_t>(td, is_trans, i, j, rmax, cmax * amul, false, true) = val; \
        else \
          P.MU.tr_elt<elt_width##_t>(td, is_trans, i, j, rmax, cmax, false, true) = val; \
      } \
    } \
    outposw++; \
    if (outposw > outw - 1) { \
      outposw = 0; \
      baseAddr += (inw - inposw - pl) * stride2 + (sh -1) * inw * stride2 + krposw * stride2; \
      inposw = - pl + krposw; \
      inposh+=sh; \
    } else { \
      inposw += sw; \
      baseAddr += sw * stride2; \
    } \
  } \


#define MTU_UF_TR_ACC_SD(is_trans, elt_width, dim) \
  reg_t baseAddr = RS1; \
  const reg_t stride2 = RS2; \
  const reg_t td = insn.td(); \
  reg_t sh = P.MU.mstr_h; \
  reg_t sw = P.MU.mstr_w; \
  reg_t dh = P.MU.mdil_h; \
  reg_t dw = P.MU.mdil_w; \
  reg_t inh = P.MU.inshape[1]; \
  reg_t inw = P.MU.inshape[0]; \
  reg_t pt = P.MU.mpad_top; \
  reg_t pb = P.MU.mpad_bottom; \
  reg_t pl = P.MU.mpad_left; \
  reg_t pr = P.MU.mpad_right; \
  reg_t outh = P.MU.outshape[1]; \
  reg_t outw = P.MU.outshape[0]; \
  sreg_t inposh = P.MU.mskin[1]; \
  sreg_t inposw = P.MU.mskin[0]; \
  reg_t krposw = P.MU.mskout[1]; \
  reg_t outposw = P.MU.mskout[0]; \
  reg_t height, width; \
  reg_t rmax = 0, cmax = 0; \
  reg_t lmul = 1; \
  reg_t amul = P.MU.mamul; \
  MTU_LS_LEN(is_trans, dim, sizeof(elt_width##_t)); \
  for (reg_t i = 0; i < height; ++i) { \
    if (inposh >= 0 && (reg_t)inposh < inh && inposw >= 0 && (reg_t)inposw < inw) { \
      for (reg_t j = 0; j < width; ++j) { \
        if (dim == 'c') { \
          auto val = P.MU.acc_elt<elt_width##_t>(td, is_trans, i, j, rmax, cmax * amul, false, true); \
          MMU.store<elt_width##_t>( \
                      baseAddr + j * sizeof(elt_width##_t), val); \
        } else { \
          auto val = P.MU.tr_elt<elt_width##_t>(td, is_trans, i, j, rmax, cmax, false, true); \
          MMU.store<elt_width##_t>( \
                      baseAddr + j * sizeof(elt_width##_t), val); \
        } \
      } \
    } \
    outposw++; \
    if (outposw > outw - 1) { \
      outposw = 0; \
      baseAddr += (inw - inposw - pl) * stride2 + (sh -1) * inw * stride2 + krposw * stride2; \
      inposw = - pl + krposw; \
      inposh+=sh; \
    } else { \
      inposw += sw; \
      baseAddr += sw * stride2; \
    } \
  } \

#define MU_MFP_LOOP_SCALE_BASE(wide, sew) \
  const reg_t ts1_num = insn.ts1(); \
  const reg_t td_num = insn.td(); \
  reg_t mmax = P.MU.mrows; \
  reg_t nmax = P.MU.mcols / sew; \
  reg_t tile_m = P.MU.tile_m->read(); \
  reg_t tile_n = P.MU.tile_n->read(); \
  /* reg_t lmul = insn.mlmul() != LMUL_RESERVE ? (1 << insn.mlmul()) : P.MU.mlmul; */ \
  reg_t lmul = 1; \
  require_align(insn.td(), lmul); \
  require_align(insn.ts1(), lmul); \
  softfloat_roundingMode = STATE.frm->read(); \
  bool reg_rename = false; \
  if (td_num == ts1_num){ \
    reg_rename = true; \
  } \
  reg_t reg_sum = 1; \
  reg_t amul = P.MU.mamul; \
  reg_t des_nmax = nmax / wide; \
  for (reg_t i = 0; i < tile_m; ++i) { \
    for (reg_t m = 0; m < lmul; m++) { \
      for (reg_t j = 0; j < tile_n; ++j) { \


#define MU_MFP_LOOP_END \
    } \
  } \
  } \

#define MXU_MFP_CVT_SCALE(BODY8, BODY16, BODY32, BODY64,\
                         CHECK8, CHECK16, CHECK32, CHECK64,\
                         wide, eew_check, sew) \
  eew_check; \
  MU_MFP_LOOP_SCALE_BASE(wide, sew) \
  switch(sew) { \
    case e8: {\
      CHECK8 \
        BODY8 \
        set_fp_exceptions; \
      } \
      break; \
    case e16: {\
      CHECK16 \
        BODY16 \
        set_fp_exceptions; \
      } \
      break; \
    case e32: {\
      CHECK32 \
        BODY32 \
        set_fp_exceptions; \
      } \
      break; \
    case e64: {\
      CHECK64 \
        BODY64 \
        set_fp_exceptions; \
      } \
      break; \
    default: \
      require(0); \
      break; \
  } \
  MU_MFP_LOOP_END \
  REGNAME_WRITE_BAKE(mmax, nmax * amul * wide, reg_sum, true) \

#define MXU_MFP_CVT(BODY8, BODY16, BODY32, BODY64, wide, sew) \
  MU_MFP_LOOP_SCALE_BASE(wide, sew) \
  switch(sew) { \
    case e8: {\
        BODY8 \
        set_fp_exceptions; \
      } \
      break; \
    case e16: {\
        BODY16 \
        set_fp_exceptions; \
      } \
      break; \
    case e32: {\
        BODY32 \
        set_fp_exceptions; \
      } \
      break; \
    case e64: {\
        BODY64 \
        set_fp_exceptions; \
      } \
      break; \
    default: \
      require(0); \
      break; \
  } \
  MU_MFP_LOOP_END \
  REGNAME_WRITE_BAKE(mmax, nmax * amul * wide, reg_sum, true) \

#define MX_CVT_LOOP_BASE(wide, sew) \
  const reg_t ts1_num = insn.ts1(); \
  const reg_t td_num = insn.td(); \
  reg_t mmax = P.MU.mrows; \
  reg_t nmax = P.MU.mcols / sew; \
  reg_t tile_m = P.MU.tile_m->read(); \
  reg_t tile_n = P.MU.tile_n->read(); \
  /* reg_t lmul = insn.mlmul() != LMUL_RESERVE ? (1 << insn.mlmul()) : P.MU.mlmul; */ \
  reg_t lmul = 1; \
  require_align(insn.td(), lmul); \
  require_align(insn.ts1(), lmul); \
  softfloat_roundingMode = STATE.frm->read(); \
  bool reg_rename = false; \
  if (td_num == ts1_num){ \
    reg_rename = true; \
  } \
  reg_t reg_sum = 1; \
  reg_t des_nmax = nmax / wide; \
  reg_t amul = P.MU.mamul; \
  for (reg_t i = 0; i < tile_m; ++i) { \
    for (reg_t m = 0; m < lmul; m++) { \
      for (reg_t j = 0; j < tile_n; ++j) { \

#define MX_CVT_LOOP_BASE_END \
    } \
  } \
  } \

#define MX_CVT(BODY, CVT, wide, sew) \
  require(sew >= e4 && sew * wide <= e64); \
  MX_CVT_LOOP_BASE(wide, sew) \
  if (sew == e4) {\
    nmax /= 2; \
    MX_##CVT##_PARAMS(e8, wide); \
    BODY; \
  } else if (sew == e8) { \
    MX_##CVT##_PARAMS(e8, wide); \
    BODY; \
  } else if (sew == e16) {\
    MX_##CVT##_PARAMS(e16, wide); \
    BODY; \
  } else if (sew == e32) {\
    MX_##CVT##_PARAMS(e32, wide); \
    BODY; \
  } else if (sew == e64) {\
    MX_##CVT##_PARAMS(e64, wide); \
    BODY; \
  } else { \
    require(0); \
  } \
  MX_CVT_LOOP_BASE_END \
  REGNAME_WRITE_BAKE(mmax, nmax * amul * wide, reg_sum, true) \

#define MX_WCVT(BODY, CVT, wide, sew) \
  require(sew >= e4 && sew * wide <= e64); \
  MX_CVT_LOOP_BASE(wide, sew) \
  if (sew == e4) {\
    nmax /= 2; \
    MX_##CVT##_PARAMS(e8, wide); \
    BODY; \
  } else if (sew == e8) { \
    MX_##CVT##_PARAMS(e8, wide); \
    BODY; \
  } else if (sew == e16) {\
    MX_##CVT##_PARAMS(e16, wide); \
    BODY; \
  } else if (sew == e32) {\
    MX_##CVT##_PARAMS(e32, wide); \
    BODY; \
  } else { \
    require(0); \
  } \
  MX_CVT_LOOP_BASE_END \
  REGNAME_WRITE_BAKE(mmax, nmax * amul * wide, reg_sum, true) \

#define MX_WQCVT(BODY, CVT, wide, sew) \
  require(sew >= e4 && sew * wide <= e64); \
  MX_CVT_LOOP_BASE(wide, sew) \
  if (sew == e4) {\
    nmax /= 2; \
    MX_##CVT##_PARAMS(e8, wide); \
    BODY; \
  } else if (sew == e8) { \
    MX_##CVT##_PARAMS(e8, wide); \
    BODY; \
  } else if (sew == e16) {\
    MX_##CVT##_PARAMS(e16, wide); \
    BODY; \
  } else { \
    require(0); \
  } \
  MX_CVT_LOOP_BASE_END \
  REGNAME_WRITE_BAKE(mmax, nmax * amul * wide, reg_sum, true) \

  #define MX_WOCVT(BODY, CVT, wide, sew) \
  require(sew >= e4 && sew * wide <= e64); \
  MX_CVT_LOOP_BASE(wide, sew) \
  if (sew == e4) {\
    nmax /= 2; \
    MX_##CVT##_PARAMS(e8, wide); \
    BODY; \
  } else if (sew == e8) { \
    MX_##CVT##_PARAMS(e8, wide); \
    BODY; \
  } else { \
    require(0); \
  } \
  MX_CVT_LOOP_BASE_END \
  REGNAME_WRITE_BAKE(mmax, nmax * amul * wide, reg_sum, true) \

#define MX_NCVT(BODY, NCVT, narr, sew) \
  require(sew >= e8 && sew  <= e64); \
  MX_CVT_LOOP_BASE(narr, sew) \
  if (sew == e8) { \
    MX_##NCVT##_PARAMS(e8, 1); \
    BODY; \
  } else if (sew == e16) {\
    MX_##NCVT##_PARAMS(e16, narr); \
    BODY; \
  } else if (sew == e32) {\
    MX_##NCVT##_PARAMS(e32, narr); \
    BODY; \
  } else if (sew == e64) {\
    MX_##NCVT##_PARAMS(e64, narr); \
    BODY; \
  } else { \
    require(0); \
  } \
  MX_CVT_LOOP_BASE_END \
  REGNAME_WRITE_BAKE(mmax, nmax * amul * narr, reg_sum, true) \

#define MX_NCVT_QUAD(BODY, NCVT, narr, sew) \
  require(sew >= e16 && sew  <= e64); \
  MX_CVT_LOOP_BASE(narr, sew) \
  if (sew == e16) { \
    MX_##NCVT##_PARAMS(e16, 2); \
    BODY; \
  } else if (sew == e32) {\
    MX_##NCVT##_PARAMS(e32, narr); \
    BODY; \
  } else if (sew == e64) {\
    MX_##NCVT##_PARAMS(e64, narr); \
    BODY; \
  } else { \
    require(0); \
  } \
  MX_CVT_LOOP_BASE_END \
  REGNAME_WRITE_BAKE(mmax, nmax * amul * narr, reg_sum, true) \

#define MX_NCVT_OCT(BODY, NCVT, narr, sew) \
  require(sew >= e32 && sew  <= e64); \
  MX_CVT_LOOP_BASE(narr, sew) \
  if (sew == e32) { \
    MX_##NCVT##_PARAMS(e32, 4); \
    BODY; \
  } else if (sew == e64) {\
    MX_##NCVT##_PARAMS(e64, narr); \
    BODY; \
  } else { \
    require(0); \
  } \
  MX_CVT_LOOP_BASE_END \
  REGNAME_WRITE_BAKE(mmax, nmax * amul * narr, reg_sum, true) \

#define MB_GENERAL_LOOP_BASE(for_num) \
  for (reg_t m = 0; m < lmul; m++) { \
    for (reg_t i = 0; i < for_num; ++i) { \

#define MB_GENERAL_LOOP_BASE_END \
   } \
  } \
// row when td == ts1 not copy , use break;
#define MB_BOARD_CORE(dir, parm_type, dim) \
  switch(dir){ \
    case 'r' : \
      MB_GENERAL_LOOP_BASE(height) \
      if (!i && !m){ \
        if (td_num == ts1_num) \
          continue; \
      } \
      memcpy(tr_elt_td + (i * cmax) * parm_type / 8 * (dim == 'c' ? amul : 1) + ( m * rmax * cmax), tr_elt_start, parm_type * width / 8); \
      MB_GENERAL_LOOP_BASE_END \
      break; \
    case 'c' : \
      MB_GENERAL_LOOP_BASE(height) \
      if ( dim != 'c') \
        val  = P.MU.tr_elt<type_sew_t<parm_type>::type>(ts1_num + m, 0, i, 0, rmax, cmax, false, false); \
      else \
        val  = P.MU.acc_elt<type_sew_t<parm_type>::type>(ts1_num + m, 0, i, 0, rmax, cmax * amul, false, false); \
      temp.assign(width, val); \
      memcpy(tr_elt_td + (i * cmax) * parm_type / 8 * (dim == 'c' ? amul : 1) + ( m * rmax * cmax), temp.data(), parm_type * width / 8); \
      MB_GENERAL_LOOP_BASE_END \
      break; \
    case 'f' : \
      MB_GENERAL_LOOP_BASE(height) \
      if (!i && !m){ \
        if ( dim != 'c') \
          val  = P.MU.tr_elt<type_sew_t<parm_type>::type>(ts1_num, 0, 0, 0, rmax, cmax, false, false); \
        else \
          val  = P.MU.acc_elt<type_sew_t<parm_type>::type>(ts1_num, 0, 0, 0, rmax, cmax * amul, false, false); \
        temp.assign(width, val); \
      } \
      memcpy(tr_elt_td + (i * cmax) * parm_type / 8 * (dim == 'c' ? amul : 1) + ( m * rmax * cmax), temp.data(), width * parm_type / 8); \
      MB_GENERAL_LOOP_BASE_END \
      break; \
    default  :\
      break; \
  } \

#define MB_PARAM_INIT(dim) \
  if (ts1_num == td_num) { \
    tr_elt_td = tr_elt_start = P.MU.board_elt(ts1_num, rmax, cmax, dim == 'c'); \
  }else { \
    tr_elt_start = P.MU.board_elt(ts1_num, rmax, cmax, dim == 'c'); \
    tr_elt_td = P.MU.board_elt(td_num, rmax, cmax, dim == 'c'); \
  } \

#define MB_PARAM_BASE(sew) \
  require(sew >= e8 && sew <= e64); \
  reg_t tile_m = P.MU.tile_m->read();\
  reg_t tile_k = P.MU.tile_k->read();\
  reg_t tile_n = P.MU.tile_n->read();\
  reg_t rmax = 0; \
  reg_t cmax = 0; \
  reg_t height, width; \
  reg_t amul = P.MU.mamul; \
  reg_t td_num = insn.rd(); \
  reg_t ts1_num = insn.rs1(); \
  char *tr_elt_start = NULL; \
  char *tr_elt_td = NULL; \
  /* reg_t lmul = P.MU.mlmul; */ \
  reg_t lmul = 1; \
  

#define MTR_BROADCAST(dim, dir, sew) \
  MB_PARAM_BASE(sew) \
  MTU_LS_LEN(0, dim, (sew / 8)) \
  MB_PARAM_INIT(dim) \
  switch (sew) { \
    case e8: { \
      type_sew_t<8>::type val = 0; \
      std::vector<type_sew_t<8>::type> temp; \
      MB_BOARD_CORE(dir, 8, dim) \
      break; \
    } \
    case e16: { \
      type_sew_t<16>::type val = 0; \
      std::vector<type_sew_t<16>::type> temp; \
      MB_BOARD_CORE(dir, 16, dim) \
      break; \
    } \
    case e32: { \
      type_sew_t<32>::type val = 0; \
      std::vector<type_sew_t<32>::type> temp; \
      MB_BOARD_CORE(dir, 32, dim) \
      break; \
    } \
    case e64: { \
      type_sew_t<64>::type val = 0; \
      std::vector<type_sew_t<64>::type> temp; \
      MB_BOARD_CORE(dir, 64, dim) \
      break; \
    } \
    default : { \
      require(0); \
      break; \
    } \
  } 

#define MI_MM_LOOP(BODY ,PARAMS, ins, sew) \
  MX_2D_LOOP_BASE(ins, sew) \
  if (sew == e8) { \
    M##PARAMS##_PARAMS(e8); \
    BODY \
  } else if (sew == e16) { \
    M##PARAMS##_PARAMS(e16); \
    BODY \
  } else if (sew == e32) { \
    M##PARAMS##_PARAMS(e32); \
    BODY \
  } else if (sew == e64) { \
    M##PARAMS##_PARAMS(e64); \
    BODY \
  } \
  MX_2D_LOOP_END \
  REGNAME_WRITE_BAKE(mmax, nmax * amul, reg_sum, true) \

#define MI_MM_LOOP_E4(BODY ,PARAMS, ins, sew) \
  MX_2D_LOOP_BASE(ins, sew) \
  nmax *= 2; \
  M##PARAMS##_PARAMS(e8); \
  BODY \
  MX_2D_LOOP_END \
  REGNAME_WRITE_BAKE(mmax, nmax * amul, reg_sum, true) \

#define MI_MM_LOOP_WIDEN(BODY ,PARAMS, ins, sew) \
  MX_2D_LOOP_BASE_WIDEN(ins, sew) \
    MXU_LOOP_ELEMENT_SKIP(); \
    if (sew == e4) { \
    nmax /= 2; \
    M##PARAMS##_PARAMS(e8, 1); \
    BODY; \
  } else if (sew == e8) { \
    M##PARAMS##_PARAMS(e8, 2); \
    BODY \
  } else if (sew == e16) { \
    M##PARAMS##_PARAMS(e16, 2); \
    BODY \
  } else if (sew == e32) { \
    M##PARAMS##_PARAMS(e32, 2); \
    BODY \
  } else { \
    require(0); \
  } \
  MX_2D_LOOP_END \
  REGNAME_WRITE_BAKE(mmax, nmax * amul, reg_sum, true) \

#define MXU_MM_LOOP_QUEN(BODY, PARAMS, wide, sew) \
  MXU_GENERAL_LOOP_BASE_WIDE(false, wide, sew) \
    MXU_LOOP_ELEMENT_SKIP(); \
  if (sew == e8) { \
    M##PARAMS##_PARAMS(e8, 4); \
    BODY \
  } else if (sew == e16) { \
    M##PARAMS##_PARAMS(e16, 4); \
    BODY \
  } else { \
    require(0); \
  } \
  MXU_LOOP_END \


#define MINT_ROUNDING(result, xrm, gb) \
  do { \
    const uint64_t lsb = 1UL << (gb); \
    const uint64_t lsb_half = lsb >> 1; \
    switch (xrm) { \
      case MRM::RNU: \
        result += lsb_half; \
        break; \
      case MRM::RNE: \
        if ((result & lsb_half) && ((result & (lsb_half - 1)) || (result & lsb))) \
          result += lsb; \
        break; \
      case MRM::RDN: \
        break; \
      case MRM::ROD: \
        if (result & (lsb - 1)) \
          result |= lsb; \
        break; \
      case MRM::INVALID_RM: \
        assert(true); \
    } \
  } while (0)

#define MI_2D_VFP_COMMON(widen, sew) \
  require_fp; \
  require((sew == e16 && p->extension_enabled(EXT_ZFH)) || \
          (sew == e32 && p->extension_enabled('F')) || \
          (sew == e64 && p->extension_enabled('D'))); \
  require_matrix(true);\
  require(STATE.frm->read() < 0x5);\
  reg_t tile_m = P.MU.tile_m->read();\
  reg_t tile_n = P.MU.tile_n->read();\
  reg_t td_num = insn.td(); \
  reg_t ts1_num = insn.ts1(); \
  reg_t ts2_num = insn.ts2(); \
  reg_t mmax = P.MU.mrows; \
  reg_t nmax = P.MU.mcols / sew; \
  /* reg_t lmul = insn.mlmul() != LMUL_RESERVE ? (1 << insn.mlmul()) : P.MU.mlmul; */ \
  reg_t lmul = 1; \
  reg_t wide = widen; \
  require_align(insn.td(), lmul); \
  require_align(insn.ts1(), lmul); \
  require_align(insn.rs2(), lmul); \
  softfloat_roundingMode = STATE.frm->read(); \
  bool reg_rename = false; \
  if (td_num == ts1_num || td_num == ts2_num){ \
    reg_rename = true; \
  } \
  reg_t amul = P.MU.mamul; \
  reg_t reg_sum = 1; \
  bool only_one_fix_reg_sum = false; \
  reg_t td_num_lmul = 0; \
  reg_t des_nmax = nmax / wide; 

#define MI_2D_VFP_LOOP_BASE(wide, sew) \
  MI_2D_VFP_COMMON(wide, sew) \
  /*printf("m,k,n = %d, %d, %d\n", tile_m, tile_k, tile_n);*/ \
  for (reg_t i=0; i<tile_m; ++i) { \
    td_num_lmul = 0; \
    for (reg_t m = 0 ; m < lmul; m++ ) { \
      td_num_lmul += m!=0? (tile_n - 1) / des_nmax : 0; \
      for (reg_t j=0; j<tile_n; ++j) { \


#define MI_LOOP_END \
      } \
    } \
  } \

#define MI_2D_VFP_LOOP_END \
  MI_LOOP_END

#define MI_VFP_MM_LOOP(BODY_F8E4M3, BODY_F8E5M2, BODY_F8E3M4, BODY16, BODYB16, BODY32, BODYT32, BODY64, wide, sew) \
  MI_2D_VFP_LOOP_BASE(wide, sew) \
  switch (sew) { \
    case e8: { \
      if (P.MU.mfp8 == MTYPE_FP8E4M3) { \
        MFU_MM_PARAMS(8_e4m3, wide); \
        BODY_F8E4M3;\
      } else if (P.MU.mfp8 == MTYPE_FP8E5M2) { \
        MFU_MM_PARAMS(8_e5m2, wide); \
        BODY_F8E5M2; \
      } else if (P.MU.mfp8 == MTYPE_FP8E4M3) { \
        MFU_MM_PARAMS(8_e3m4, wide); \
        BODY_F8E5M2; \
      } else { \
        require(0); \
      } \
      set_fp_exceptions; \
      break; \
     } \
    case e16: { \
      if (P.MU.mfp16 == MTYPE_FP16) { \
        MFU_MM_PARAMS(16, wide); \
        BODY16; \
      } else if (P.MU.mfp16 == MTYPE_BF16) { \
        MBFU_MM_PARAMS(16, wide); \
        BODYB16; \
      } else { \
        require(0); \
      }\
      set_fp_exceptions; \
      break; \
     } \
     case e32: { \
      if (P.MU.mfp32 == MTYPE_FP32) { \
        MFU_MM_PARAMS(32, wide); \
        BODY32; \
      } else if (P.MU.mfp32 == MTYPE_TFP32) { \
        MTFU_MM_PARAMS(32, wide); \
        BODYT32; \
      } else { \
        require(0); \
      } \
      set_fp_exceptions; \
      break; \
     } \
     case e64: { \
      MFU_MM_PARAMS(64, wide); \
      BODY64; \
      set_fp_exceptions; \
      break; \
     } \
     default: { \
      require(0); \
      break; \
     } \
  } \
  MI_2D_VFP_LOOP_END \
  REGNAME_WRITE_BAKE(mmax, nmax * amul, 1, true) \

#define MTRANSPOSE_BASE(dim, elt_width) \
  require((sew >= e8 && sew <= e64)); \
  require_matrix(true);\
  reg_t td_num = insn.td(); \
  reg_t ts1_num = insn.ts1(); \
  reg_t rmax = 0, cmax = 0; \
  reg_t height = 0, width = 0; \
  reg_t amul = P.MU.mamul; \
  MTU_LS_LEN(0, dim, sizeof(elt_width##_t)) \
  /* mba not 0 must copy agnoic date to dest reg*/ \
  reg_t square_min = std::min(height, width); \
  bool reg_rename = false; \
  reg_t lmul = 1; \
  if (td_num == ts1_num){ \
    reg_rename = true; \
  } \
  bool is_acc = dim == 'c'; \
  
#define MTRANSPOSE(dim, elt_width) \
  reg_t sew = sizeof(elt_width##_t) << 3; \
  MTRANSPOSE_BASE(dim, elt_width) \
  MD2X_LOOP_BASE \
  if ( !is_acc ) { \
    auto ts1 = P.MU.tr_elt<elt_width##_t>(ts1_num, 0, i, j, rmax, cmax, reg_rename, false); \
    if ((i >= square_min || j >= square_min) && !P.MU.mba){ \
      auto &td = P.MU.tr_elt<elt_width##_t>(td_num, 0, i, j, rmax, cmax, reg_rename, true); \
      td = ts1; \
    } else { \
      auto &td = P.MU.tr_elt<elt_width##_t>(td_num, 0, j, i, rmax, cmax, reg_rename, true); \
      td = ts1; \
    } \
  } \
  else { \
    auto acc1 = P.MU.acc_elt<elt_width##_t>(ts1_num, 0, i, j, rmax, cmax * amul, reg_rename, false); \
    if ((i >= square_min || j >= square_min) && !P.MU.mba){ \
      auto &accd = P.MU.acc_elt<elt_width##_t>(td_num, 0, i, j, rmax, cmax * amul, reg_rename, true); \
      accd = acc1; \
    } else { \
      auto &accd = P.MU.acc_elt<elt_width##_t>(td_num, 0, j, i, rmax, cmax * amul, reg_rename, true); \
      accd = acc1; \
    } \
  } \
  MD2X_LOOP_END \
  REGNAME_WRITE_BAKE(td_num, is_acc ? cmax * amul: cmax, 1, is_acc) \


#endif // _RISCV_M_EXT_MACROS_H