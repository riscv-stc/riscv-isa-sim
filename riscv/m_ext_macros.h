#ifndef _RISCV_M_EXT_MACROS_H
#define _RISCV_M_EXT_MACROS_H

#include "vector_unit.h"
#include "v_ext_macros.h"
#include <math.h>

#define LMUL_RESERVE 3

#define P_SET_OM(ov) \
  if (ov) P.MU.mxsat->write(1);

#define MX_PARAMS(x) \
  type_sew_t<x>::type &td  = P.MU.tr_elt<type_sew_t<x>::type>(td_num + m, 0, i, j, mmax, nmax, reg_rename, true); \
  type_sew_t<x>::type &ts1  = P.MU.tr_elt<type_sew_t<x>::type>(ts1_num + m, 0, i, k, mmax, nmax, false, false); \
  type_sew_t<x>::type &ts2  = P.MU.tr_elt<type_sew_t<x>::type>(ts2_num + m, 0, k, j, mmax, nmax, false, false); \

#define MX2D_PARAMS(x) \
  type_sew_t<x>::type &td  = P.MU.tr_elt<type_sew_t<x>::type>(td_num + m, 0, i, j, mmax, nmax, reg_rename, true); \
  type_sew_t<x>::type ts1  = P.MU.tr_elt<type_sew_t<x>::type>(ts1_num + m, 0, i, j, mmax, nmax, false, false); \
  type_sew_t<x>::type ts2  = P.MU.tr_elt<type_sew_t<x>::type>(ts2_num + m, 0, i, j, mmax, nmax, false, false); \

#define MXSU_PARAMS(x) \
  type_sew_t<x>::type &td  = P.MU.tr_elt<type_sew_t<x>::type>(td_num + m, 0, i, j, mmax, nmax, reg_rename, true); \
  type_sew_t<x>::type ts1  = P.MU.tr_elt<type_sew_t<x>::type>(ts1_num + m, 0, i, j, mmax, nmax, false, false); \
  type_usew_t<x>::type ts2  = P.MU.tr_elt<type_usew_t<x>::type>(ts2_num + m, 0, i, j, mmax, nmax, false, false); \

#define MX_TS1_PARAMS(x) \
  type_sew_t<x>::type ts1  = P.MU.tr_elt<type_sew_t<x>::type>(ts1_num + m, 0, i, j, mmax, nmax, false, false); \

#define MXU_TS1_PARAMS(x) \
  type_usew_t<x>::type ts1  = P.MU.tr_elt<type_usew_t<x>::type>(ts1_num + m, 0, i, k, mmax, nmax, false, false); \

#define MX_TS2_PARAMS(x) \
  type_sew_t<x>::type ts2  = P.MU.tr_elt<type_sew_t<x>::type>(ts2_num + m, 0, i, k, mmax, nmax, false, false); \

#define MXU_TS2_PARAMS(x) \
  type_usew_t<x>::type ts2  = P.MU.tr_elt<type_usew_t<x>::type>(ts2_num + m, 0, i, j, mmax, nmax, false, false); \

#define MXU_PARAMS(x) \
  type_usew_t<x>::type &td  = P.MU.tr_elt<type_usew_t<x>::type>(td_num + m, 0, i, j, mmax, nmax, reg_rename, true); \
  type_usew_t<x>::type &ts1  = P.MU.tr_elt<type_usew_t<x>::type>(ts1_num + m, 0, i, k, mmax, nmax, false, false); \
  type_usew_t<x>::type &ts2  = P.MU.tr_elt<type_usew_t<x>::type>(ts2_num + m, 0, k, j, mmax, nmax, false, false); \

#define MXU2D_PARAMS(x) \
  type_usew_t<x>::type &td  = P.MU.tr_elt<type_usew_t<x>::type>(td_num + m, 0, i, j, mmax, nmax, reg_rename, true); \
  type_usew_t<x>::type ts1  = P.MU.tr_elt<type_usew_t<x>::type>(ts1_num + m, 0, i, j, mmax, nmax, false, false); \
  type_usew_t<x>::type ts2  = P.MU.tr_elt<type_usew_t<x>::type>(ts2_num + m, 0, i, j, mmax, nmax, false, false); \

#define MX_PARAM_BASE(x) \
  type_sew_t<x>::type ts1  = P.MU.tr_elt<type_sew_t<x>::type>(ts1_num + m, 0, i, k, mmax, nmax, false, false); \
  type_sew_t<x>::type ts2  = P.MU.tr_elt<type_sew_t<x>::type>(ts2_num + m, 0, k, j, mmax, nmax, false, false); \

#define MX_PARAM2D_BASE(x) \
  type_sew_t<x>::type ts1  = P.MU.tr_elt<type_sew_t<x>::type>(ts1_num + m, 0, i, j, mmax, nmax, false, false); \
  type_sew_t<x>::type ts2  = P.MU.tr_elt<type_sew_t<x>::type>(ts2_num + m, 0, i, j, mmax, nmax, false, false); \

#define MXU_PARAM_BASE(x) \
  type_usew_t<x>::type ts1  = P.MU.tr_elt<type_usew_t<x>::type>(ts1_num + m, 0, i, k, mmax, nmax, false, false); \
  type_usew_t<x>::type ts2  = P.MU.tr_elt<type_usew_t<x>::type>(ts2_num + m, 0, k, j, mmax, nmax, false, false); \

#define MMULXU_PARAMS(x, mult) \
  type_usew_t<x * mult>::type &td  = P.MU.tr_elt<type_usew_t<x * mult>::type>(td_num + m + td_num_lmul + j / des_nmax, \
        0, i, j % (nmax / mult), mmax, nmax / mult, reg_rename, true); \
  MXU_PARAM_BASE(x) \

#define MMULXU2D_PARAMS(x, mult) \
  type_usew_t<x * mult>::type &td  = P.MU.tr_elt<type_usew_t<x * mult>::type>(td_num + m + td_num_lmul + j / des_nmax, \
        0, i, j % (nmax / mult), mmax, nmax / mult, reg_rename, true); \
  type_usew_t<x>::type ts1  = P.MU.tr_elt<type_usew_t<x>::type>(ts1_num + m, 0, i, j, mmax, nmax, false, false); \
  type_usew_t<x>::type ts2  = P.MU.tr_elt<type_usew_t<x>::type>(ts2_num + m, 0, i, j, mmax, nmax, false, false); \

#define MMULX_PARAMS(x, mult) \
  type_sew_t<x * mult>::type &td  = P.MU.tr_elt<type_sew_t<x * mult>::type>(td_num + m + td_num_lmul + j / des_nmax, \
          0, i, j % (nmax / mult), mmax, nmax / mult, reg_rename, true); \
  MX_PARAM_BASE(x) \

#define MMULX2D_PARAMS(x, mult) \
  MX_PARAM2D_BASE(x) \

#define MMULXXU_PARAMS(x, mult) \
  type_sew_t<x * mult>::type &td  = P.MU.tr_elt<type_sew_t<x * mult>::type>(td_num + m + td_num_lmul + j / des_nmax, \
          0, i, j % (nmax / mult), mmax, nmax / mult, reg_rename, true); \
  MX_TS1_PARAMS(x) \
  MXU_TS2_PARAMS(x) \

#define MXXU_PARAMS(x) \
  type_sew_t<x>::type &td  = P.MU.tr_elt<type_sew_t<x>::type>(td_num + m, 0, i, j, mmax, nmax, reg_rename, true); \
  type_sew_t<x>::type ts1  = P.MU.tr_elt<type_sew_t<x>::type>(ts1_num + m, 0, i, j, mmax, nmax, false, false); \
  type_usew_t<x>::type ts2  = P.MU.tr_elt<type_usew_t<x>::type>(ts2_num + m, 0, i, j, mmax, nmax, false, false); \

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

