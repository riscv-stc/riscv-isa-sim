#ifndef _RISCV_M_EXT_MACROS_H
#define _RISCV_M_EXT_MACROS_H

#include "vector_unit.h"
#include "v_ext_macros.h"
#include <math.h>

#define LMUL_RESERVE 3
#define MX_PARAMS(x) \
  type_sew_t<x>::type &td  = P.MU.tr_elt<type_sew_t<x>::type>(td_num + m, 0, i, j, mmax, nmax, false); \
  type_sew_t<x>::type &ts1  = P.MU.tr_elt<type_sew_t<x>::type>(ts1_num + m, 0, i, k, mmax, nmax, false); \
  type_sew_t<x>::type &ts2  = P.MU.tr_elt<type_sew_t<x>::type>(ts2_num + m, 0, k, j, mmax, nmax, false); \


#define MXSU_PARAMS(x) \
  type_sew_t<x>::type &td  = P.MU.tr_elt<type_sew_t<x>::type>(td_num + m, 0, i, j, mmax, nmax, false); \
  type_sew_t<x>::type ts1  = P.MU.tr_elt<type_sew_t<x>::type>(ts1_num + m, 0, i, k, mmax, nmax, false); \
  type_usew_t<x>::type ts2  = P.MU.tr_elt<type_usew_t<x>::type>(ts2_num + m, 0, k, j, mmax, nmax, false); \

#define MX_TS1_PARAMS(x) \
  type_sew_t<x>::type ts1  = P.MU.tr_elt<type_sew_t<x>::type>(ts1_num + m, 0, i, k, mmax, nmax, false); \

#define MXU_TS1_PARAMS(x) \
  type_usew_t<x>::type ts1  = P.MU.tr_elt<type_usew_t<x>::type>(ts1_num + m, 0, i, k, mmax, nmax, false); \

#define MX_TS2_PARAMS(x) \
  type_sew_t<x>::type ts2  = P.MU.tr_elt<type_sew_t<x>::type>(ts2_num + m, 0, i, k, mmax, nmax, false); \

#define MXU_TS2_PARAMS(x) \
  type_usew_t<x>::type ts2  = P.MU.tr_elt<type_usew_t<x>::type>(ts2_num + m, 0, i, k, mmax, nmax, false); \

#define MXU_PARAMS(x) \
  type_usew_t<x>::type &td  = P.MU.tr_elt<type_usew_t<x>::type>(td_num + m, 0, i, j, mmax, nmax, false); \
  type_usew_t<x>::type &ts1  = P.MU.tr_elt<type_usew_t<x>::type>(ts1_num + m, 0, i, k, mmax, nmax, false); \
  type_usew_t<x>::type &ts2  = P.MU.tr_elt<type_usew_t<x>::type>(ts2_num + m, 0, k, j, mmax, nmax, false); \

#define MX_PARAM_BASE(x) \
  type_sew_t<x>::type ts1  = P.MU.tr_elt<type_sew_t<x>::type>(ts1_num + m, 0, i, k, mmax, nmax, false); \
  type_sew_t<x>::type ts2  = P.MU.tr_elt<type_sew_t<x>::type>(ts2_num + m, 0, k, j, mmax, nmax, false); \

#define MXU_PARAM_BASE(x) \
  type_usew_t<x>::type ts1  = P.MU.tr_elt<type_usew_t<x>::type>(ts1_num + m, 0, i, k, mmax, nmax, false); \
  type_usew_t<x>::type ts2  = P.MU.tr_elt<type_usew_t<x>::type>(ts2_num + m, 0, k, j, mmax, nmax, false); \

#define MMULXU_PARAMS(x, mult) \
  type_usew_t<x * mult>::type &td  = P.MU.tr_elt<type_usew_t<x * mult>::type>(td_num + m, 0, i, j, mmax, nmax, false); \
  MXU_PARAM_BASE(x) \

#define MMULX_PARAMS(x, mult) \
  type_sew_t<x * mult>::type &td  = P.MU.tr_elt<type_sew_t<x * mult>::type>(td_num + m, 0, i, j, mmax, nmax, false); \
  MX_PARAM_BASE(x) \

#define MMULXXU_PARAMS(x, mult) \
  type_sew_t<x * mult>::type &td  = P.MU.tr_elt<type_sew_t<x * mult>::type>(td_num + m, 0, i, j, mmax, nmax, false); \
  MX_TS1_PARAMS(x) \
  MXU_TS2_PARAMS(x) \

#define MXXU_PARAMS(x) \
  type_sew_t<x>::type &td  = P.MU.tr_elt<type_sew_t<x>::type>(td_num + m, 0, i, j, mmax, nmax, false); \
  type_sew_t<x>::type &ts1  = P.MU.tr_elt<type_sew_t<x>::type>(ts1_num + m, 0, i, k, mmax, nmax, false); \
  type_usew_t<x>::type &ts2  = P.MU.tr_elt<type_usew_t<x>::type>(ts2_num + m, 0, k, j, mmax, nmax, false); \

#define MXUX_PARAMS(x) \
  type_sew_t<x>::type &td  = P.MU.tr_elt<type_sew_t<x>::type>(td_num + m, 0, i, j, mmax, nmax, false); \
  type_usew_t<x>::type &ts1  = P.MU.tr_elt<type_usew_t<x>::type>(ts1_num + m, 0, i, k, mmax, nmax, false); \
  type_sew_t<x>::type &ts2  = P.MU.tr_elt<type_sew_t<x>::type>(ts2_num + m, 0, k, j, mmax, nmax, false); \

#define MXDSU_PARAMS(x) \
  type_usew_t<x>::type ts1  = P.MU.tr_elt<type_usew_t<x>::type>(ts1_num + m, 0, i, k, mmax, nmax, false); \
  type_usew_t<x>::type ts2  = P.MU.tr_elt<type_usew_t<x>::type>(ts2_num + m, 0, k, j, mmax, nmax, false); \

#define MXDS_PARAMS(x) \
  type_sew_t<x>::type &ts1  = P.MU.tr_elt<type_sew_t<x>::type>(ts1_num + m, 0, i, k, mmax, nmax, false); \
  type_sew_t<x>::type &ts2  = P.MU.tr_elt<type_sew_t<x>::type>(ts2_num + m, 0, k, j, mmax, nmax, false); \

#define MXX_PARAMS_TR1(x) \
  type_sew_t<x>::type &ts1  = P.MU.tr_elt<type_sew_t<x>::type>(ts1_num + m, 0, i, j, mmax, nmax, false); \

#define MFU_PARAMS_TR1(width) \
  float##width##_t UNUSED &ts1  = P.MU.tr_elt<float##width##_t>(ts1_num + m, 0, i, j, mmax, nmax, false); \

#define MFU_PARAMS_TR2(width) \
  float##width##_t UNUSED &ts2  = P.MU.tr_elt<float##width##_t>(ts2_num + m, 0, i, j, mmax, nmax, false); \