#define MFU_PARAMS_TR1(width) \
  float##width##_t UNUSED &ts1  = P.MU.tr_elt<float##width##_t>(ts1_num + m, 0, i, j, mmax, nmax, false, false); \

#define MFU_PARAMS_TR2(width) \
  float##width##_t UNUSED &ts2  = P.MU.tr_elt<float##width##_t>(ts2_num + m, 0, i, j, mmax, nmax, false, false); \

#define MFU_MM_PARAMS(width) \
  float##width##_t &td = P.MU.tr_elt<float##width##_t>(td_num + m, 0, i, j, mmax, nmax, reg_rename, true); \
  MFU_PARAMS_TR1(width) \
  MFU_PARAMS_TR2(width) \

#define REGNAME_WRITE_BAKE(tr_mmax, tr_nmax, reg_sum) \
  if (reg_rename) { \
    P.MU.reg_rename_write_back_elt(td_num, tr_mmax, tr_nmax, lmul, reg_sum); \
  } \

#define MMV_TR_XPR(BODY, num) \
  require(P.MU.msew >= e8 && P.MU.msew <= e64); \
  reg_t sew = P.MU.msew; \
  reg_t i = RS2 & 0xFF; \
  reg_t j = (RS2 >> 16); \
  reg_t ts1_num = num; \
  reg_t mmax = P.MU.mrows; \
  reg_t nmax = P.MU.mcols / sew; \
  reg_t m = 0; \
  if (i >mmax || j > nmax) \
    require(0); \
  switch (sew) { \
    case e8: { \
      MXX_PARAMS_TR1(8); \
      BODY; \
      break; \
    } \
    case e16: { \
      MXX_PARAMS_TR1(16); \
      BODY \
      break; \
    } \
    case e32: { \
      MXX_PARAMS_TR1(32); \
      BODY; \
      break; \
    } \
    default : \
      MXX_PARAMS_TR1(64); \
      BODY; \
      break; \
  } \

#define MMV_TR_BASE(num) \
  require(P.MU.msew >= e8 && P.MU.msew <= e64); \
  reg_t sew = P.MU.msew; \
  reg_t i = RS2 & 0xFF; \
  reg_t j = (RS2 >> 16); \
  reg_t ts1_num = num; \
  reg_t mmax = P.MU.mrows; \
  reg_t nmax = P.MU.mcols / sew; \
  reg_t m = 0; \

#define MMV_TR_FPR(BODY, BODY16, BODY32, BODY64, num) \
  MMV_TR_BASE(num) \
  switch (sew) { \
    case e16: { \
      MFU_PARAMS_TR1(16) \
      BODY16; \
      BODY; \
      break; \
    } \
    case e32: { \
      MFU_PARAMS_TR1(32) \
      BODY32; \
      BODY; \
      break; \
    } \
    case e64:  { \
      MFU_PARAMS_TR1(64) \
      BODY64; \
      BODY; \
      break; \
    } \
    default : \
      break; \
  } \
  

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


#define MMV_GENERAL_LOOP_BASE(is_trans, dim) \
  require(P.MU.msew >= e8 && P.MU.msew <= e64); \
  require(P.VU.vsew >= e8 && P.VU.vsew <= e64); \
  reg_t sew = P.MU.msew; \
  reg_t rd_num = insn.rd(); \
  reg_t rs1_num = insn.rs1(); \
  reg_t start_height = RS2; \
  reg_t height, width; \
  reg_t rmax = 0, cmax = 0; \
  reg_t lmul = P.MU.mlmul; \
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

#define MTU_TR_VREG_PARAMS(trans, x) \
  type_sew_t<x>::type vs1 = P.VU.elt<type_sew_t<x>::type>(rs1_num + m, i*width+j); \
  type_sew_t<x>::type &td = P.MU.tr_elt<type_sew_t<x>::type>(rd_num + m, trans, i + start_height, j, rmax, cmax, false, true); \


#define MTU_VM_LOOP_END \
    } \
  } \
  } \

// vreg <-- tr
#define MMV_VREG_FROM_TR(is_trans, dim) \
  require(P.MU.msew == P.VU.vsew);\
  MMV_GENERAL_LOOP_BASE(is_trans, dim) \
  if (sew == e8){ \
    MTU_VREG_TR_PARAMS(is_trans, e8); \
    vd = ts1; \
  }else if(sew == e16){ \
    MTU_VREG_TR_PARAMS(is_trans, e16); \
    vd = ts1; \
  }else if(sew == e32){ \
    MTU_VREG_TR_PARAMS(is_trans, e32); \
    vd = ts1; \
  }else if(sew == e64){ \
    MTU_VREG_TR_PARAMS(is_trans, e64); \
    vd = ts1; \
  } \
  MTU_VM_LOOP_END 

// tr <-- vreg
#define MMV_TR_FROM_VREG(is_trans, dim) \
  require(P.MU.msew == P.VU.vsew); \
  MMV_GENERAL_LOOP_BASE(is_trans, dim) \
  if (sew == e8){ \
    MTU_TR_VREG_PARAMS(is_trans, e8); \
    td = vs1; \
  }else if(sew == e16){ \
    MTU_TR_VREG_PARAMS(is_trans, e16); \
    td = vs1; \
  }else if(sew == e32){ \
    MTU_TR_VREG_PARAMS(is_trans, e32); \
    td = vs1; \
  }else if(sew == e64){ \
    MTU_TR_VREG_PARAMS(is_trans, e64); \
    td = vs1; \
  } \
  MTU_VM_LOOP_END 

#define MXU_GENERAL_LOOP_BASE(ins) \
  require(P.MU.msew >= e8 && P.MU.msew <= e64); \
  reg_t tile_m = P.MU.tile_m->read();\
  reg_t tile_k = P.MU.tile_k->read();\
  reg_t tile_n = P.MU.tile_n->read();\
  reg_t mmax = P.MU.mrows;\
  reg_t nmax = P.MU.mcols / P.MU.msew;\
  reg_t kmax = std::min(nmax, P.MU.mrows);\
  reg_t sew = P.MU.msew; \
  reg_t td_num = insn.td(); \
  reg_t ts1_num = insn.ts1(); \
  reg_t ts2_num = insn.rs2(); \
  reg_t lmul = P.MU.mlmul; \
  reg_t reg_sum = 1; \
  if (ins && insn.mlmul() != LMUL_RESERVE) \
    lmul = (1 << insn.mlmul()); \
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

#define MXU_LOOP_BASE \
    MXU_GENERAL_LOOP_BASE(false) \
    MXU_LOOP_ELEMENT_SKIP();

#define MXU_GENERAL_LOOP_BASE_WIDE(ins, wide) \
  require(P.MU.msew >= e8 && P.MU.msew <= e64); \
  reg_t tile_m = P.MU.tile_m->read();\
  reg_t tile_k = P.MU.tile_k->read();\
  reg_t tile_n = P.MU.tile_n->read();\
  reg_t mmax = P.MU.mrows;\
  reg_t nmax = P.MU.mcols / P.MU.msew;\
  reg_t kmax = std::min(nmax, P.MU.mrows);\
  reg_t sew = P.MU.msew; \
  reg_t td_num = insn.td(); \
  reg_t ts1_num = insn.ts1(); \
  reg_t ts2_num = insn.rs2(); \
  reg_t lmul = P.MU.mlmul; \
  reg_t reg_sum = 1; \
  if (ins && insn.mlmul() != LMUL_RESERVE) \
    lmul = (1 << insn.mlmul()); \
  bool reg_rename = false; \
  if (td_num == ts1_num || td_num == ts2_num){ \
    reg_rename = true; \
  } \
  bool only_one_fix_reg_sum = false; \
  reg_t td_num_lmul = 0; \
  reg_t des_nmax = nmax / wide; \
  for (reg_t i = 0; i < tile_m; ++i) { \
    td_num_lmul = 0; \
    for (reg_t m = 0 ; m < lmul; m++) { \
      td_num_lmul += m!=0? (tile_n - 1) / des_nmax : 0; \
      for (reg_t j = 0; j < tile_n; ++j) { \
        for (reg_t k = 0; k < tile_k; ++k) { \

#define MXU_LOOP_BASE_WIDE(wide) \
    MXU_GENERAL_LOOP_BASE_WIDE(false, wide) \
    MXU_LOOP_ELEMENT_SKIP();

#define MXU_LOOP_END \
        } \
      } \
    } \
  } \

#define MXU_VFP_LOOP_END \
  MXU_LOOP_END \


#define MXU_MM_LOOP(BODY, TYPE) \
  MXU_LOOP_BASE \
  if (sew == e8){ \
    M##TYPE##_PARAMS(e8); \
    BODY; \
  }else if(sew == e16){ \
    M##TYPE##_PARAMS(e16); \
    BODY; \
  }else if(sew == e32){ \
    M##TYPE##_PARAMS(e32); \
    BODY; \
  }else if(sew == e64){ \
    M##TYPE##_PARAMS(e64); \
    BODY; \
  } \
  MXU_LOOP_END \
  REGNAME_WRITE_BAKE(mmax, nmax, reg_sum) \

#define MXU_MM_LOOP_WIDE(BODY ,PARAMS, wide) \
  MXU_GENERAL_LOOP_BASE_WIDE(false, wide) \
    MXU_LOOP_ELEMENT_SKIP(); \
    if (!only_one_fix_reg_sum){ \
      reg_sum += (tile_n -1) / des_nmax; \
      only_one_fix_reg_sum = true; \
    } \
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
  REGNAME_WRITE_BAKE(mmax, nmax, reg_sum) \

#define MXU_CHECK_OVERFLOW(eew) \
  res = res > std::numeric_limits<eew>::max()? std::numeric_limits<eew>::max() : res; \
  res = res < std::numeric_limits<eew>::min()? std::numeric_limits<eew>::min() : res; \
  

#define MX_2D_GENERAL_LOOP_BASE(ins) \
  require(P.MU.msew >= e8 && P.MU.msew <= e64); \
  reg_t tile_m = P.MU.tile_m->read();\
  reg_t tile_n = P.MU.tile_n->read();\
  reg_t mmax = P.MU.mrows;\
  reg_t nmax = P.MU.mcols / P.MU.msew;\
  reg_t sew = P.MU.msew; \
  reg_t td_num = insn.td(); \
  reg_t ts1_num = insn.ts1(); \
  reg_t ts2_num = insn.rs2(); \
  reg_t lmul = P.MU.mlmul; \
  reg_t reg_sum = 1; \
  if (ins && insn.mlmul() != LMUL_RESERVE) \
    lmul = (1 << insn.mlmul()); \
  bool reg_rename = false; \
  if (td_num == ts1_num || td_num == ts2_num){ \
    reg_rename = true; \
  } \
  bool only_one_fix_reg_sum = false; \
  for (reg_t i = 0; i < tile_m; ++i) { \
    for (reg_t m = 0 ; m < lmul; m++) { \
      for (reg_t j = 0; j < tile_n; ++j) { \

#define MX_2D_LOOP_ELEMENT_SKIP(BODY)

#define MX_2D_LOOP_BASE(ins) \
    MX_2D_GENERAL_LOOP_BASE(ins) \
    MX_2D_LOOP_ELEMENT_SKIP();

#define MX_2D_LOOP_END \
      } \
    } \
  } \