#define MFU_MM_PARAMS(width) \
  float##width##_t &td = P.MU.tr_elt<float##width##_t>(td_num + m, 0, i, j, mmax, nmax, false); \
  MFU_PARAMS_TR1(width) \
  MFU_PARAMS_TR2(width) \


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
    for (reg_t j = 0; j < width; j++) { \

#define MTU_VREG_TR_PARAMS(trans, x) \
  type_sew_t<x>::type &vd = P.VU.elt<type_sew_t<x>::type>(rd_num, i*width+j, true); \
  type_sew_t<x>::type ts1 = P.MU.tr_elt<type_sew_t<x>::type>(rs1_num, trans, i + start_height, j, rmax, cmax); \

#define MTU_TR_VREG_PARAMS(trans, x) \
  type_sew_t<x>::type vs1 = P.VU.elt<type_sew_t<x>::type>(rs1_num, i*width+j); \
  type_sew_t<x>::type &td = P.MU.tr_elt<type_sew_t<x>::type>(rd_num, trans, i + start_height, j, rmax, cmax, true); \


#define MTU_VM_LOOP_END \
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
  if (ins && insn.mlmul() != LMUL_RESERVE) \
    lmul = (1 << insn.mlmul()); \
  for (reg_t m = 0 ; m < lmul; m++) { \
    for (reg_t i = 0; i < tile_m; ++i) { \
      for (reg_t j = 0; j < tile_n; ++j) { \
        for (reg_t k = 0; k < tile_k; ++k) { \

#define MXU_LOOP_ELEMENT_SKIP(BODY)

#define MXU_LOOP_BASE \
    MXU_GENERAL_LOOP_BASE(false) \
    MXU_LOOP_ELEMENT_SKIP();

#define MXU_LOOP_END \
        } \
      } \
    } \
  } \

#define MXU_VFP_LOOP_END \
  MXU_LOOP_END


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
  MXU_LOOP_END

#define MXU_CHECK_OVERFLOW(eew) \
  res = res > (pow(2, eew - 1) - 1)? (pow(2, eew - 1) -1): res; \
  res = res < (-pow(2, eew - 1))? (-pow(2, eew - 1)) : res; \
    


#define MX_2D_GENERAL_LOOP_BASE \
  require(P.MU.msew >= e8 && P.MU.msew <= e64); \
  reg_t tile_m = P.MU.tile_m->read();\
  reg_t tile_n = P.MU.tile_n->read();\
  reg_t mmax = P.MU.mrows;\
  reg_t nmax = P.MU.mcols / P.MU.msew;\
  reg_t sew = P.MU.msew; \
  reg_t td_num = insn.rd(); \
  reg_t ts1_num = insn.rs1(); \
  reg_t ts2_num = insn.rs2(); \
  for (reg_t i = 0; i < tile_m; ++i) { \
    for (reg_t j = 0; j < tile_n; ++j) { \

#define MX_2D_LOOP_ELEMENT_SKIP(BODY)

#define MX_2D_LOOP_BASE \
    MX_2D_GENERAL_LOOP_BASE \
    MX_2D_LOOP_ELEMENT_SKIP();

#define MX_2D_LOOP_END \
      } \
    } \

#define MXU_MM_ADD(opd, op0, sign, type_t, td_type) \
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
  for (reg_t m = 0; m < lmul; m++) {\
    for (reg_t i = 0; i < tile_m; ++i) { \
      for (reg_t j = 0; j < tile_n; ++j) { \
        if (sew == e8){ \
          auto &td = P.MU.tr_elt<sign<e8>::type>(td_num + m , 0, i, j, mmax, nmax, true); \
          auto ts1  = P.MU.tr_elt<sign<e8>::type>(ts1_num + m, 0, i, j, mmax, nmax, false); \
          auto ts2  = P.MU.tr_elt<sign<e8>::type>(ts2_num + m, 0, i, j, mmax, nmax, false); \
          res = (type_t)ts1 op0 (type_t)ts2; \
          MXU_CHECK_OVERFLOW(e8) \
          td = (td_type##8_t)res; \
        }else if(sew == e16){ \
          auto &td = P.MU.tr_elt<sign<e16>::type>(td_num + m, 0, i, j, mmax, nmax, true); \
          auto ts1  = P.MU.tr_elt<sign<e16>::type>(ts1_num + m, 0, i, j, mmax, nmax, false); \
          auto ts2  = P.MU.tr_elt<sign<e16>::type>(ts2_num + m, 0, i, j, mmax, nmax, false); \
          res = (type_t)ts1 op0 (type_t)ts2; \
          MXU_CHECK_OVERFLOW(e16) \
          td = (td_type##16_t)res; \
        }else if(sew == e32){ \
          auto &td = P.MU.tr_elt<sign<e32>::type>(td_num + m, 0, i, j, mmax, nmax, true); \
          auto ts1  = P.MU.tr_elt<sign<e32>::type>(ts1_num + m, 0, i, j, mmax, nmax, false); \
          auto ts2  = P.MU.tr_elt<sign<e32>::type>(ts2_num + m, 0, i, j, mmax, nmax, false); \
          res = (type_t)ts1 op0 (type_t)ts2; \
          MXU_CHECK_OVERFLOW(e32) \
          td = (td_type##32_t)res; \
        }else if(sew == e64){ \
          auto &td = P.MU.tr_elt<sign<e64>::type>(td_num + m, 0, i, j, mmax, nmax, true); \
          auto ts1  = P.MU.tr_elt<sign<e64>::type>(ts1_num + m, 0, i, j, mmax, nmax, false); \
          auto ts2  = P.MU.tr_elt<sign<e64>::type>(ts2_num + m, 0, i, j, mmax, nmax, false); \
          res = (type_t)ts1 op0 (type_t)ts2; \
          MXU_CHECK_OVERFLOW(e64) \
          td = (td_type##64_t)res; \
        } \
      } \
    } \
  } \

#define MXU_W_MM_ADD(opd, op0, sign, type_t, td_type) \
  require(P.MU.msew >= e8 && P.MU.msew <= e32); \
  reg_t lmul = insn.mlmul() != LMUL_RESERVE ? (1 << insn.mlmul()) : P.MU.mlmul; \
  require_align(insn.td(), lmul); \
  require_align(insn.ts1(), lmul); \
  require_align(insn.rs2(), lmul); \
  reg_t tile_m = P.MU.tile_m->read() * lmul;\
  reg_t tile_n = P.MU.tile_n->read() * lmul;\
  reg_t sew = P.MU.msew; \
  reg_t td_num = insn.td(); \
  reg_t ts1_num = insn.ts1(); \
  reg_t ts2_num = insn.rs2(); \
  reg_t mmax = P.MU.mrows;\
  reg_t nmax = P.MU.mcols / P.MU.msew;\
  type_t res; \
  for (reg_t m = 0; m < lmul; m++) { \
    for (reg_t i = 0; i < tile_m; ++i) { \
      for (reg_t j = 0; j < tile_n; ++j) { \
        if (sew == e8){ \
          reg_t des_nmax = nmax/2; \
          auto &td = P.MU.tr_elt<sign<e16>::type>(td_num + m + j / des_nmax, 0, i, j % des_nmax, mmax, des_nmax, true); \
          auto ts1  = P.MU.tr_elt<sign<e8>::type>(ts1_num + m, 0, i, j, mmax, nmax, false); \
          auto ts2  = P.MU.tr_elt<sign<e8>::type>(ts2_num + m, 0, i, j, mmax, nmax, false); \
          res = (type_t)ts1 op0 (type_t)ts2; \
          MXU_CHECK_OVERFLOW(e16) \
          td = (td_type##16_t)res; \
        }else if(sew == e16){ \
          reg_t des_nmax = nmax/2; \
          auto &td = P.MU.tr_elt<sign<e32>::type>(td_num + m + j / des_nmax, 0, i, j % des_nmax, mmax, des_nmax, true); \
          auto ts1  = P.MU.tr_elt<sign<e16>::type>(ts1_num + m, 0, i, j, mmax, nmax, false); \
          auto ts2  = P.MU.tr_elt<sign<e16>::type>(ts2_num + m, 0, i, j, mmax, nmax, false); \
          res = (type_t)ts1 op0 (type_t)ts2; \
          MXU_CHECK_OVERFLOW(e32) \
          td = (td_type##32_t)res; \
        }else if(sew == e32){ \
          reg_t des_nmax = nmax/2; \
          auto &td = P.MU.tr_elt<sign<e64>::type>(td_num + m + j / des_nmax, 0, i, j % des_nmax, mmax, des_nmax, true); \
          auto ts1  = P.MU.tr_elt<sign<e32>::type>(ts1_num + m, 0, i, j, mmax, nmax, false); \
          auto ts2  = P.MU.tr_elt<sign<e32>::type>(ts2_num + m, 0, i, j, mmax, nmax, false); \
          res = (type_t)ts1 op0 (type_t)ts2; \
          MXU_CHECK_OVERFLOW(e64) \
          td = (td_type##64_t)res; \
        }\
      } \
    } \
  } \

#define MXU_Q_MM_ADD(opd, op0, sign, type, td_type) \
  require(P.MU.msew >= e8 && P.MU.msew <= e16); \
  reg_t tile_m = P.MU.tile_m->read();\
  reg_t tile_n = P.MU.tile_n->read();\
  reg_t sew = P.MU.msew; \
  reg_t td_num = insn.td(); \
  reg_t ts1_num = insn.ts1(); \
  reg_t ts2_num = insn.rs2(); \
  type res; \
  for (reg_t i = 0; i < tile_m; ++i) { \
    for (reg_t j = 0; j < tile_n; ++j) { \
      if (sew == e8){ \
        reg_t des_nmax = nmax / 4; \
        auto &td = P.MU.tr_elt<sign<e32>::type>(td_num + m + j / des_nmax, 0, i, j % des_nmax, mmax, des_nmax, true); \
        auto ts1  = P.MU.tr_elt<sign<e8>::type>(ts1_num, 0, i, j, mmax, nmax, false); \
        auto ts2  = P.MU.tr_elt<sign<e8>::type>(ts2_num, 0, i, j, mmax, nmax, false); \
        res = (type)ts1 op0 (type)ts2; \
        MXU_CHECK_OVERFLOW(e32) \
        td = (td_type##32_t)res; \
      }else if(sew == e16){ \
        reg_t des_nmax = nmax / 4; \
        auto &td = P.MU.tr_elt<sign<e64>::type>(td_num + m + j / des_nmax, 0, i, j % des_nmax, mmax, des_nmax, true); \
        auto ts1  = P.MU.tr_elt<sign<e16>::type>(ts1_num, 0, i, j, mmax, nmax, false); \
        auto ts2  = P.MU.tr_elt<sign<e16>::type>(ts2_num, 0, i, j, mmax, nmax, false); \
        res = (type)ts1 op0 (type)ts2; \
        MXU_CHECK_OVERFLOW(e64) \
        td = (td_type##64_t)res; \
      }\
    } \
  } \



#define MXU_WIDE_OP_AND_ASSIGN(var0, var1, var2, op0, op1, sign) \
  switch(P.MU.msew) { \
  case e8: { \
    reg_t des_nmax = nmax / 2; \
    sign##16_t td_w = P.MU.tr_elt<sign##16_t>(td_num + m + j / des_nmax, 0, i, j % des_nmax, mmax, des_nmax); \
    P.MU.tr_elt<uint16_t>(td_num + m + j / des_nmax, 0, i, j % des_nmax, mmax, des_nmax, true) = \
      op1((sign##16_t)(sign##8_t)var0 op0 (sign##16_t)(sign##8_t)var1) + var2; \
    } \
    break; \
  case e16: { \
    reg_t des_nmax = nmax / 2; \
    sign##32_t td_w = P.MU.tr_elt<sign##32_t>(td_num + m + j / des_nmax, 0, i, j % des_nmax, mmax, des_nmax); \
    P.MU.tr_elt<uint32_t>(td_num + m + j / des_nmax, 0, i, j % des_nmax, mmax, des_nmax, true) = \
      op1((sign##32_t)(sign##16_t)var0 op0 (sign##32_t)(sign##16_t)var1) + var2; \
    } \
    break; \
  default: { \
    reg_t des_nmax = nmax / 2; \
    sign##64_t td_w = P.MU.tr_elt<sign##64_t>(td_num + m + j / des_nmax, 0, i, j % des_nmax, mmax, des_nmax); \
    P.MU.tr_elt<uint64_t>(td_num + m + j / des_nmax, 0, i, j % des_nmax, mmax, des_nmax, true) = \
      op1((sign##64_t)(sign##32_t) var0 op0 (sign##64_t)(sign##32_t)var1) + var2; \
    } \
    break; \
  }

#define MXU_WIDE_OP_AND_ASSIGN_MIX(var0, var1, var2, op0, op1, signd, sign_1, sign_2) \
  switch(P.MU.msew) { \
  case e8: { \
    reg_t des_nmax = nmax / 2; \
    signd##16_t UNUSED td_w = P.MU.tr_elt<signd##16_t>(td_num + m + j / des_nmax, 0, i, j % des_nmax, mmax, des_nmax); \
    P.MU.tr_elt<uint16_t>(td_num + m + j / des_nmax, 0, i, j % des_nmax, mmax, des_nmax, true) = \
      op1((sign_1##16_t)(sign_1##8_t)var0 op0 (sign_2##16_t)(sign_2##8_t)var1) + var2; \
    } \
    break; \
  case e16: { \
    reg_t des_nmax = nmax / 2; \
    signd##32_t td_w = P.MU.tr_elt<signd##32_t>(td_num + m + j / des_nmax, 0, i, j % des_nmax, mmax, des_nmax); \
    P.MU.tr_elt<uint32_t>(td_num + m + j / des_nmax, 0, i, j % des_nmax, mmax, des_nmax, true) = \
      op1((sign_1##32_t)(sign_1##16_t)var0 op0 (sign_2##32_t)(sign_2##16_t)var1) + var2; \
    } \
    break; \
  default: { \
    reg_t des_nmax = nmax / 2; \
    signd##64_t td_w = P.MU.tr_elt<signd##64_t>(td_num + m + j / des_nmax, 0, i, j % des_nmax, mmax, des_nmax); \
    P.MU.tr_elt<uint64_t>(td_num + m + j / des_nmax, 0, i, j % des_nmax, mmax, des_nmax, true) = \
      op1((sign_1##64_t)(sign_1##32_t) var0 op0 (sign_2##64_t)(sign_2##32_t)var1) + var2; \
    } \
    break; \
  }

#define MXU_QUAD_OP_AND_ASSIGN(var0, var1, var2, op0, op1, sign) \
  switch(P.MU.msew) { \
  case e8: { \
    reg_t des_nmax = nmax/4; \
    sign##32_t td_q = P.MU.tr_elt<sign##32_t>(td_num + m + j/des_nmax, 0, i, j % des_nmax, mmax, des_nmax); \
    P.MU.tr_elt<uint32_t>(td_num + m + j/des_nmax, 0, i, j % des_nmax, mmax, des_nmax, true) = \
      op1((sign##32_t)(sign##8_t)var0 op0 (sign##32_t)(sign##8_t)var1) + var2; \
    } \
    break; \
  case e16: { \
    reg_t des_nmax = nmax/4; \
    sign##64_t td_q = P.MU.tr_elt<sign##64_t>(td_num + m + j/des_nmax, 0, i, j % des_nmax, mmax, des_nmax); \
    P.MU.tr_elt<uint64_t>(td_num + m + j/des_nmax, 0, i, j % des_nmax, mmax, des_nmax, true) = \
      op1((sign##64_t)(sign##16_t)var0 op0 (sign##64_t)(sign##16_t)var1) + var2; \
    } \
    break; \
  }


#define VM_WIDE_CHECK_COMMON \
  require_matrix(false);\

#define VM_CHECK_DSS(is_vs1) \
  VM_WIDE_CHECK_COMMON; \

// widen operation loop
#define MXU_VV_LOOP_WIDEN(BODY) \
  MXU_LOOP_BASE \
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

#define MXU_VFP_COMMON \
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
  

#define MXU_VFP_LOOP_BASE \
  MXU_VFP_COMMON \
  /*printf("m,k,n = %d, %d, %d\n", tile_m, tile_k, tile_n);*/ \
  for (reg_t m = 0; m < lmul; m++) { \
    for (reg_t i=0; i<tile_m; ++i) { \
      for (reg_t j=0; j<tile_n; ++j) { \
        for (reg_t k = 0; k < tile_k; ++k) { \

#define MXU_CLEAR \
  reg_t td_num = insn.rd(); \
  for (reg_t i = 0; i < P.MU.mrows; i++) { \
    for (reg_t j = 0; j < P.MU.mcols * 2 / 8; j++) { \
      P.MU.tr_elt<int8_t>(td_num, 0, i, j, mmax, nmax, true) = 0; \
    } \
  }

#define MXU_VFP_VV_LOOP(BODY16, BODY32, BODY64) \
  MXU_VFP_LOOP_BASE \
  switch(P.MU.msew) { \
    case e16: { \
      float16_t &td = P.MU.tr_elt<float16_t>(td_num + m, 0, i, j, mmax, nmax, true); \
      float16_t ts1 = P.MU.tr_elt<float16_t>(ts1_num + m, 0, i, k, mmax, nmax, false); \
      float16_t ts2 = P.MU.tr_elt<float16_t>(ts2_num + m, 0, k, j, mmax, nmax, false); \
      BODY16; \
      set_fp_exceptions; \
      break; \
    }\
    case e32: {\
      float32_t &td = P.MU.tr_elt<float32_t>(td_num + m, 0, i, j, mmax, nmax, true); \
      float32_t ts1 = P.MU.tr_elt<float32_t>(ts1_num + m, 0, i, k, mmax, nmax, false); \
      float32_t ts2 = P.MU.tr_elt<float32_t>(ts2_num + m, 0, k, j, mmax, nmax, false); \
      BODY32; \
      set_fp_exceptions; \
      break; \
    }\
    case e64: {\
      float64_t &td = P.MU.tr_elt<float64_t>(td_num + m, 0, i, j, mmax, nmax, true); \
      float64_t ts1 = P.MU.tr_elt<float64_t>(ts1_num + m, 0, i, k, mmax, nmax, false); \
      float64_t ts2 = P.MU.tr_elt<float64_t>(ts2_num + m, 0, k, j, mmax, nmax, false); \
      BODY64; \
      set_fp_exceptions; \
      break; \
    }\
    default: \
      require(0); \
      break; \
  }; \
  DEBUG_MVV_FP_VV; \
  MXU_VFP_LOOP_END


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
  for (reg_t m = 0 ; m < lmul; m++) { \
    for (reg_t i=0; i<tile_m; ++i) { \
      for (reg_t j=0; j<tile_n; ++j) { \
        switch(P.MU.msew) { \
        case e16: { \
          float16_t &td = P.MU.tr_elt<float16_t>(td_num + m, 0, i, j, mmax, nmax, true); \
          float16_t ts1  = P.MU.tr_elt<float16_t>(ts1_num + m, 0, i, j, mmax, nmax, false); \
          float16_t ts2  = P.MU.tr_elt<float16_t>(ts2_num + m, 0, i, j, mmax, nmax, false); \
          BODY16; \
          set_fp_exceptions; \
          break; \
        }\
        case e32: {\
          float32_t &td = P.MU.tr_elt<float32_t>(td_num + m, 0, i, j, mmax, nmax, true); \
          float32_t ts1  = P.MU.tr_elt<float32_t>(ts1_num + m, 0, i, j, mmax, nmax, false); \
          float32_t ts2  = P.MU.tr_elt<float32_t>(ts2_num + m, 0, i, j, mmax, nmax, false); \
          BODY32; \
          set_fp_exceptions; \
          break; \
        }\
        case e64: {\
          float64_t &td = P.MU.tr_elt<float64_t>(td_num + m, 0, i, j, mmax, nmax, true); \
          float64_t ts1  = P.MU.tr_elt<float64_t>(ts1_num + m, 0, i, j, mmax, nmax, false); \
          float64_t ts2  = P.MU.tr_elt<float64_t>(ts2_num + m, 0, i, j, mmax, nmax, false); \
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
  for (reg_t m = 0; m < lmul; m++) { \
    for (reg_t i=0; i<tile_m; ++i) { \
      for (reg_t j=0; j<tile_n; ++j) { \
        switch(P.MU.msew) { \
        case e8: { \
          reg_t des_nmax = nmax / 2; \
          float16_t &td = P.MU.tr_elt<float16_t>(td_num + m + j / des_nmax, 0, i, j % des_nmax, mmax, des_nmax, true); \
          float16_t ts1  = i32_to_f16(P.MU.tr_elt<int8_t>(ts1_num + m, 0, i, j, mmax, nmax, false)); \
          float16_t ts2  = i32_to_f16(P.MU.tr_elt<int8_t>(ts2_num + m, 0, i, j, mmax, nmax, false)); \
          BODY16; \
          set_fp_exceptions; \
          break; \
        }\
        case e16: {\
          reg_t des_nmax = nmax / 2; \
          float32_t &td = P.MU.tr_elt<float32_t>(td_num + m + j / des_nmax, 0, i, j % des_nmax, mmax, des_nmax, true); \
          float32_t ts1  = f16_to_f32(P.MU.tr_elt<float16_t>(ts1_num + m, 0, i, j, mmax, nmax, false)); \
          float32_t ts2  = f16_to_f32(P.MU.tr_elt<float16_t>(ts2_num + m, 0, i, j, mmax, nmax, false)); \
          BODY32; \
          set_fp_exceptions; \
          break; \
        }\
        case e32: {\
          reg_t des_nmax = nmax / 2; \
          float64_t &td = P.MU.tr_elt<float64_t>(td_num + m + j / des_nmax, 0, i, j % des_nmax, mmax, des_nmax, true); \
          float64_t ts1  = f32_to_f64(P.MU.tr_elt<float32_t>(ts1_num + m, 0, i, j, mmax, nmax, false)); \
          float64_t ts2  = f32_to_f64(P.MU.tr_elt<float32_t>(ts2_num + m, 0, i, j, mmax, nmax, false)); \
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

#define MXU_VFP_VV_LOOP_WIDE(BODY16, BODY32) \
  MXU_VFP_LOOP_BASE \
  switch(P.MU.msew) { \
    case e16: {\
      reg_t des_nmax = nmax / 2; \
      float32_t &td_w = P.MU.tr_elt<float32_t>(td_num + m + j / des_nmax, 0, i, j % des_nmax, mmax, des_nmax, true); \
      float32_t ts1 = f16_to_f32(P.MU.tr_elt<float16_t>(ts1_num + m, 0, i, k, mmax, nmax, false)); \
      float32_t ts2 = f16_to_f32(P.MU.tr_elt<float16_t>(ts2_num + m, 0, k, j, mmax, nmax, false)); \
      BODY16; \
      set_fp_exceptions; \
      break; \
    }\
    case e32: {\
      reg_t des_nmax = nmax / 2; \
      float64_t &td_w = P.MU.tr_elt<float64_t>(td_num + m + j / des_nmax, 0, i, j % des_nmax, mmax, des_nmax, true); \
      float64_t ts1 = f32_to_f64(P.MU.tr_elt<float32_t>(ts1_num + m, 0, i, k, mmax, nmax, false)); \
      float64_t ts2 = f32_to_f64(P.MU.tr_elt<float32_t>(ts2_num + m, 0, k, j, mmax, nmax, false)); \
      BODY32; \
      set_fp_exceptions; \
      break; \
    }\
    default: \
      require(0); \
      break; \
  }; \
  DEBUG_RVV_FP_VV; \
  MXU_VFP_LOOP_END

#define MXU_VFP_VV_LOOP_QUAD(BODY16) \
  MXU_VFP_LOOP_BASE \
  switch(P.MU.msew) { \
    case e16: {\
      float64_t &td_w = P.MU.tr_elt<float64_t>(td_num + m, 0, i, j, mmax, nmax, true); \
      float64_t ts1 = f16_to_f64(P.MU.tr_elt<float16_t>(ts1_num + m, 0, i, k, mmax, nmax, false)); \
      float64_t ts2 = f16_to_f64(P.MU.tr_elt<float16_t>(ts2_num + m, 0, k, j, mmax, nmax, false)); \
      BODY16; \
      set_fp_exceptions; \
      break; \
    }\
    default: \
      require(0); \
      break; \
  }; \
  DEBUG_RVV_FP_VV; \
  MXU_VFP_LOOP_END

#define MXU_MEMUL_MX(factor) \
  require(P.MU.msew >= e8 && P.MU.msew <= e64); \
  reg_t tile_m = P.MU.tile_m->read();\
  reg_t tile_n = P.MU.tile_n->read();\
  reg_t sew = P.MU.msew; \
  reg_t td_num = insn.rd(); \
  reg_t ts1_num = insn.rs1(); \
  int128_t res; \
  for (reg_t i = 0; i < tile_m; ++i) { \
    for (reg_t j = 0; j < tile_n; ++j) { \
      if (sew == e8){ \
        auto &td = P.MU.tr_elt<type_sew_t<e8>::type>(td_num, 0, i, j, mmax, nmax, true); \
        auto ts1  = P.MU.tr_elt<type_sew_t<e8>::type>(ts1_num, 0, i, j, mmax, nmax, false); \
        res = (int128_t)ts1 * (int128_t)factor; \
        MXU_CHECK_OVERFLOW(e8) \
        td = (int8_t)res; \
      }else if(sew == e16){ \
        auto &td = P.MU.tr_elt<type_sew_t<e16>::type>(td_num, 0, i, j, mmax, nmax, true); \
        auto ts1  = P.MU.tr_elt<type_sew_t<e16>::type>(ts1_num, 0, i, j, mmax, nmax, false); \
        res = (int128_t)ts1 * (int128_t)factor; \
        MXU_CHECK_OVERFLOW(e16) \
        td = (int16_t)res; \
      }else if(sew == e32){ \
        auto &td = P.MU.tr_elt<type_sew_t<e32>::type>(td_num, 0, i, j, mmax, nmax, true); \
        auto ts1  = P.MU.tr_elt<type_sew_t<e32>::type>(ts1_num, 0, i, j, mmax, nmax, false); \
        res = (int128_t)ts1 * (int128_t)factor; \
        MXU_CHECK_OVERFLOW(e32) \
        td = (int32_t)res; \
      }else if(sew == e64){ \
        auto &td = P.MU.tr_elt<type_sew_t<e64>::type>(td_num, 0, i, j, mmax, nmax, true); \
        auto ts1  = P.MU.tr_elt<type_sew_t<e64>::type>(ts1_num, 0, i, j, mmax, nmax, false); \
        res = (int128_t)td * (int128_t)factor; \
        MXU_CHECK_OVERFLOW(e64) \
        td = (int64_t)res; \
      } \
    } \
  } \

#define MXU_MEMUL_MX_WIDEN(factor) \
  require(P.MU.msew >= e8 && P.MU.msew <= e32); \
  reg_t tile_m = P.MU.tile_m->read();\
  reg_t tile_n = P.MU.tile_n->read();\
  reg_t sew = P.MU.msew; \
  reg_t td_num = insn.rd(); \
  reg_t ts1_num = insn.rs1(); \
  int128_t res; \
  for (reg_t i = 0; i < tile_m; ++i) { \
    for (reg_t j = 0; j < tile_n; ++j) { \
      if (sew == e8){ \
        auto &td = P.MU.tr_elt<type_sew_t<e16>::type>(td_num, 0, i, j, mmax, nmax, true); \
        auto ts1  = P.MU.tr_elt<type_sew_t<e16>::type>(ts1_num, 0, i, j, mmax, nmax, false); \
        res = (int128_t)ts1 * (int128_t)factor; \
        MXU_CHECK_OVERFLOW(e16) \
        td = (int16_t)res; \
      }else if(sew == e16){ \
        auto &td = P.MU.tr_elt<type_sew_t<e32>::type>(td_num, 0, i, j, mmax, nmax, true); \
        auto ts1  = P.MU.tr_elt<type_sew_t<e32>::type>(ts1_num, 0, i, j, mmax, nmax, false); \
        res = (int128_t)ts1 * (int128_t)factor; \
        MXU_CHECK_OVERFLOW(e32) \
        td = (int32_t)res; \
      }else if(sew == e32){ \
        auto &td = P.MU.tr_elt<type_sew_t<e64>::type>(td_num, 0, i, j, mmax, nmax, true); \
        auto ts1  = P.MU.tr_elt<type_sew_t<e64>::type>(ts1_num, 0, i, j, mmax, nmax, false); \
        res = (int128_t)ts1 * (int128_t)factor; \
        MXU_CHECK_OVERFLOW(e64) \
        td = (int64_t)res; \
      } \
    } \
  } \

#define MXU_MEMUL_MX_QUAD(factor) \
  require(P.MU.msew >= e8 && P.MU.msew <= e16); \
  reg_t tile_m = P.MU.tile_m->read();\
  reg_t tile_n = P.MU.tile_n->read();\
  reg_t sew = P.MU.msew; \
  reg_t td_num = insn.rd(); \
  reg_t ts1_num = insn.rs1(); \
  int128_t res; \
  for (reg_t i = 0; i < tile_m; ++i) { \
    for (reg_t j = 0; j < tile_n; ++j) { \
      if (sew == e8){ \
        auto &td = P.MU.tr_elt<type_sew_t<e32>::type>(td_num, 0, i, j, mmax, nmax, true); \
        auto ts1  = P.MU.tr_elt<type_sew_t<e32>::type>(ts1_num, 0, i, j, mmax, nmax, false); \
        res = (int128_t)ts1 * (int128_t)factor; \
        MXU_CHECK_OVERFLOW(e32) \
        td = (int32_t)res; \
      }else if(sew == e16){ \
        auto &td = P.MU.tr_elt<type_sew_t<e64>::type>(td_num, 0, i, j, mmax, nmax, true); \
        auto ts1  = P.MU.tr_elt<type_sew_t<e64>::type>(ts1_num, 0, i, j, mmax, nmax, false); \
        res = (int128_t)ts1 * (int128_t)factor; \
        MXU_CHECK_OVERFLOW(e64) \
        td = (int64_t)res; \
      } \
    } \
  } \


#define MXU_MEMUL_MFP(BODY16, BODY32, BODY64) \
  require(P.MU.msew >= e16 && P.MU.msew <= e64); \
  reg_t tile_m = P.MU.tile_m->read();\
  reg_t tile_n = P.MU.tile_n->read();\
  reg_t sew = P.MU.msew; \
  reg_t td_num = insn.rd(); \
  reg_t ts1_num = insn.rs1(); \
  reg_t rs2_num = insn.rs2(); \
  int128_t res; \
  for (reg_t i = 0; i < tile_m; ++i) { \
    for (reg_t j = 0; j < tile_n; ++j) { \
      if(sew == e16){ \
        auto &td = P.MU.tr_elt<float16_t>(td_num, 0, i, j, mmax, nmax, true); \
        auto ts1  = P.MU.tr_elt<float16_t>(ts1_num, 0, i, j, mmax, nmax, false); \
        float16_t rs2 = f16(READ_FREG(rs2_num)); \
        BODY16; \
      }else if(sew == e32){ \
        auto &td = P.MU.tr_elt<float32_t>(td_num, 0, i, j, mmax, nmax, true); \
        auto ts1  = P.MU.tr_elt<float32_t>(ts1_num, 0, i, j, mmax, nmax, false); \
        float32_t rs2 = f32(READ_FREG(rs2_num)); \
        BODY32; \
      }else if(sew == e64){ \
        auto &td = P.MU.tr_elt<float64_t>(td_num, 0, i, j, mmax, nmax, true); \
        auto ts1  = P.MU.tr_elt<float64_t>(ts1_num, 0, i, j, mmax, nmax, false); \
        float64_t rs2 = f64(READ_FREG(rs2_num)); \
        BODY64; \
      } \
    } \
  } \

#define MXU_MEMUL_MFP_WIDEN(BODY32, BODY64) \
  require(P.MU.msew >= e16 && P.MU.msew <= e32); \
  reg_t tile_m = P.MU.tile_m->read();\
  reg_t tile_n = P.MU.tile_n->read();\
  reg_t sew = P.MU.msew; \
  reg_t td_num = insn.rd(); \
  reg_t ts1_num = insn.rs1(); \
  reg_t rs2_num = insn.rs2(); \
  int128_t res; \
  for (reg_t i = 0; i < tile_m; ++i) { \
    for (reg_t j = 0; j < tile_n; ++j) { \
      if(sew == e16){ \
        auto &td = P.MU.tr_elt<float32_t>(td_num, 0, i, j, mmax, nmax, true); \
        auto ts1  = P.MU.tr_elt<float32_t>(ts1_num, 0, i, j, mmax, nmax, false); \
        float32_t rs2 = f32(READ_FREG(rs2_num)); \
        BODY32; \
      }else if(sew == e32){ \
        auto &td = P.MU.tr_elt<float64_t>(td_num, 0, i, j, mmax, nmax, true); \
        auto ts1  = P.MU.tr_elt<float64_t>(ts1_num, 0, i, j, mmax, nmax, false); \
        float64_t rs2 = f64(READ_FREG(rs2_num)); \
        BODY64; \
      } \
    } \
  } \

#define GET_VS1(sew, is_trans, i, j) \
  if (!is_trans)  { \
    vs1 = P.VU.elt<type_sew_t<sew>::type>(vs1_num, j); \
  } else { \
    vs1 = P.VU.elt<type_sew_t<sew>::type>(vs1_num, i); \
  } \

#define GET_VS2(sew, is_trans, i, j) \
  if (!is_trans)  { \
    vs2 = P.VU.elt<type_sew_t<sew>::type>(vs2_num, j); \
  } else { \
    vs2 = P.VU.elt<type_sew_t<sew>::type>(vs2_num, i); \
  } \

#define MXU_MEMUL_MV(is_trans, BODY) \
  require(P.MU.msew >= e8 && P.MU.msew <= e64); \
  reg_t height = P.MU.tile_m->read();\
  reg_t width = P.MU.tile_n->read();\
  reg_t sew = P.MU.msew; \
  reg_t td_num = insn.rd(); \
  reg_t ts1_num = insn.rs1(); \
  reg_t vs2_num = insn.rs2(); \
  int128_t res, vs2; \
  for (reg_t i = 0; i < height; ++i) { \
    for (reg_t j = 0; j < width; ++j) { \
      if (sew == e8){ \
        GET_VS2(e8, is_trans, i, j) \
        auto &td = P.MU.tr_elt<type_sew_t<e8>::type>(td_num, 0, i, j, mmax, nmax, true); \
        auto ts1  = P.MU.tr_elt<type_sew_t<e8>::type>(ts1_num, 0, i, j, mmax, nmax, false); \
        BODY \
        MXU_CHECK_OVERFLOW(e8) \
        td = (int8_t)res; \
      }else if(sew == e16){ \
        GET_VS2(e16, is_trans, i, j) \
        auto &td = P.MU.tr_elt<type_sew_t<e16>::type>(td_num, 0, i, j, mmax, nmax, true); \
        auto ts1  = P.MU.tr_elt<type_sew_t<e16>::type>(ts1_num, 0, i, j, mmax, nmax, false); \
        BODY \
        MXU_CHECK_OVERFLOW(e16) \
        td = (int16_t)res; \
      }else if(sew == e32){ \
        GET_VS2(e32, is_trans, i, j) \
        auto &td = P.MU.tr_elt<type_sew_t<e32>::type>(td_num, 0, i, j, mmax, nmax, true); \
        auto ts1  = P.MU.tr_elt<type_sew_t<e32>::type>(ts1_num, 0, i, j, mmax, nmax, false); \
        BODY \
        MXU_CHECK_OVERFLOW(e32) \
        td = (int32_t)res; \
      }else if(sew == e64){ \
        GET_VS2(e64, is_trans, i, j) \
        auto &td = P.MU.tr_elt<type_sew_t<e64>::type>(td_num, 0, i, j, mmax, nmax, true); \
        auto ts1  = P.MU.tr_elt<type_sew_t<e64>::type>(ts1_num, 0, i, j, mmax, nmax, false); \
        BODY \
        MXU_CHECK_OVERFLOW(e64) \
        td = (int64_t)res; \
      } \
    } \
  } \


#define MXU_MEMUL_MV_WIDEN(is_trans, BODY) \
  require(P.MU.msew >= e8 && P.MU.msew <= e32); \
  reg_t height = P.MU.tile_m->read();\
  reg_t width = P.MU.tile_n->read();\
  reg_t sew = P.MU.msew; \
  reg_t td_num = insn.rd(); \
  reg_t ts1_num = insn.rs1(); \
  reg_t vs2_num = insn.rs2(); \
  int128_t res, vs2; \
  for (reg_t i = 0; i < height; ++i) { \
    for (reg_t j = 0; j < width; ++j) { \
      if (sew == e8){ \
        GET_VS2(e8, is_trans, i, j) \
        auto &td = P.MU.tr_elt<type_sew_t<e16>::type>(td_num, 0, i, j, mmax, nmax, true); \
        auto ts1  = P.MU.tr_elt<type_sew_t<e16>::type>(ts1_num, 0, i, j, mmax, nmax, false); \
        BODY \
        MXU_CHECK_OVERFLOW(e16) \
        td = (int16_t)res; \
      }else if(sew == e16){ \
        GET_VS2(e16, is_trans, i, j) \
        auto &td = P.MU.tr_elt<type_sew_t<e32>::type>(td_num, 0, i, j, mmax, nmax, true); \
        auto ts1  = P.MU.tr_elt<type_sew_t<e32>::type>(ts1_num, 0, i, j, mmax, nmax, false); \
        BODY \
        MXU_CHECK_OVERFLOW(e32) \
        td = (int32_t)res; \
      }else if(sew == e32){ \
        GET_VS2(e32, is_trans, i, j) \
        auto &td = P.MU.tr_elt<type_sew_t<e64>::type>(td_num, 0, i, j, mmax, nmax, true); \
        auto ts1  = P.MU.tr_elt<type_sew_t<e64>::type>(ts1_num, 0, i, j, mmax, nmax, false); \
        BODY \
        MXU_CHECK_OVERFLOW(e64) \
        td = (int64_t)res; \
      } \
    } \
  } \

#define MXU_MEMUL_MV_QUAD(is_trans, BODY) \
  require(P.MU.msew >= e8 && P.MU.msew <= e16); \
  reg_t height = P.MU.tile_m->read();\
  reg_t width = P.MU.tile_n->read();\
  reg_t sew = P.MU.msew; \
  reg_t td_num = insn.rd(); \
  reg_t ts1_num = insn.rs1(); \
  reg_t vs2_num = insn.rs2(); \
  int128_t res, vs2; \
  for (reg_t i = 0; i < height; ++i) { \
    for (reg_t j = 0; j < width; ++j) { \
      if (sew == e8){ \
        GET_VS2(e8, is_trans, i, j) \
        auto &td = P.MU.tr_elt<type_sew_t<e32>::type>(td_num, 0, i, j, mmax, nmax, true); \
        auto ts1  = P.MU.tr_elt<type_sew_t<e32>::type>(ts1_num, 0, i, j, mmax, nmax, false); \
        BODY \
        MXU_CHECK_OVERFLOW(e32) \
        td = (int32_t)res; \
      }else if(sew == e16){ \
        GET_VS2(e16, is_trans, i, j) \
        auto &td = P.MU.tr_elt<type_sew_t<e64>::type>(td_num, 0, i, j, mmax, nmax, true); \
        auto ts1  = P.MU.tr_elt<type_sew_t<e64>::type>(ts1_num, 0, i, j, mmax, nmax, false); \
        BODY \
        MXU_CHECK_OVERFLOW(e64) \
        td = (int64_t)res; \
      } \
    } \
  } \

#define MXU_MEMUL_MVFP(is_trans, BODY16, BODY32, BODY64) \
  require(P.MU.msew >= e16 && P.MU.msew <= e64); \
  reg_t tile_m = P.MU.tile_m->read();\
  reg_t tile_n = P.MU.tile_n->read();\
  reg_t sew = P.MU.msew; \
  reg_t td_num = insn.rd(); \
  reg_t ts1_num = insn.rs1(); \
  reg_t vs2_num = insn.rs2(); \
  int128_t res; \
  for (reg_t i = 0; i < tile_m; ++i) { \
    for (reg_t j = 0; j < tile_n; ++j) { \
      if(sew == e16){ \
        auto &td = P.MU.tr_elt<float16_t>(td_num, 0, i, j, mmax, nmax, true); \
        auto ts1  = P.MU.tr_elt<float16_t>(ts1_num, 0, i, j, mmax, nmax, false); \
        float16_t rs2; \
        if (!is_trans) { \
          rs2 = P.VU.elt<float16_t>(vs2_num, j); \
        } else { \
          rs2 = P.VU.elt<float16_t>(vs2_num, i); \
        }\
        BODY16; \
      }else if(sew == e32){ \
        auto &td = P.MU.tr_elt<float32_t>(td_num, 0, i, j, mmax, nmax, true); \
        auto ts1  = P.MU.tr_elt<float32_t>(ts1_num, 0, i, j, mmax, nmax, false); \
        float32_t rs2; \
        if (!is_trans) { \
          rs2 = P.VU.elt<float32_t>(vs2_num, j); \
        } else { \
          rs2 = P.VU.elt<float32_t>(vs2_num, i); \
        }\
        BODY32; \
      }else if(sew == e64){ \
        auto &td = P.MU.tr_elt<float64_t>(td_num, 0, i, j, mmax, nmax, true); \
        auto ts1  = P.MU.tr_elt<float64_t>(ts1_num, 0, i, j, mmax, nmax, false); \
        float64_t rs2; \
        if (!is_trans) { \
          rs2 = P.VU.elt<float64_t>(vs2_num, j); \
        } else { \
          rs2 = P.VU.elt<float64_t>(vs2_num, i); \
        }\
        BODY64; \
      } \
    } \
  } \

#define MXU_MEMUL_MVFP_WIDEN(is_trans, BODY32, BODY64) \
  require(P.MU.msew >= e16 && P.MU.msew <= e32); \
  reg_t tile_m = P.MU.tile_m->read();\
  reg_t tile_n = P.MU.tile_n->read();\
  reg_t sew = P.MU.msew; \
  reg_t td_num = insn.rd(); \
  reg_t ts1_num = insn.rs1(); \
  reg_t vs2_num = insn.rs2(); \
  int128_t res; \
  for (reg_t i = 0; i < tile_m; ++i) { \
    for (reg_t j = 0; j < tile_n; ++j) { \
      if(sew == e16){ \
        auto &td = P.MU.tr_elt<float32_t>(td_num, 0, i, j, mmax, nmax, true); \
        auto ts1  = P.MU.tr_elt<float32_t>(ts1_num, 0, i, j, mmax, nmax, false); \
        float32_t rs2; \
        if (!is_trans) { \
          rs2 = f16_to_f32(P.VU.elt<float16_t>(vs2_num, j)); \
        } else { \
          rs2 = f16_to_f32(P.VU.elt<float16_t>(vs2_num, i)); \
        }\
        BODY32; \
      }else if(sew == e32){ \
        auto &td = P.MU.tr_elt<float64_t>(td_num, 0, i, j, mmax, nmax, true); \
        auto ts1  = P.MU.tr_elt<float64_t>(ts1_num, 0, i, j, mmax, nmax, false); \
        float64_t rs2; \
        if (!is_trans) { \
          rs2 = f32_to_f64(P.VU.elt<float32_t>(vs2_num, j)); \
        } else { \
          rs2 = f32_to_f64(P.VU.elt<float32_t>(vs2_num, i)); \
        }\
        BODY64; \
      } \
    } \
  } \

#define MXU_MMACC_MV(is_trans, BODY) \
  require(P.MU.msew >= e8 && P.MU.msew <= e64); \
  reg_t height = P.MU.tile_m->read();\
  reg_t width = P.MU.tile_n->read();\
  reg_t sew = P.MU.msew; \
  reg_t td_num = insn.rd(); \
  reg_t vs1_num = insn.rs1(); \
  reg_t vs2_num = insn.rs2(); \
  int128_t res, vs1, vs2; \
  for (reg_t i = 0; i < height; ++i) { \
    for (reg_t j = 0; j < width; ++j) { \
      if (sew == e8){ \
        GET_VS1(e8, is_trans, i, j) \
        GET_VS2(e8, is_trans, i, j) \
        auto &td = P.MU.tr_elt<type_sew_t<e8>::type>(td_num, 0, i, j, mmax, nmax, true); \
        BODY \
        MXU_CHECK_OVERFLOW(e8) \
        td = (int8_t)res; \
      }else if(sew == e16){ \
        GET_VS1(e16, is_trans, i, j) \
        GET_VS2(e16, is_trans, i, j) \
        auto &td = P.MU.tr_elt<type_sew_t<e16>::type>(td_num, 0, i, j, mmax, nmax, true); \
        BODY \
        MXU_CHECK_OVERFLOW(e16) \
        td = (int16_t)res; \
      }else if(sew == e32){ \
        GET_VS1(e32, is_trans, i, j) \
        GET_VS2(e32, is_trans, i, j) \
        auto &td = P.MU.tr_elt<type_sew_t<e32>::type>(td_num, 0, i, j, mmax, nmax, true); \
        BODY \
        MXU_CHECK_OVERFLOW(e32) \
        td = (int32_t)res; \
      }else if(sew == e64){ \
        GET_VS1(e64, is_trans, i, j) \
        GET_VS2(e64, is_trans, i, j) \
        auto &td = P.MU.tr_elt<type_sew_t<e64>::type>(td_num, 0, i, j, mmax, nmax, true); \
        BODY \
        MXU_CHECK_OVERFLOW(e64) \
        td = (int64_t)res; \
      } \
    } \
  } \


#define MXU_MMACC_MV_WIDEN(is_trans, BODY) \
  require(P.MU.msew >= e8 && P.MU.msew <= e32); \
  reg_t height = P.MU.tile_m->read();\
  reg_t width = P.MU.tile_n->read();\
  reg_t sew = P.MU.msew; \
  reg_t td_num = insn.rd(); \
  reg_t vs1_num = insn.rs1(); \
  reg_t vs2_num = insn.rs2(); \
  int128_t res, vs1, vs2; \
  for (reg_t i = 0; i < height; ++i) { \
    for (reg_t j = 0; j < width; ++j) { \
      if (sew == e8){ \
        GET_VS1(e8, is_trans, i, j) \
        GET_VS2(e8, is_trans, i, j) \
        auto &td = P.MU.tr_elt<type_sew_t<e16>::type>(td_num, 0, i, j, mmax, nmax, true); \
        BODY \
        MXU_CHECK_OVERFLOW(e16) \
        td = (int16_t)res; \
      }else if(sew == e16){ \
        GET_VS1(e16, is_trans, i, j) \
        GET_VS2(e16, is_trans, i, j) \
        auto &td = P.MU.tr_elt<type_sew_t<e32>::type>(td_num, 0, i, j, mmax, nmax, true); \
        BODY \
        MXU_CHECK_OVERFLOW(e32) \
        td = (int32_t)res; \
      }else if(sew == e32){ \
        GET_VS1(e32, is_trans, i, j) \
        GET_VS2(e32, is_trans, i, j) \
        auto &td = P.MU.tr_elt<type_sew_t<e64>::type>(td_num, 0, i, j, mmax, nmax, true); \
        BODY \
        MXU_CHECK_OVERFLOW(e64) \
        td = (int64_t)res; \
      } \
    } \
  } \


#define MXU_MMACC_MV_QUAD(is_trans, BODY) \
  require(P.MU.msew >= e8 && P.MU.msew <= e16); \
  reg_t height = P.MU.tile_m->read();\
  reg_t width = P.MU.tile_n->read();\
  reg_t sew = P.MU.msew; \
  reg_t td_num = insn.rd(); \
  reg_t vs1_num = insn.rs1(); \
  reg_t vs2_num = insn.rs2(); \
  int128_t res, vs1, vs2; \
  for (reg_t i = 0; i < height; ++i) { \
    for (reg_t j = 0; j < width; ++j) { \
      if (sew == e8){ \
        GET_VS1(e8, is_trans, i, j) \
        GET_VS2(e8, is_trans, i, j) \
        auto &td = P.MU.tr_elt<type_sew_t<e32>::type>(td_num, 0, i, j, mmax, nmax, true); \
        BODY \
        MXU_CHECK_OVERFLOW(e32) \
        td = (int32_t)res; \
      }else if(sew == e16){ \
        GET_VS1(e16, is_trans, i, j) \
        GET_VS2(e16, is_trans, i, j) \
        auto &td = P.MU.tr_elt<type_sew_t<e64>::type>(td_num, 0, i, j, mmax, nmax, true); \
        BODY \
        MXU_CHECK_OVERFLOW(e64) \
        td = (int64_t)res; \
      } \
    } \
  } \

#define MXU_MMACC_MVFP(is_trans, BODY16, BODY32, BODY64) \
  require(P.MU.msew >= e16 && P.MU.msew <= e64); \
  reg_t tile_m = P.MU.tile_m->read();\
  reg_t tile_n = P.MU.tile_n->read();\
  reg_t sew = P.MU.msew; \
  reg_t td_num = insn.rd(); \
  reg_t vs1_num = insn.rs1(); \
  reg_t vs2_num = insn.rs2(); \
  int128_t res; \
  for (reg_t i = 0; i < tile_m; ++i) { \
    for (reg_t j = 0; j < tile_n; ++j) { \
      if(sew == e16){ \
        auto &td = P.MU.tr_elt<float16_t>(td_num, 0, i, j, mmax, nmax, true); \
        float16_t vs1, vs2; \
        if (!is_trans) { \
          vs1 = P.VU.elt<float16_t>(vs1_num, j); \
          vs2 = P.VU.elt<float16_t>(vs2_num, j); \
        } else { \
          vs1 = P.VU.elt<float16_t>(vs1_num, i); \
          vs2 = P.VU.elt<float16_t>(vs2_num, i); \
        }\
        BODY16; \
      }else if(sew == e32){ \
        auto &td = P.MU.tr_elt<float32_t>(td_num, 0, i, j, mmax, nmax, true); \
        float32_t vs1, vs2; \
        if (!is_trans) { \
          vs1 = P.VU.elt<float32_t>(vs1_num, j); \
          vs2 = P.VU.elt<float32_t>(vs2_num, j); \
        } else { \
          vs1 = P.VU.elt<float32_t>(vs1_num, i); \
          vs2 = P.VU.elt<float32_t>(vs2_num, i); \
        }\
        BODY32; \
      }else if(sew == e64){ \
        auto &td = P.MU.tr_elt<float64_t>(td_num, 0, i, j, mmax, nmax, true); \
        float64_t vs1, vs2; \
        if (!is_trans) { \
          vs1 = P.VU.elt<float64_t>(vs1_num, j); \
          vs2 = P.VU.elt<float64_t>(vs2_num, j); \
        } else { \
          vs1 = P.VU.elt<float64_t>(vs1_num, i); \
          vs2 = P.VU.elt<float64_t>(vs2_num, i); \
        }\
        BODY64; \
      } \
    } \
  } \


#define MXU_MMACC_MVFP_WIDEN(is_trans, BODY32, BODY64) \
  require(P.MU.msew >= e16 && P.MU.msew <= e32); \
  reg_t tile_m = P.MU.tile_m->read();\
  reg_t tile_n = P.MU.tile_n->read();\
  reg_t sew = P.MU.msew; \
  reg_t td_num = insn.rd(); \
  reg_t vs1_num = insn.rs1(); \
  reg_t vs2_num = insn.rs2(); \
  int128_t res; \
  for (reg_t i = 0; i < tile_m; ++i) { \
    for (reg_t j = 0; j < tile_n; ++j) { \
      if(sew == e16){ \
        auto &td = P.MU.tr_elt<float32_t>(td_num, 0, i, j, mmax, nmax, true); \
        float32_t vs1, vs2; \
        if (!is_trans) { \
          vs1 = f16_to_f32(P.VU.elt<float16_t>(vs1_num, j)); \
          vs2 = f16_to_f32(P.VU.elt<float16_t>(vs2_num, j)); \
        } else { \
          vs1 = f16_to_f32(P.VU.elt<float16_t>(vs1_num, i)); \
          vs2 = f16_to_f32(P.VU.elt<float16_t>(vs2_num, i)); \
        }\
        BODY32; \
      }else if(sew == e32){ \
        auto &td = P.MU.tr_elt<float64_t>(td_num, 0, i, j, mmax, nmax, true); \
        float64_t vs1, vs2; \
        if (!is_trans) { \
          vs1 = f32_to_f64(P.VU.elt<float32_t>(vs1_num, j)); \
          vs2 = f32_to_f64(P.VU.elt<float32_t>(vs2_num, j)); \
        } else { \
          vs1 = f32_to_f64(P.VU.elt<float32_t>(vs1_num, i)); \
          vs2 = f32_to_f64(P.VU.elt<float32_t>(vs2_num, i)); \
        }\
        BODY64; \
      } \
    } \
  } \


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
  for (reg_t i = 0; i < height; i++) { \
    for (reg_t j = 0; j < P.MU.mcols / 8; j++) { \
      P.MU.tr_elt<int8_t>(td, 0, i, j, P.MU.mrows, P.MU.mcols >> 3, true) = 0; \
    } \
  } \

#define CLEAR_ACC(td) \
  for (reg_t i = 0; i < P.MU.mrows; i++) { \
    for (reg_t j = 0; j < P.MU.mcols * 2 / 8; j++) { \
      P.MU.tr_elt<int8_t>(td, 0, i, j, true) = 0; \
    } \
  } \

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
  for (reg_t m = 0; m < lmul; m++) {\
    for (reg_t i = 0; i < height; ++i) { \
      for (reg_t j = 0; j < width; ++j) { \
          elt_width##_t val = MMU.load<elt_width##_t>( \
                    baseAddr + i * stride2 + j * sizeof(elt_width##_t) + \
                      m * (height *stride2 + width * sizeof(elt_width##_t))); \
          P.MU.tr_elt<elt_width##_t>(td + m, is_trans, i, j, rmax, cmax, true) = val; \
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
  for (reg_t m = 0; m < lmul; m++) {\
    for (reg_t i = 0; i < height; ++i) { \
      for (reg_t j = 0; j < width; ++j) { \
          elt_width##_t val = P.MU.tr_elt<elt_width##_t>(td + m, is_trans, i, j, rmax, cmax, true); \
          MMU.store<elt_width##_t>( \
                    baseAddr + i * stride2 + j * sizeof(elt_width##_t) + \
                    m * (height *stride2 + width * sizeof(elt_width##_t)), val); \
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
  reg_t veew = sizeof(elt_width##_t) * 8; \
  float vemul = ((float)veew / P.VU.vsew * P.VU.vflmul); \
  reg_t emul = vemul < 1 ? 1 : vemul; \
  height = emul; \
  require(vemul >= 0.125 && vemul <= 8); \
  require(height*width*sizeof(elt_width##_t) <= P.VU.vlenb * emul); \
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
  reg_t veew = sizeof(elt_width##_t) * 8; \
  float vemul = ((float)veew / P.VU.vsew * P.VU.vflmul); \
  reg_t emul = vemul < 1 ? 1 : vemul; \
  height = emul; \
  require(vemul >= 0.125 && vemul <= 8); \
  require(height*width*sizeof(elt_width##_t) <= P.VU.vlenb * vemul); \
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
  reg_t pr = P.MU.mpad_bottom; \
  reg_t outh = P.MU.outshape[1]; \
  reg_t outw = P.MU.outshape[0]; \
  sreg_t inposh = P.MU.mskin[1]; \
  sreg_t inposw = P.MU.mskin[0]; \
  reg_t krposw = P.MU.mskout[1]; \
  reg_t outposw = P.MU.mskout[0]; \
  reg_t height, width; \
  reg_t rmax = 0, cmax = 0; \
  MTU_LS_LEN(is_trans, dim); \
  CLEAR_TILE(td); \
  for (reg_t i = 0; i < height; ++i) { \
    if (inposh >= 0 && (reg_t)inposh < inh && inposw >= 0 && (reg_t)inposw < inw) { \
      for (reg_t j = 0; j < width; ++j) { \
        elt_width##_t val = MMU.load<elt_width##_t>( \
                      baseAddr + j * sizeof(elt_width##_t)); \
        P.MU.tr_elt<elt_width##_t>(td, is_trans, i, j, rmax, cmax, true) = val; \
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
  reg_t pr = P.MU.mpad_bottom; \
  reg_t outh = P.MU.outshape[1]; \
  reg_t outw = P.MU.outshape[0]; \
  sreg_t inposh = P.MU.mskin[1]; \
  sreg_t inposw = P.MU.mskin[0]; \
  reg_t krposw = P.MU.mskout[1]; \
  reg_t outposw = P.MU.mskout[0]; \
  reg_t height, width; \
  reg_t rmax = 0, cmax = 0; \
  MTU_LS_LEN(is_trans, dim); \
  CLEAR_TILE(td); \
  for (reg_t i = 0; i < height; ++i) { \
    if (inposh >= 0 && (reg_t)inposh < inh && inposw >= 0 && (reg_t)inposw < inw) { \
      for (reg_t j = 0; j < width; ++j) { \
        auto val = P.MU.tr_elt<elt_width##_t>(td, is_trans, i, j, rmax, cmax, true); \
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

#define MU_MFP_LOOP_SCALE_BASE \
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
  for (reg_t m = 0; m < lmul; m++) { \
    for (reg_t i = 0; i < tile_m; ++i) { \
      for (reg_t j = 0; j < tile_n; ++j) { \


#define MU_MFP_LOOP_END \
    } \
  } \
  } \

#define MXU_MFP_CVT_SCALE(BODY8, BODY16, BODY32, BODY64,\
                         CHECK8, CHECK16, CHECK32, CHECK64,\
                         is_widen, eew_check) \
  require(eew_check); \
  switch(P.MU.msew) { \
    case e8: {\
      CHECK8 \
      MU_MFP_LOOP_SCALE_BASE \
        BODY8 \
        set_fp_exceptions; \
      MU_MFP_LOOP_END \
      } \
      break; \
    case e16: {\
      CHECK16 \
      MU_MFP_LOOP_SCALE_BASE \
        BODY16 \
        set_fp_exceptions; \
      MU_MFP_LOOP_END \
      } \
      break; \
    case e32: {\
      CHECK32 \
      MU_MFP_LOOP_SCALE_BASE \
        BODY32 \
        set_fp_exceptions; \
      MU_MFP_LOOP_END \
      } \
      break; \
    case e64: {\
      CHECK64 \
      MU_MFP_LOOP_SCALE_BASE \
        BODY64 \
        set_fp_exceptions; \
      MU_MFP_LOOP_END \
      } \
      break; \
    default: \
      require(0); \
      break; \
  }


#define MXU_MFP_CVT(BODY8, BODY16, BODY32, BODY64) \
  switch(P.MU.msew) { \
    case e8: {\
      MU_MFP_LOOP_SCALE_BASE \
        BODY8 \
        set_fp_exceptions; \
      MU_MFP_LOOP_END \
      } \
      break; \
    case e16: {\
      MU_MFP_LOOP_SCALE_BASE \
        BODY16 \
        set_fp_exceptions; \
      MU_MFP_LOOP_END \
      } \
      break; \
    case e32: {\
      MU_MFP_LOOP_SCALE_BASE \
        BODY32 \
        set_fp_exceptions; \
      MU_MFP_LOOP_END \
      } \
      break; \
    case e64: {\
      MU_MFP_LOOP_SCALE_BASE \
        BODY64 \
        set_fp_exceptions; \
      MU_MFP_LOOP_END \
      } \
      break; \
    default: \
      require(0); \
      break; \
  }

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
      val  = P.MU.tr_elt<type_sew_t<parm_type>::type>(ts1_num + m, 0, i, 0, rmax, cmax, false); \
      temp.assign(width, val); \
      memcpy(tr_elt_td + (i * cmax) * sew / 8 + ( m * rmax * cmax), temp.data(), sew * width / 8); \
      MB_GENERAL_LOOP_BASE_END \
      break; \
    case 'f' : \
      MB_GENERAL_LOOP_BASE(height) \
      if (!i && !m){ \
        val  = P.MU.tr_elt<type_sew_t<parm_type>::type>(ts1_num, 0, 0, 0, rmax, cmax, false); \
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
  } \
  

#define MI_LOOP_BASE \
  require(P.MU.msew >= e8 && P.MU.msew <= e64); \
  reg_t tile_m = P.MU.tile_m->read(); \
  reg_t tile_n = P.MU.tile_n->read(); \
  reg_t mmax = P.MU.mrows; \
  reg_t nmax = (P.MU.mcols / P.MU.msew) ; \
  reg_t sew = P.MU.msew; \
  reg_t td_num = insn.td(); \
  reg_t ts1_num = insn.ts1(); \
  reg_t ts2_num = insn.rs2(); \
  for (reg_t i = 0; i < tile_m; i++){ \
    for (reg_t j = 0; j < tile_n; j++){ \

#define  MI_WIDE_OP_AND_ASSIGN(var1, var0, op1, op0, sign) \
  switch(sew) { \
    case e8: { \
      sign##16_t & td_w = P.MU.tr_elt<sign##16_t>(td_num, td_num, 0, i, j, mmax, nmax, false); \
      td_w = (sign##16_t)(sign##8_t)var1 op1 (sign##16_t)(sign##8_t)var0 op0 td_w; \
    } \
    case e16: { \
      sign##32_t & td_w = P.MU.tr_elt<sign##32_t>(td_num, td_num, 0, i, j, mmax, nmax, false); \
      td_w = (sign##32_t)(sign##16_t)var1 op1 (sign##32_t)(sign##16_t)var0 op0 td_w; \
    } \
    break; \
    default: { \
      sign##64_t & td_w = P.MU.tr_elt<sign##64_t>(td_num, td_num, 0, i, j, mmax, nmax, false); \
      td_w = (sign##64_t)(sign##32_t)var1 op1 (sign##64_t)(sign##32_t)var0 op0 td_w; \
    } \
    break; \
  } \

#define MI_MM_LOOP(BODY ,PARAMS, ins) \
  MXU_GENERAL_LOOP_BASE(ins) \
    MXU_LOOP_ELEMENT_SKIP(); \
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
  MXU_LOOP_END \

#define MI_MM_LOOP_WIDEN(BODY ,PARAMS, ins) \
  MXU_GENERAL_LOOP_BASE(ins) \
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

#define MI_MM_LOOP_QUEN(BODY, PARAMS) \
  MXU_GENERAL_LOOP_BASE(false) \
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

#define MI_2D_VFP_COMMON \
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

#define MI_2D_VFP_LOOP_BASE \
  MI_2D_VFP_COMMON \
  /*printf("m,k,n = %d, %d, %d\n", tile_m, tile_k, tile_n);*/ \
  for (reg_t m = 0 ; m < lmul; m++ ) { \
    for (reg_t i=0; i<tile_m; ++i) { \
      for (reg_t j=0; j<tile_n; ++j) { \


#define MI_LOOP_END \
      } \
    } \
  } \

#define MI_2D_VFP_LOOP_END \
  MI_LOOP_END

#define MI_VFP_MM_LOOP(BODY16, BODY32, BODY64) \
  MI_2D_VFP_LOOP_BASE \
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

#endif // _RISCV_M_EXT_MACROS_H