#define MX_2D_GENERAL_LOOP_BASE_WIDE(ins) \
  require(P.MU.msew >= e8 && P.MU.msew <= e64); \
  reg_t tile_m = P.MU.tile_m->read();\
  reg_t tile_n = P.MU.tile_n->read();\
  reg_t mmax = P.MU.mrows;\
  reg_t nmax = P.MU.mcols / P.MU.msew;\
  reg_t sew = P.MU.msew; \
  reg_t td_num = insn.td(); \
  reg_t ts1_num = insn.ts1(); \
  reg_t ts2_num = insn.rs2(); \
  reg_t lmul = P.MU.mlmul; \
  reg_t reg_sum = 1; \
  if (ins && insn.mlmul() != LMUL_RESERVE) \
    lmul = (1 << insn.mlmul()); \
  bool reg_rename = false; \
  if (td_num == ts1_num || td_num == ts2_num){ \
    reg_rename = true; \
  } \
  bool only_one_fix_reg_sum = false; \
  reg_t td_num_lmul = 0; \
  reg_t des_nmax = nmax / 2; \
  for (reg_t i = 0; i < tile_m; ++i) { \
    td_num_lmul = 0; \
    for (reg_t m = 0 ; m < lmul; m++) { \
      td_num_lmul += m!=0? (tile_n - 1) / des_nmax : 0; \
      for (reg_t j = 0; j < tile_n; ++j) { \

#define MX_2D_LOOP_BASE_WIDEN(ins) \
    MX_2D_GENERAL_LOOP_BASE_WIDE(ins) \
    MX_2D_LOOP_ELEMENT_SKIP();


#define MXU_MM_ADD(opd, op0, sign, type_t, td_type, saturated) \
  require(P.MU.msew >= e8 && P.MU.msew <= e64); \
  reg_t lmul = insn.mlmul() != LMUL_RESERVE ? (1 << insn.mlmul()) : P.MU.mlmul; \
  require_align(insn.td(), lmul); \
  require_align(insn.ts1(), lmul); \
  require_align(insn.rs2(), lmul); \
  reg_t tile_m = P.MU.tile_m->read() * lmul;\
  reg_t tile_n = P.MU.tile_n->read() * lmul;\
  reg_t sew = P.MU.msew; \
  reg_t mmax = P.MU.mrows;\
  reg_t nmax = P.MU.mcols / P.MU.msew;\
  reg_t td_num = insn.td(); \
  reg_t ts1_num = insn.ts1(); \
  reg_t ts2_num = insn.rs2(); \
  type_t res; \
  bool reg_rename = false; \
  if (td_num == ts1_num || td_num == ts2_num){ \
    reg_rename = true; \
  } \
  for (reg_t i = 0; i < tile_m; ++i) { \
    for (reg_t m = 0; m < lmul; m++) {\
      for (reg_t j = 0; j < tile_n; ++j) { \
        if (sew == e8){ \
          auto &td = P.MU.tr_elt<sign<e8>::type>(td_num + m , 0, i, j, mmax, nmax, reg_rename, true); \
          auto ts1  = P.MU.tr_elt<sign<e8>::type>(ts1_num + m, 0, i, j, mmax, nmax, false, false); \
          auto ts2  = P.MU.tr_elt<sign<e8>::type>(ts2_num + m, 0, i, j, mmax, nmax, false, false); \
          res = (type_t)ts1 op0 (type_t)ts2; \
          if (saturated) { \
            MXU_CHECK_OVERFLOW(td_type##8_t) \
          } \
          td = (td_type##8_t)res; \
        }else if(sew == e16){ \
          auto &td = P.MU.tr_elt<sign<e16>::type>(td_num + m, 0, i, j, mmax, nmax, reg_rename, true); \
          auto ts1  = P.MU.tr_elt<sign<e16>::type>(ts1_num + m, 0, i, j, mmax, nmax, false, false); \
          auto ts2  = P.MU.tr_elt<sign<e16>::type>(ts2_num + m, 0, i, j, mmax, nmax, false, false); \
          res = (type_t)ts1 op0 (type_t)ts2; \
          if (saturated) { \
            MXU_CHECK_OVERFLOW(td_type##16_t) \
          } \
          td = (td_type##16_t)res; \
        }else if(sew == e32){ \
          auto &td = P.MU.tr_elt<sign<e32>::type>(td_num + m, 0, i, j, mmax, nmax, reg_rename, true); \
          auto ts1  = P.MU.tr_elt<sign<e32>::type>(ts1_num + m, 0, i, j, mmax, nmax, false, false); \
          auto ts2  = P.MU.tr_elt<sign<e32>::type>(ts2_num + m, 0, i, j, mmax, nmax, false, false); \
          res = (type_t)ts1 op0 (type_t)ts2; \
          if (saturated) { \
            MXU_CHECK_OVERFLOW(td_type##32_t) \
          } \
          td = (td_type##32_t)res; \
        }else if(sew == e64){ \
          auto &td = P.MU.tr_elt<sign<e64>::type>(td_num + m, 0, i, j, mmax, nmax, reg_rename, true); \
          auto ts1  = P.MU.tr_elt<sign<e64>::type>(ts1_num + m, 0, i, j, mmax, nmax, false, false); \
          auto ts2  = P.MU.tr_elt<sign<e64>::type>(ts2_num + m, 0, i, j, mmax, nmax, false, false); \
          res = (type_t)ts1 op0 (type_t)ts2; \
          if (saturated) { \
            MXU_CHECK_OVERFLOW(td_type##64_t) \
          } \
          td = (td_type##64_t)res; \
        } \
      } \
    } \
  } \
  REGNAME_WRITE_BAKE(mmax, nmax, 1); \

#define MXU_W_MM_ADD(opd, op0, sign, type_t, td_type, saturated) \
  require(P.MU.msew >= e8 && P.MU.msew <= e32); \
  reg_t lmul = insn.mlmul() != LMUL_RESERVE ? (1 << insn.mlmul()) : P.MU.mlmul; \
  require_align(insn.td(), lmul); \
  require_align(insn.ts1(), lmul); \
  require_align(insn.rs2(), lmul); \
  reg_t tile_m = P.MU.tile_m->read();\
  reg_t tile_n = P.MU.tile_n->read();\
  reg_t sew = P.MU.msew; \
  reg_t td_num = insn.td(); \
  reg_t ts1_num = insn.ts1(); \
  reg_t ts2_num = insn.rs2(); \
  reg_t mmax = P.MU.mrows;\
  reg_t nmax = P.MU.mcols / P.MU.msew;\
  type_t res; \
  bool reg_rename = false; \
  if (td_num == ts1_num || td_num == ts2_num){ \
    reg_rename = true; \
  } \
  reg_t des_nmax = nmax/2; \
  reg_t reg_sum = 1 + (tile_n - 1) / des_nmax; \
  reg_t td_num_lmul = 0; \
  for (reg_t i = 0; i < tile_m; ++i) { \
    td_num_lmul = 0; \
    for (reg_t m = 0; m < lmul; m++) { \
      td_num_lmul += (m != 0) ? ((tile_n - 1) / des_nmax) : 0; \
      for (reg_t j = 0; j < tile_n; ++j) { \
        if (sew == e8){ \
          auto &td = P.MU.tr_elt<sign<e16>::type>(td_num + m + td_num_lmul + j / des_nmax, 0, i, j % des_nmax, mmax, des_nmax, reg_rename, true); \
          auto ts1  = P.MU.tr_elt<sign<e8>::type>(ts1_num + m, 0, i, j, mmax, nmax, false, false); \
          auto ts2  = P.MU.tr_elt<sign<e8>::type>(ts2_num + m, 0, i, j, mmax, nmax, false, false); \
          res = (type_t)ts1 op0 (type_t)ts2; \
          if (saturated) { \
            MXU_CHECK_OVERFLOW(td_type##16_t) \
          } \
          td = (td_type##16_t)res; \
        }else if(sew == e16){ \
          auto &td = P.MU.tr_elt<sign<e32>::type>(td_num + m + td_num_lmul + j / des_nmax, 0, i, j % des_nmax, mmax, des_nmax, reg_rename, true); \
          auto ts1  = P.MU.tr_elt<sign<e16>::type>(ts1_num + m, 0, i, j, mmax, nmax, false, false); \
          auto ts2  = P.MU.tr_elt<sign<e16>::type>(ts2_num + m, 0, i, j, mmax, nmax, false, false); \
          res = (type_t)ts1 op0 (type_t)ts2; \
          if (saturated) { \
            MXU_CHECK_OVERFLOW(td_type##32_t) \
          } \
          td = (td_type##32_t)res; \
        }else if(sew == e32){ \
          auto &td = P.MU.tr_elt<sign<e64>::type>(td_num + m + td_num_lmul + j / des_nmax, 0, i, j % des_nmax, mmax, des_nmax, reg_rename, true); \
          auto ts1  = P.MU.tr_elt<sign<e32>::type>(ts1_num + m, 0, i, j, mmax, nmax, false, false); \
          auto ts2  = P.MU.tr_elt<sign<e32>::type>(ts2_num + m, 0, i, j, mmax, nmax, false, false); \
          res = (type_t)ts1 op0 (type_t)ts2; \
          if (saturated) { \
            MXU_CHECK_OVERFLOW(td_type##64_t) \
          } \
          td = (td_type##64_t)res; \
        }\
      } \
    } \
  } \
  REGNAME_WRITE_BAKE(mmax, nmax, reg_sum); \

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
  REGNAME_WRITE_BAKE(mmax, nmax, reg_sum); \



#define MXU_WIDE_OP_AND_ASSIGN(var0, var1, var2, op0, op1, sign) \
  switch(P.MU.msew) { \
  case e8: { \
    sign##16_t td_w = P.MU.tr_elt<sign##16_t>(td_num + m + td_num_lmul + j / des_nmax, 0, i, j % des_nmax, mmax, des_nmax, reg_rename, false); \
    P.MU.tr_elt<uint16_t>(td_num + m + td_num_lmul + j / des_nmax, 0, i, j % des_nmax, mmax, des_nmax, reg_rename, true) = \
      op1((sign##16_t)(sign##8_t)var0 op0 (sign##16_t)(sign##8_t)var1) + var2; \
    } \
    break; \
  case e16: { \
    sign##32_t td_w = P.MU.tr_elt<sign##32_t>(td_num + m + td_num_lmul + j / des_nmax, 0, i, j % des_nmax, mmax, des_nmax, reg_rename, false); \
    P.MU.tr_elt<uint32_t>(td_num + m + td_num_lmul + j / des_nmax, 0, i, j % des_nmax, mmax, des_nmax, reg_rename, true) = \
      op1((sign##32_t)(sign##16_t)var0 op0 (sign##32_t)(sign##16_t)var1) + var2; \
    } \
    break; \
  default: { \
    sign##64_t td_w = P.MU.tr_elt<sign##64_t>(td_num + m + td_num_lmul + j / des_nmax, 0, i, j % des_nmax, mmax, des_nmax, reg_rename, false); \
    P.MU.tr_elt<uint64_t>(td_num + m + td_num_lmul + j / des_nmax, 0, i, j % des_nmax, mmax, des_nmax, reg_rename, true) = \
      op1((sign##64_t)(sign##32_t) var0 op0 (sign##64_t)(sign##32_t)var1) + var2; \
    } \
    break; \
  }

#define MXU_WIDE_OP_AND_ASSIGN_MIX(var0, var1, var2, op0, op1, signd, sign_1, sign_2) \
  switch(P.MU.msew) { \
  case e8: { \
    signd##16_t UNUSED td_w = P.MU.tr_elt<signd##16_t>(td_num + m + td_num_lmul + j / des_nmax, 0, i, j % des_nmax, mmax, des_nmax, reg_rename, false); \
    P.MU.tr_elt<uint16_t>(td_num + m + td_num_lmul + j / des_nmax, 0, i, j % des_nmax, mmax, des_nmax, reg_rename, true) = \
      op1((sign_1##16_t)(sign_1##8_t)var0 op0 (sign_2##16_t)(sign_2##8_t)var1) + var2; \
    } \
    break; \
  case e16: { \
    signd##32_t td_w = P.MU.tr_elt<signd##32_t>(td_num + m + td_num_lmul + j / des_nmax, 0, i, j % des_nmax, mmax, des_nmax, reg_rename, false); \
    P.MU.tr_elt<uint32_t>(td_num + m + td_num_lmul + j / des_nmax, 0, i, j % des_nmax, mmax, des_nmax, reg_rename, true) = \
      op1((sign_1##32_t)(sign_1##16_t)var0 op0 (sign_2##32_t)(sign_2##16_t)var1) + var2; \
    } \
    break; \
  default: { \
    signd##64_t td_w = P.MU.tr_elt<signd##64_t>(td_num + m + td_num_lmul + j / des_nmax, 0, i, j % des_nmax, mmax, des_nmax, reg_rename, false); \
    P.MU.tr_elt<uint64_t>(td_num + m + td_num_lmul + j / des_nmax, 0, i, j % des_nmax, mmax, des_nmax, reg_rename, true) = \
      op1((sign_1##64_t)(sign_1##32_t) var0 op0 (sign_2##64_t)(sign_2##32_t)var1) + var2; \
    } \
    break; \
  }

#define MXU_QUAD_OP_AND_ASSIGN(var0, var1, var2, op0, op1, sign) \
  switch(P.MU.msew) { \
  case e8: { \
    sign##32_t td_q = P.MU.tr_elt<sign##32_t>(td_num + m + td_num_lmul + j / des_nmax, 0, i, j % des_nmax, mmax, des_nmax, reg_rename, false); \
    P.MU.tr_elt<uint32_t>(td_num + m + td_num_lmul + j / des_nmax, 0, i, j % des_nmax, mmax, des_nmax, reg_rename, true) = \
      op1((sign##32_t)(sign##8_t)var0 op0 (sign##32_t)(sign##8_t)var1) + var2; \
    } \
    break; \
  case e16: { \
    sign##64_t td_q = P.MU.tr_elt<sign##64_t>(td_num + m + td_num_lmul + j / des_nmax, 0, i, j % des_nmax, mmax, des_nmax, reg_rename, false); \
    P.MU.tr_elt<uint64_t>(td_num + m + td_num_lmul + j / des_nmax, 0, i, j % des_nmax, mmax, des_nmax, reg_rename, true) = \
      op1((sign##64_t)(sign##16_t)var0 op0 (sign##64_t)(sign##16_t)var1) + var2; \
    } \
    break; \
  }


#define VM_WIDE_CHECK_COMMON \
  require_matrix(false);\

#define VM_CHECK_DSS(is_vs1) \
  VM_WIDE_CHECK_COMMON; \

// widen operation loop
#define MXU_VV_LOOP_WIDEN(BODY, wide) \
  MXU_LOOP_BASE_WIDE(wide) \
  if (sew == e8){ \
    MX_PARAMS(e8); \
    BODY; \
  }else if(sew == e16){ \
    MX_PARAMS(e16); \
    BODY; \
  } else if(sew == e32){ \
    MX_PARAMS(e32); \
    BODY; \
  } \
  MXU_LOOP_END

#define MXU_VFP_COMMON(wide) \
  require_fp; \
  require((P.MU.msew == e16 && p->extension_enabled(EXT_ZFH)) || \
          (P.MU.msew == e32 && p->extension_enabled('F')) || \
          (P.MU.msew == e64 && p->extension_enabled('D'))); \
  require_matrix(true);\
  require(STATE.frm->read() < 0x5);\
  reg_t tile_m = P.MU.tile_m->read();\
  reg_t tile_k = P.MU.tile_k->read();\
  reg_t tile_n = P.MU.tile_n->read();\
  reg_t sew = P.MU.msew; \
  reg_t td_num = insn.td(); \
  reg_t ts1_num = insn.ts1(); \
  reg_t ts2_num = insn.rs2(); \
  reg_t mmax = P.MU.mrows;\
  reg_t nmax = P.MU.mcols / P.MU.msew;\
  reg_t kmax = std::min(nmax, P.MU.mrows);\
  reg_t lmul = P.MU.mlmul; \
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

#define MXU_VFP_LOOP_BASE(wide) \
  MXU_VFP_COMMON(wide) \
  /*printf("m,k,n = %d, %d, %d\n", tile_m, tile_k, tile_n);*/ \
  for (reg_t i=0; i<tile_m; ++i) { \
    td_num_lmul = 0; \
    for (reg_t m = 0; m < lmul; m++) { \
      td_num_lmul += m!=0? (tile_n - 1) / des_nmax : 0; \
      for (reg_t j=0; j<tile_n; ++j) { \
        for (reg_t k = 0; k < tile_k; ++k) { \

#define MXU_VFP_VV_LOOP(BODY16, BODY32, BODY64, wide) \
  MXU_VFP_LOOP_BASE(wide) \
  switch(P.MU.msew) { \
    case e16: { \
      float16_t &td = P.MU.tr_elt<float16_t>(td_num + m, 0, i, j, mmax, nmax, reg_rename, true); \
      float16_t ts1 = P.MU.tr_elt<float16_t>(ts1_num + m, 0, i, k, mmax, nmax, false, false); \
      float16_t ts2 = P.MU.tr_elt<float16_t>(ts2_num + m, 0, k, j, mmax, nmax, false, false); \
      BODY16; \
      set_fp_exceptions; \
      break; \
    }\
    case e32: {\
      float32_t &td = P.MU.tr_elt<float32_t>(td_num + m, 0, i, j, mmax, nmax, reg_rename, true); \
      float32_t ts1 = P.MU.tr_elt<float32_t>(ts1_num + m, 0, i, k, mmax, nmax, false, false); \
      float32_t ts2 = P.MU.tr_elt<float32_t>(ts2_num + m, 0, k, j, mmax, nmax, false, false); \
      BODY32; \
      set_fp_exceptions; \
      break; \
    }\
    case e64: {\
      float64_t &td = P.MU.tr_elt<float64_t>(td_num + m, 0, i, j, mmax, nmax, reg_rename, true); \
      float64_t ts1 = P.MU.tr_elt<float64_t>(ts1_num + m, 0, i, k, mmax, nmax, false, false); \
      float64_t ts2 = P.MU.tr_elt<float64_t>(ts2_num + m, 0, k, j, mmax, nmax, false, false); \
      BODY64; \
      set_fp_exceptions; \
      break; \
    }\
    default: \
      require(0); \
      break; \
  }; \
  DEBUG_MVV_FP_VV; \
  MXU_VFP_LOOP_END \
  REGNAME_WRITE_BAKE(mmax, nmax, reg_sum) \


#define MXU_VFP_MM_ADD(BODY16, BODY32, BODY64) \
  require_fp; \
  require((P.MU.msew == e16 && p->extension_enabled(EXT_ZFH)) || \
          (P.MU.msew == e32 && p->extension_enabled('F')) || \
          (P.MU.msew == e64 && p->extension_enabled('D'))); \
  require_matrix(true);\
  require(STATE.frm->read() < 0x5);\
  reg_t tile_m = P.MU.tile_m->read();\
  reg_t tile_n = P.MU.tile_n->read();\
  reg_t sew = P.MU.msew; \
  reg_t td_num = insn.td(); \
  reg_t ts1_num = insn.ts1(); \
  reg_t ts2_num = insn.rs2(); \
  reg_t mmax = P.MU.mrows;\
  reg_t nmax = P.MU.mcols / P.MU.msew; \
  reg_t lmul = insn.mlmul() != LMUL_RESERVE ? (1 << insn.mlmul()) : P.MU.mlmul; \
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
        switch(P.MU.msew) { \
        case e16: { \
          float16_t &td = P.MU.tr_elt<float16_t>(td_num + m, 0, i, j, mmax, nmax, reg_rename, true); \
          float16_t ts1  = P.MU.tr_elt<float16_t>(ts1_num + m, 0, i, j, mmax, nmax, false, false); \
          float16_t ts2  = P.MU.tr_elt<float16_t>(ts2_num + m, 0, i, j, mmax, nmax, false, false); \
          BODY16; \
          set_fp_exceptions; \
          break; \
        }\
        case e32: {\
          float32_t &td = P.MU.tr_elt<float32_t>(td_num + m, 0, i, j, mmax, nmax, reg_rename, true); \
          float32_t ts1  = P.MU.tr_elt<float32_t>(ts1_num + m, 0, i, j, mmax, nmax, false, false); \
          float32_t ts2  = P.MU.tr_elt<float32_t>(ts2_num + m, 0, i, j, mmax, nmax, false, false); \
          BODY32; \
          set_fp_exceptions; \
          break; \
        }\
        case e64: {\
          float64_t &td = P.MU.tr_elt<float64_t>(td_num + m, 0, i, j, mmax, nmax, reg_rename, true); \
          float64_t ts1  = P.MU.tr_elt<float64_t>(ts1_num + m, 0, i, j, mmax, nmax, false, false); \
          float64_t ts2  = P.MU.tr_elt<float64_t>(ts2_num + m, 0, i, j, mmax, nmax, false, false); \
          BODY64; \
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
  REGNAME_WRITE_BAKE(mmax, nmax, reg_sum) \

#define MXU_VFP_W_MM_ADD(BODY16, BODY32, BODY64) \
  require_fp; \
  require((P.MU.msew == e16 && p->extension_enabled(EXT_ZFH)) || \
          (P.MU.msew == e32 && p->extension_enabled('F')) || \
          (P.MU.msew == e64 && p->extension_enabled('D'))); \
  require_matrix(true);\
  require(STATE.frm->read() < 0x5);\
  reg_t tile_m = P.MU.tile_m->read();\
  reg_t tile_n = P.MU.tile_n->read();\
  reg_t sew = P.MU.msew; \
  reg_t mmax = P.MU.mrows;\
  reg_t nmax = P.MU.mcols / P.MU.msew;\
  reg_t td_num = insn.td(); \
  reg_t ts1_num = insn.rs1(); \
  reg_t ts2_num = insn.rs2(); \
  reg_t lmul = insn.mlmul() != LMUL_RESERVE ? (1 << insn.mlmul()) : P.MU.mlmul; \
  require_align(insn.td(), lmul); \
  require_align(insn.ts1(), lmul); \
  require_align(insn.rs2(), lmul); \
  softfloat_roundingMode = STATE.frm->read(); \
  bool reg_rename = false; \
  if (td_num == ts1_num || td_num == ts2_num){ \
    reg_rename = true; \
  } \
  reg_t des_nmax = nmax / 2; \
  reg_t reg_sum = 1 + (tile_n - 1) / des_nmax; \
  reg_t td_num_lmul = 0; \
  for (reg_t i=0; i<tile_m; ++i) { \
    td_num_lmul = 0; \
    for (reg_t m = 0; m < lmul; m++) { \
      td_num_lmul += (m != 0) ? ((tile_n - 1) / des_nmax) : 0; \
      for (reg_t j=0; j<tile_n; ++j) { \
        switch(P.MU.msew) { \
        case e8: { \
          float16_t &td = P.MU.tr_elt<float16_t>(td_num + m + td_num_lmul + j / des_nmax, 0, i, j % des_nmax, mmax, des_nmax, reg_rename, true); \
          float16_t ts1  = i32_to_f16(P.MU.tr_elt<int8_t>(ts1_num + m, 0, i, j, mmax, nmax, false, false)); \
          float16_t ts2  = i32_to_f16(P.MU.tr_elt<int8_t>(ts2_num + m, 0, i, j, mmax, nmax, false, false)); \
          BODY16; \
          set_fp_exceptions; \
          break; \
        }\
        case e16: {\
          float32_t &td = P.MU.tr_elt<float32_t>(td_num + m + td_num_lmul + j / des_nmax, 0, i, j % des_nmax, mmax, des_nmax, reg_rename, true); \
          float32_t ts1  = f16_to_f32(P.MU.tr_elt<float16_t>(ts1_num + m, 0, i, j, mmax, nmax, false, false)); \
          float32_t ts2  = f16_to_f32(P.MU.tr_elt<float16_t>(ts2_num + m, 0, i, j, mmax, nmax, false, false)); \
          BODY32; \
          set_fp_exceptions; \
          break; \
        }\
        case e32: {\
          float64_t &td = P.MU.tr_elt<float64_t>(td_num + m + td_num_lmul + j / des_nmax, 0, i, j % des_nmax, mmax, des_nmax, reg_rename, true); \
          float64_t ts1  = f32_to_f64(P.MU.tr_elt<float32_t>(ts1_num + m, 0, i, j, mmax, nmax, false, false)); \
          float64_t ts2  = f32_to_f64(P.MU.tr_elt<float32_t>(ts2_num + m, 0, i, j, mmax, nmax, false, false)); \
          BODY64; \
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
  REGNAME_WRITE_BAKE(mmax, nmax, reg_sum) \

#define MXU_VFP_VV_LOOP_WIDE(BODY16, BODY32, wide) \
  MXU_VFP_LOOP_BASE(wide) \
  if (!only_one_fix_reg_sum){ \
      reg_sum += (tile_n - 1) / des_nmax; \
      only_one_fix_reg_sum = true; \
    } \
  switch(P.MU.msew) { \
    case e16: {\
      float32_t &td_w = P.MU.tr_elt<float32_t>(td_num + m + td_num_lmul + j / des_nmax, 0, i, j % des_nmax, mmax, des_nmax, reg_rename, true); \
      float32_t ts1 = f16_to_f32(P.MU.tr_elt<float16_t>(ts1_num + m, 0, i, k, mmax, nmax, false, false)); \
      float32_t ts2 = f16_to_f32(P.MU.tr_elt<float16_t>(ts2_num + m, 0, k, j, mmax, nmax, false, false)); \
      BODY16; \
      set_fp_exceptions; \
      break; \
    }\
    case e32: {\
      float64_t &td_w = P.MU.tr_elt<float64_t>(td_num + m + td_num_lmul + j / des_nmax, 0, i, j % des_nmax, mmax, des_nmax, reg_rename, true); \
      float64_t ts1 = f32_to_f64(P.MU.tr_elt<float32_t>(ts1_num + m, 0, i, k, mmax, nmax, false, false)); \
      float64_t ts2 = f32_to_f64(P.MU.tr_elt<float32_t>(ts2_num + m, 0, k, j, mmax, nmax, false, false)); \
      BODY32; \
      set_fp_exceptions; \
      break; \
    }\
    default: \
      require(0); \
      break; \
  }; \
  DEBUG_RVV_FP_VV; \
  MXU_VFP_LOOP_END \
  REGNAME_WRITE_BAKE(mmax, nmax, reg_sum) \

#define MXU_VFP_VV_LOOP_QUAD(BODY16, wide) \
  MXU_VFP_LOOP_BASE(wide) \
  switch(P.MU.msew) { \
    case e16: {\
      if (!only_one_fix_reg_sum){ \
        reg_sum += (tile_n - 1) / des_nmax; \
        only_one_fix_reg_sum = true; \
      } \
      float64_t &td_w = P.MU.tr_elt<float64_t>(td_num + m + td_num_lmul + j / des_nmax, 0, i, j % des_nmax, mmax, des_nmax, reg_rename, true); \
      float64_t ts1 = f16_to_f64(P.MU.tr_elt<float16_t>(ts1_num + m, 0, i, k, mmax, nmax, false, false)); \
      float64_t ts2 = f16_to_f64(P.MU.tr_elt<float16_t>(ts2_num + m, 0, k, j, mmax, nmax, false, false)); \
      BODY16; \
      set_fp_exceptions; \
      break; \
    }\
    default: \
      require(0); \
      break; \
  }; \
  DEBUG_RVV_FP_VV; \
  MXU_VFP_LOOP_END \
  REGNAME_WRITE_BAKE (mmax, nmax, reg_sum) \

#define VM_CHECK_STORE(elt_width, rows, cols)

#define VM_CHECK_LOAD(elt_width, rows, cols) \
  VM_CHECK_STORE(elt_width, rows, cols); \

// #define MTI_CHECK_LOAD(let_width, is_mask_ldst)

#define MTU_LS_LEN(trans, ch) \
  require_matrix(false) \
  switch (ch) \
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

#define CLEAR_TILE(td) \
for (reg_t m = 0; m < lmul; m++) {\
  for (reg_t i = 0; i < height; i++) { \
    for (reg_t j = 0; j < P.MU.mcols / 8; j++) { \
      P.MU.tr_elt<int8_t>(td + m, 0, i, j, P.MU.mrows, P.MU.mcols >> 3, false, true) = 0; \
    } \
  } \
}\

#define PAD_TILE(td, elt_width, val) \
for (reg_t m = 0; m < lmul; m++) {\
  for (reg_t i = 0; i < height; i++) { \
    for (reg_t j = 0; j < width; j++) { \
      P.MU.tr_elt<elt_width##_t>(td + m, 0, i, j, rmax, cmax, false, true) = (elt_width##_t)val; \
    } \
  } \
}\

#define WHOLE_MATRIX(is_true) \
  if (is_true) { \
    height = rmax ; \
    width = cmax ; \
  } \

#define MTU_TR_LD(is_trans, dim, elt_width, is_max) \
  const reg_t baseAddr = RS1; \
  const reg_t stride2 = RS2; \
  const reg_t td = insn.td(); \
  reg_t lmul = insn.mlmul() != LMUL_RESERVE ? (1 << insn.mlmul()) : P.MU.mlmul; \
  require_align(td, lmul); \
  reg_t height, width; \
  reg_t rmax = 0, cmax = 0;\
  MTU_LS_LEN(is_trans, dim); \
  WHOLE_MATRIX(is_max) \
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

#define MTU_TR_ST(is_trans, dim, elt_width, is_max) \
  const reg_t baseAddr = RS1; \
  const reg_t stride2 = RS2; \
  const reg_t td = insn.td(); \
  reg_t lmul = insn.mlmul() != LMUL_RESERVE ? (1 << insn.mlmul()) : P.MU.mlmul; \
  require_align(td, lmul); \
  reg_t height, width; \
  reg_t rmax = 0, cmax = 0; \
  MTU_LS_LEN(is_trans, dim); \
  WHOLE_MATRIX(is_max) \
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

#define MTU_VECTOR_LD(is_trans, dim, elt_width) \
  const reg_t baseAddr = RS1; \
  const reg_t stride2 = RS2; \
  const reg_t vd = insn.rd(); \
  reg_t height, width; \
  reg_t cmax = 0, rmax = 0; \
  MTU_LS_LEN(is_trans, dim); \
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
  MTU_LS_LEN(is_trans, dim); \
  for (reg_t i = 0; i < height; i++) { \
    for (reg_t j = 0; j < width; j++) { \
      elt_width##_t val = P.VU.elt<elt_width##_t>(vd, i * width + j); \
      MMU.store<elt_width##_t>( \
        baseAddr + i * stride2 + j * sizeof(elt_width##_t), val); \
    } \
  } \


#define MTU_UF_TR_LD(is_trans, elt_width, dim) \
  reg_t baseAddr = RS1; \
  const reg_t stride2 = RS2; \
  const reg_t td = insn.rd(); \
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
  MTU_LS_LEN(is_trans, dim); \
  CLEAR_TILE(td); \
  PAD_TILE(td, elt_width, mpadv); \
  for (reg_t i = 0; i < height; ++i) { \
    if (inposh >= 0 && (reg_t)inposh < inh && inposw >= 0 && (reg_t)inposw < inw) { \
      for (reg_t j = 0; j < width; ++j) { \
        elt_width##_t val = MMU.load<elt_width##_t>( \
                      baseAddr + j * sizeof(elt_width##_t)); \
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


#define MTU_UF_TR_SD(is_trans, elt_width, dim) \
  reg_t baseAddr = RS1; \
  const reg_t stride2 = RS2; \
  const reg_t td = insn.rd(); \
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
  MTU_LS_LEN(is_trans, dim); \
  for (reg_t i = 0; i < height; ++i) { \
    if (inposh >= 0 && (reg_t)inposh < inh && inposw >= 0 && (reg_t)inposw < inw) { \
      for (reg_t j = 0; j < width; ++j) { \
        auto val = P.MU.tr_elt<elt_width##_t>(td, is_trans, i, j, rmax, cmax, false, true); \
        MMU.store<elt_width##_t>( \
                      baseAddr + j * sizeof(elt_width##_t), val); \
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

#define MU_MFP_LOOP_SCALE_BASE(wide) \
  const reg_t ts1_num = insn.ts1(); \
  const reg_t td_num = insn.td(); \
  reg_t mmax = P.MU.mrows; \
  reg_t nmax = P.MU.mcols / P.MU.msew; \
  reg_t tile_m = P.MU.tile_m->read(); \
  reg_t tile_n = P.MU.tile_n->read(); \
  reg_t lmul = insn.mlmul() != LMUL_RESERVE ? (1 << insn.mlmul()) : P.MU.mlmul; \
  require_align(insn.td(), lmul); \
  require_align(insn.ts1(), lmul); \
  softfloat_roundingMode = STATE.frm->read(); \
  bool reg_rename = false; \
  if (td_num == ts1_num){ \
    reg_rename = true; \
  } \
  reg_t reg_sum = 1; \
  reg_t des_nmax = nmax / wide; \
  bool only_one_fix_reg_sum = false; \
  reg_t td_num_lmul = 0; \
  for (reg_t i = 0; i < tile_m; ++i) { \
    td_num_lmul = 0; \
    for (reg_t m = 0; m < lmul; m++) { \
      td_num_lmul += m!=0? (tile_n - 1) / des_nmax : 0; \
      for (reg_t j = 0; j < tile_n; ++j) { \


#define MU_MFP_LOOP_END \
    } \
  } \
  } \

#define MXU_MFP_CVT_SCALE(BODY8, BODY16, BODY32, BODY64,\
                         CHECK8, CHECK16, CHECK32, CHECK64,\
                         wide, eew_check) \
  require(eew_check); \
  MU_MFP_LOOP_SCALE_BASE(wide) \
  switch(P.MU.msew) { \
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
  REGNAME_WRITE_BAKE(mmax, nmax, reg_sum) \

#define MXU_MFP_CVT(BODY8, BODY16, BODY32, BODY64, wide) \
  MU_MFP_LOOP_SCALE_BASE(wide) \
  switch(P.MU.msew) { \
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
  REGNAME_WRITE_BAKE(mmax, nmax, reg_sum) \

#define MB_GENERAL_LOOP_BASE(for_num) \
  for (reg_t m = 0; m < lmul; m++) { \
    for (reg_t i = 0; i < for_num; ++i) { \

#define MB_GENERAL_LOOP_BASE_END \
   } \
  } \
// row when td == ts1 not copy , use break;
#define MB_BOARD_CORE(dir, parm_type) \
  switch(dir){ \
    case 'r' : \
      MB_GENERAL_LOOP_BASE(height) \
      if (!i && !m){ \
        if (td_num == ts1_num) \
          continue; \
      } \
      memcpy(tr_elt_td + (i * cmax) * sew / 8 + ( m * rmax * cmax), tr_elt_start, sew * width / 8); \
      MB_GENERAL_LOOP_BASE_END \
      break; \
    case 'c' : \
      MB_GENERAL_LOOP_BASE(height) \
      val  = P.MU.tr_elt<type_sew_t<parm_type>::type>(ts1_num + m, 0, i, 0, rmax, cmax, false, false); \
      temp.assign(width, val); \
      memcpy(tr_elt_td + (i * cmax) * sew / 8 + ( m * rmax * cmax), temp.data(), sew * width / 8); \
      MB_GENERAL_LOOP_BASE_END \
      break; \
    case 'f' : \
      MB_GENERAL_LOOP_BASE(height) \
      if (!i && !m){ \
        val  = P.MU.tr_elt<type_sew_t<parm_type>::type>(ts1_num, 0, 0, 0, rmax, cmax, false, false); \
        temp.assign(width, val); \
      } \
      memcpy(tr_elt_td + (i * cmax) * sew / 8 + ( m * rmax * cmax), temp.data(), width * sew / 8); \
      MB_GENERAL_LOOP_BASE_END \
      break; \
    default  :\
      break; \
  } \

#define MB_PARAM_INIT \
  if (ts1_num == td_num) { \
    tr_elt_td = tr_elt_start = P.MU.board_elt(ts1_num, rmax, cmax); \
  }else { \
    tr_elt_start = P.MU.board_elt(ts1_num, rmax, cmax); \
    tr_elt_td = P.MU.board_elt(td_num, rmax, cmax); \
  } \

#define MB_PARAM_BASE \
  require(P.MU.msew >= e8 && P.MU.msew <= e64); \
  reg_t tile_m = P.MU.tile_m->read();\
  reg_t tile_k = P.MU.tile_k->read();\
  reg_t tile_n = P.MU.tile_n->read();\
  reg_t rmax = 0; \
  reg_t cmax = 0; \
  reg_t height, width; \
  reg_t sew = P.MU.msew; \
  reg_t td_num = insn.rd(); \
  reg_t ts1_num = insn.rs1(); \
  char *tr_elt_start = NULL; \
  char *tr_elt_td = NULL; \
  reg_t lmul = P.MU.mlmul; \
  

#define MTR_BROADCAST(dim, dir) \
  MB_PARAM_BASE \
  MTU_LS_LEN(0, dim) \
  MB_PARAM_INIT \
  switch (sew) { \
    case e8: { \
      type_sew_t<8>::type val = 0; \
      std::vector<type_sew_t<8>::type> temp; \
      MB_BOARD_CORE(dir, 8) \
      break; \
    } \
    case e16: { \
      type_sew_t<16>::type val = 0; \
      std::vector<type_sew_t<16>::type> temp; \
      MB_BOARD_CORE(dir, 16) \
      break; \
    } \
    case e32: { \
      type_sew_t<32>::type val = 0; \
      std::vector<type_sew_t<32>::type> temp; \
      MB_BOARD_CORE(dir, 32) \
      break; \
    } \
    case e64: { \
      type_sew_t<64>::type val = 0; \
      std::vector<type_sew_t<64>::type> temp; \
      MB_BOARD_CORE(dir, 64) \
      break; \
    } \
    default : { \
      require(0); \
      break; \
    } \
  } 

#define MI_MM_LOOP(BODY ,PARAMS, ins) \
  MX_2D_LOOP_BASE(ins) \
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
  REGNAME_WRITE_BAKE(mmax, nmax, reg_sum) \

#define MI_MM_LOOP_WIDEN(BODY ,PARAMS, ins) \
  MX_2D_LOOP_BASE_WIDEN(ins) \
    MXU_LOOP_ELEMENT_SKIP(); \
    if (!only_one_fix_reg_sum){ \
      reg_sum += (tile_n - 1) / des_nmax; \
      only_one_fix_reg_sum = true; \
    } \
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
  MX_2D_LOOP_END \
  REGNAME_WRITE_BAKE(mmax, nmax, reg_sum) \

#define MXU_MM_LOOP_QUEN(BODY, PARAMS, wide) \
  MXU_GENERAL_LOOP_BASE_WIDE(false, wide) \
    MXU_LOOP_ELEMENT_SKIP(); \
    if (!only_one_fix_reg_sum){ \
      reg_sum += (tile_n - 1) / des_nmax; \
      only_one_fix_reg_sum = true; \
    } \
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
  REGNAME_WRITE_BAKE(mmax, nmax, reg_sum) \


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

#define MI_2D_VFP_COMMON(wide) \
  require_fp; \
  require((P.MU.msew == e16 && p->extension_enabled(EXT_ZFH)) || \
          (P.MU.msew == e32 && p->extension_enabled('F')) || \
          (P.MU.msew == e64 && p->extension_enabled('D'))); \
  require_matrix(true);\
  require(STATE.frm->read() < 0x5);\
  reg_t tile_m = P.MU.tile_m->read();\
  reg_t tile_n = P.MU.tile_n->read();\
  reg_t sew = P.MU.msew; \
  reg_t td_num = insn.td(); \
  reg_t ts1_num = insn.ts1(); \
  reg_t ts2_num = insn.rs2(); \
  reg_t mmax = P.MU.mrows; \
  reg_t nmax = P.MU.mcols / P.MU.msew; \
  reg_t lmul = insn.mlmul() != LMUL_RESERVE ? (1 << insn.mlmul()) : P.MU.mlmul; \
  require_align(insn.td(), lmul); \
  require_align(insn.ts1(), lmul); \
  require_align(insn.rs2(), lmul); \
  softfloat_roundingMode = STATE.frm->read(); \
  bool reg_rename = false; \
  if (td_num == ts1_num || td_num == ts2_num){ \
    reg_rename = true; \
  } \
  reg_t reg_sum = 1; \
  bool only_one_fix_reg_sum = false; \
  reg_t td_num_lmul = 0; \
  reg_t des_nmax = nmax / wide; 

#define MI_2D_VFP_LOOP_BASE(wide) \
  MI_2D_VFP_COMMON(wide) \
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

#define MI_VFP_MM_LOOP(BODY16, BODY32, BODY64,wide) \
  MI_2D_VFP_LOOP_BASE(wide) \
  switch (sew) { \
    case e16: { \
      MFU_MM_PARAMS(16); \
      BODY16; \
      set_fp_exceptions; \
      break; \
     } \
     case e32: { \
      MFU_MM_PARAMS(32); \
      BODY32; \
      set_fp_exceptions; \
      break; \
     } \
     case e64: { \
      MFU_MM_PARAMS(64); \
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
  REGNAME_WRITE_BAKE(mmax, nmax, reg_sum) \

#endif // _RISCV_M_EXT_MACROS_H