#ifndef _RISCV_M_EXT_MACROS_H
#define _RISCV_M_EXT_MACROS_H

#include "vector_unit.h"
#include "v_ext_macros.h"
#include <math.h>


#define MXU_PARAMS(x) \
  type_sew_t<x>::type &accd = P.MU.acc_elt<type_sew_t<x>::type>(accd_num, 0, i, j, true); \
  type_sew_t<x>::type &ts1  = P.MU.tr_elt<type_sew_t<x>::type>(ts1_num, 0, i, k, false); \
  type_sew_t<x>::type &ts2  = P.MU.tr_elt<type_sew_t<x>::type>(ts2_num, 0, k, j, false); \

#define MTU_MV_LEN(trans, dim) \
  switch (dim) \
  { \
  case 'c' : \
    width = trans? P.MU.tile_m : P.MU.tile_n; \
    break; \
  case 'a' : \
    width = trans? P.MU.tile_m : P.MU.tile_k; \
    break; \
  case 'b' : \
    width = trans? P.MU.tile_k : P.MU.tile_n; \
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
  MTU_MV_LEN(is_trans, dim); \
  float vemul = (float)P.MU.msew / P.VU.vsew * P.VU.vflmul; \
  height = vemul < 1 ? 1 : vemul; \
  for (reg_t i = 0; i < height; ++i) { \
    for (reg_t j = 0; j < width; ++j) { \

#define MTU_VREG_TR_PARAMS(trans, x) \
  type_sew_t<x>::type &vd = P.VU.elt<type_sew_t<x>::type>(rd_num, i*width+j, true); \
  type_sew_t<x>::type ts1 = P.MU.tr_elt<type_sew_t<x>::type>(rs1_num, trans, i+start_height, j); \

#define MTU_VREG_ACC_PARAMS(trans, x) \
  type_sew_t<x>::type &vd = P.VU.elt<type_sew_t<x>::type>(rd_num, i*width+j, true); \
  type_sew_t<x>::type acc1 = P.MU.acc_elt<type_sew_t<x>::type>(rs1_num, trans, i+start_height, j); \

#define MTU_TR_VREG_PARAMS(trans, x) \
  type_sew_t<x>::type vs1 = P.VU.elt<type_sew_t<x>::type>(rs1_num, i*width+j); \
  type_sew_t<x>::type &td = P.MU.tr_elt<type_sew_t<x>::type>(rd_num, trans, i+start_height, j, true); \

#define MTU_ACC_VREG_PARAMS(trans, x) \
  type_sew_t<x>::type vs1 = P.VU.elt<type_sew_t<x>::type>(rs1_num, i*width+j); \
  type_sew_t<x>::type &accd = P.MU.acc_elt<type_sew_t<x>::type>(rd_num, trans, i+start_height, j, true); \

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

// vreg <-- acc
#define MMV_VREG_FROM_ACC(is_trans, dim) \
  require(P.MU.msew == P.VU.vsew);\
  MMV_GENERAL_LOOP_BASE(is_trans, dim) \
  if (sew == e8){ \
    MTU_VREG_ACC_PARAMS(is_trans, e8); \
    vd = acc1; \
  }else if(sew == e16){ \
    MTU_VREG_ACC_PARAMS(is_trans, e16); \
    vd = acc1; \
  }else if(sew == e32){ \
    MTU_VREG_ACC_PARAMS(is_trans, e32); \
    vd = acc1; \
  }else if(sew == e64){ \
    MTU_VREG_ACC_PARAMS(is_trans, e64); \
    vd = acc1; \
  } \
  MTU_VM_LOOP_END

#define MWMV_VREG_FROM_ACC(is_trans, dim) \
  MMV_GENERAL_LOOP_BASE(is_trans, dim) \
  if (sew == e8){ \
    auto acc1  = P.MU.acc_elt<uint16_t>(rs1_num, 0, i+start_height, j, false); \
    auto &vd = P.VU.elt<uint16_t>(rd_num, i*width+j, true); \
    vd = acc1; \
  }else if(sew == e16){ \
    auto acc1  = P.MU.acc_elt<uint32_t>(rs1_num, 0, i+start_height, j, false); \
    auto &vd = P.VU.elt<uint32_t>(rd_num, i*width+j, true); \
    vd = acc1; \
  }else if(sew == e32){ \
    auto acc1  = P.MU.acc_elt<uint64_t>(rs1_num, 0, i+start_height, j, false); \
    auto &vd = P.VU.elt<uint64_t>(rd_num, i*width+j, true); \
    vd = acc1; \
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

// acc <-- vreg
#define MMV_ACC_FROM_VREG(is_trans, dim) \
  reg_t td_num = insn.rd(); \
  reg_t acc1_num = insn.rs1(); \
  for (reg_t i = 0; i < P.MU.mrows; i++) { \
    for (reg_t j = 0; j < P.MU.mcols / 8; j++) { \
      P.MU.tr_elt<int8_t>(td_num, 0, i, j, true) = P.MU.acc_elt<int8_t>(acc1_num, 0, i, j); \
    } \
  }

// tr <-- acc
#define MMV_TR_FROM_ACC(is_trans, dim) \
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

#define MXU_GENERAL_LOOP_BASE \
  require(P.MU.msew >= e8 && P.MU.msew <= e64); \
  reg_t tile_m = P.MU.tile_m;\
  reg_t tile_k = P.MU.tile_k;\
  reg_t tile_n = P.MU.tile_n;\
  reg_t sew = P.MU.msew; \
  reg_t accd_num = insn.rd(); \
  reg_t ts1_num = insn.rs1(); \
  reg_t ts2_num = insn.rs2(); \
  for (reg_t i = 0; i < tile_m; ++i) { \
    for (reg_t j = 0; j < tile_n; ++j) { \
      for (reg_t k = 0; k < tile_k; ++k) { \

#define MXU_LOOP_ELEMENT_SKIP(BODY)

#define MXU_LOOP_BASE \
    MXU_GENERAL_LOOP_BASE \
    MXU_LOOP_ELEMENT_SKIP();

#define MXU_LOOP_END \
        } \
      } \
    } \

#define MXU_VFP_LOOP_END \
  MXU_LOOP_END

#define MXU_MM_LOOP(BODY) \
  MXU_LOOP_BASE \
  if (sew == e8){ \
    MXU_PARAMS(e8); \
    BODY; \
  }else if(sew == e16){ \
    MXU_PARAMS(e16); \
    BODY; \
  }else if(sew == e32){ \
    MXU_PARAMS(e32); \
    BODY; \
  }else if(sew == e64){ \
    MXU_PARAMS(e64); \
    BODY; \
  } \
  MXU_LOOP_END

#define MXU_CHECK_OVERFLOW(eew) \
  res = res > (pow(2, eew - 1) - 1)? (pow(2, eew - 1) -1): res; \
  res = res < (-pow(2, eew - 1))? (-pow(2, eew - 1)) : res; \
    

#define MXU_MM_ADD(opd, op0) \
  require(P.MU.msew >= e8 && P.MU.msew <= e64); \
  reg_t tile_m = P.MU.tile_m;\
  reg_t tile_n = P.MU.tile_n;\
  reg_t sew = P.MU.msew; \
  reg_t accd_num = insn.rd(); \
  reg_t acc1_num = insn.rs1(); \
  int128_t res; \
  for (reg_t i = 0; i < tile_m; ++i) { \
    for (reg_t j = 0; j < tile_n; ++j) { \
      if (sew == e8){ \
        auto &accd = P.MU.acc_elt<type_sew_t<e8>::type>(accd_num, 0, i, j, true); \
        auto acc1  = P.MU.acc_elt<type_sew_t<e8>::type>(acc1_num, 0, i, j, false); \
        res = opd(int128_t)accd op0 (int128_t)acc1; \
        MXU_CHECK_OVERFLOW(e8) \
        accd = (int8_t)res; \
      }else if(sew == e16){ \
        auto &accd = P.MU.acc_elt<type_sew_t<e16>::type>(accd_num, 0, i, j, true); \
        auto acc1  = P.MU.acc_elt<type_sew_t<e16>::type>(acc1_num, 0, i, j, false); \
        res = opd(int128_t)accd op0 (int128_t)acc1; \
        MXU_CHECK_OVERFLOW(e16) \
        accd = (int16_t)res; \
      }else if(sew == e32){ \
        auto &accd = P.MU.acc_elt<type_sew_t<e32>::type>(accd_num, 0, i, j, true); \
        auto acc1  = P.MU.acc_elt<type_sew_t<e32>::type>(acc1_num, 0, i, j, false); \
        res = opd(int128_t)accd op0 (int128_t)acc1; \
        MXU_CHECK_OVERFLOW(e32) \
        accd = (int32_t)res; \
      }else if(sew == e64){ \
        auto &accd = P.MU.acc_elt<type_sew_t<e64>::type>(accd_num, 0, i, j, true); \
        auto acc1  = P.MU.acc_elt<type_sew_t<e64>::type>(acc1_num, 0, i, j, false); \
        res = opd(int128_t)accd op0 (int128_t)acc1; \
        MXU_CHECK_OVERFLOW(e64) \
        accd = (int64_t)res; \
      } \
    } \
  } \

#define MXU_W_MM_ADD(opd, op0) \
  require(P.MU.msew >= e8 && P.MU.msew <= e32); \
  reg_t tile_m = P.MU.tile_m;\
  reg_t tile_n = P.MU.tile_n;\
  reg_t sew = P.MU.msew; \
  reg_t accd_num = insn.rd(); \
  reg_t acc1_num = insn.rs1(); \
  int128_t res; \
  for (reg_t i = 0; i < tile_m; ++i) { \
    for (reg_t j = 0; j < tile_n; ++j) { \
      if (sew == e8){ \
        auto &accd = P.MU.acc_elt<type_sew_t<e16>::type>(accd_num, 0, i, j, true); \
        auto acc1  = P.MU.acc_elt<type_sew_t<e16>::type>(acc1_num, 0, i, j, false); \
        res = opd(int128_t)accd op0 (int128_t)acc1; \
        MXU_CHECK_OVERFLOW(e16) \
        accd = (int16_t)res; \
      }else if(sew == e16){ \
        auto &accd = P.MU.acc_elt<type_sew_t<e32>::type>(accd_num, 0, i, j, true); \
        auto acc1  = P.MU.acc_elt<type_sew_t<e32>::type>(acc1_num, 0, i, j, false); \
        res = opd(int128_t)accd op0 (int128_t)acc1; \
        MXU_CHECK_OVERFLOW(e32) \
        accd = (int32_t)res; \
      }else if(sew == e32){ \
        auto &accd = P.MU.acc_elt<type_sew_t<e64>::type>(accd_num, 0, i, j, true); \
        auto acc1  = P.MU.acc_elt<type_sew_t<e64>::type>(acc1_num, 0, i, j, false); \
        res = opd(int128_t)accd op0 (int128_t)acc1; \
        MXU_CHECK_OVERFLOW(e64) \
        accd = (int64_t)res; \
      }\
    } \
  } \

#define MXU_Q_MM_ADD(opd, op0) \
  require(P.MU.msew >= e8 && P.MU.msew <= e16); \
  reg_t tile_m = P.MU.tile_m;\
  reg_t tile_n = P.MU.tile_n;\
  reg_t sew = P.MU.msew; \
  reg_t accd_num = insn.rd(); \
  reg_t acc1_num = insn.rs1(); \
  int128_t res; \
  for (reg_t i = 0; i < tile_m; ++i) { \
    for (reg_t j = 0; j < tile_n; ++j) { \
      if (sew == e8){ \
        auto &accd = P.MU.acc_elt<type_sew_t<e32>::type>(accd_num, 0, i, j, true); \
        auto acc1  = P.MU.acc_elt<type_sew_t<e32>::type>(acc1_num, 0, i, j, false); \
        res = opd(int128_t)accd op0 (int128_t)acc1; \
        MXU_CHECK_OVERFLOW(e32) \
        accd = (int32_t)res; \
      }else if(sew == e16){ \
        auto &accd = P.MU.acc_elt<type_sew_t<e64>::type>(accd_num, 0, i, j, true); \
        auto acc1  = P.MU.acc_elt<type_sew_t<e64>::type>(acc1_num, 0, i, j, false); \
        res = opd(int128_t)accd op0 (int128_t)acc1; \
        MXU_CHECK_OVERFLOW(e64) \
        accd = (int64_t)res; \
      }\
    } \
  } \



#define MXU_WIDE_OP_AND_ASSIGN(var0, var1, var2, op0, op1, sign) \
  switch(P.MU.msew) { \
  case e8: { \
    sign##16_t accd_w = P.MU.acc_elt<sign##16_t>(accd_num, 0, i, j); \
    P.MU.acc_elt<sign##16_t>(accd_num, 0, i, j, true) = \
      op1((sign##16_t)(sign##8_t)var0 op0 (sign##16_t)(sign##8_t)var1) + var2; \
    } \
    break; \
  case e16: { \
    sign##32_t accd_w = P.MU.acc_elt<sign##32_t>(accd_num, 0, i, j); \
    P.MU.acc_elt<sign##32_t>(accd_num, 0, i, j, true) = \
      op1((sign##32_t)(sign##16_t)var0 op0 (sign##32_t)(sign##16_t)var1) + var2; \
    } \
    break; \
  default: { \
    sign##64_t accd_w = P.MU.acc_elt<sign##64_t>(accd_num, 0, i, j); \
    P.MU.acc_elt<sign##64_t>(accd_num, 0, i, j, true) = \
      op1((sign##64_t)(sign##32_t) var0 op0 (sign##64_t)(sign##32_t)var1) + var2; \
    } \
    break; \
  }

#define MXU_QUAD_OP_AND_ASSIGN(var0, var1, var2, op0, op1, sign) \
  switch(P.MU.msew) { \
  case e8: { \
    sign##32_t accd_q = P.MU.acc_elt<sign##32_t>(accd_num, 0, i, j); \
    P.MU.acc_elt<uint32_t>(accd_num, 0, i, j, true) = \
      op1((sign##32_t)(sign##8_t)var0 op0 (sign##32_t)(sign##8_t)var1) + var2; \
    } \
    break; \
  case e16: { \
    sign##64_t accd_q = P.MU.acc_elt<sign##64_t>(accd_num, 0, i, j); \
    P.MU.acc_elt<uint64_t>(accd_num, 0, i, j, true) = \
      op1((sign##64_t)(sign##16_t)var0 op0 (sign##64_t)(sign##16_t)var1) + var2; \
    } \
    break; \
  }


#define VM_WIDE_CHECK_COMMON \
  require_matrix(true);\

#define VM_CHECK_DSS(is_vs1) \
  VM_WIDE_CHECK_COMMON; \

// widen operation loop
#define MXU_VV_LOOP_WIDEN(BODY) \
  MXU_LOOP_BASE \
  if (sew == e8){ \
    MXU_PARAMS(e8); \
    BODY; \
  }else if(sew == e16){ \
    MXU_PARAMS(e16); \
    BODY; \
  } else if(sew == e32){ \
    MXU_PARAMS(e32); \
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
  reg_t tile_m = P.MU.tile_m;\
  reg_t tile_k = P.MU.tile_k;\
  reg_t tile_n = P.MU.tile_n;\
  reg_t sew = P.MU.msew; \
  reg_t accd_num = insn.rd(); \
  reg_t ts1_num = insn.rs1(); \
  reg_t ts2_num = insn.rs2(); \
  softfloat_roundingMode = STATE.frm->read(); \

#define MXU_VFP_LOOP_BASE \
  MXU_VFP_COMMON \
  /*printf("m,k,n = %d, %d, %d\n", tile_m, tile_k, tile_n);*/ \
  for (reg_t i=0; i<tile_m; ++i) { \
    for (reg_t j=0; j<tile_n; ++j) { \
      for (reg_t k = 0; k < tile_k; ++k) { \

#define MXU_CLEAR \
  reg_t accd_num = insn.rd(); \
  for (reg_t i = 0; i < P.MU.mrows; i++) { \
    for (reg_t j = 0; j < P.MU.mcols * 2 / 8; j++) { \
      P.MU.acc_elt<int8_t>(accd_num, 0, i, j, true) = 0; \
    } \
  }

#define MXU_VFP_VV_LOOP(BODY16, BODY32, BODY64) \
  MXU_VFP_LOOP_BASE \
  switch(P.MU.msew) { \
    case e16: { \
      float16_t &accd = P.MU.acc_elt<float16_t>(accd_num, 0, i, j, true); \
      float16_t ts1 = P.MU.tr_elt<float16_t>(ts1_num, 0, i, k, false); \
      float16_t ts2 = P.MU.tr_elt<float16_t>(ts2_num, 0, k, j, false); \
      BODY16; \
      set_fp_exceptions; \
      break; \
    }\
    case e32: {\
      float32_t &accd = P.MU.acc_elt<float32_t>(accd_num, 0, i, j, true); \
      float32_t ts1 = P.MU.tr_elt<float32_t>(ts1_num, 0, i, k, false); \
      float32_t ts2 = P.MU.tr_elt<float32_t>(ts2_num, 0, k, j, false); \
      BODY32; \
      set_fp_exceptions; \
      break; \
    }\
    case e64: {\
      float64_t &accd = P.MU.acc_elt<float64_t>(accd_num, 0, i, j, true); \
      float64_t ts1 = P.MU.tr_elt<float64_t>(ts1_num, 0, i, k, false); \
      float64_t ts2 = P.MU.tr_elt<float64_t>(ts2_num, 0, k, j, false); \
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
  reg_t tile_m = P.MU.tile_m;\
  reg_t tile_n = P.MU.tile_n;\
  reg_t sew = P.MU.msew; \
  reg_t accd_num = insn.rd(); \
  reg_t acc1_num = insn.rs1(); \
  softfloat_roundingMode = STATE.frm->read(); \
  for (reg_t i=0; i<tile_m; ++i) { \
    for (reg_t j=0; j<tile_n; ++j) { \
      switch(P.MU.msew) { \
      case e16: { \
        float16_t &accd = P.MU.acc_elt<float16_t>(accd_num, 0, i, j, true); \
        float16_t acc1  = P.MU.acc_elt<float16_t>(acc1_num, 0, i, j, false); \
        BODY16; \
        set_fp_exceptions; \
        break; \
      }\
      case e32: {\
        float32_t &accd = P.MU.acc_elt<float32_t>(accd_num, 0, i, j, true); \
        float32_t acc1  = P.MU.acc_elt<float32_t>(acc1_num, 0, i, j, false); \
        BODY32; \
        set_fp_exceptions; \
        break; \
      }\
      case e64: {\
        float64_t &accd = P.MU.acc_elt<float64_t>(accd_num, 0, i, j, true); \
        float64_t acc1  = P.MU.acc_elt<float64_t>(acc1_num, 0, i, j, false); \
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

#define MXU_VFP_W_MM_ADD(BODY16, BODY32, BODY64) \
  require_fp; \
  require((P.MU.msew == e16 && p->extension_enabled(EXT_ZFH)) || \
          (P.MU.msew == e32 && p->extension_enabled('F')) || \
          (P.MU.msew == e64 && p->extension_enabled('D'))); \
  require_matrix(true);\
  require(STATE.frm->read() < 0x5);\
  reg_t tile_m = P.MU.tile_m;\
  reg_t tile_n = P.MU.tile_n;\
  reg_t sew = P.MU.msew; \
  reg_t accd_num = insn.rd(); \
  reg_t acc1_num = insn.rs1(); \
  softfloat_roundingMode = STATE.frm->read(); \
  for (reg_t i=0; i<tile_m; ++i) { \
    for (reg_t j=0; j<tile_n; ++j) { \
      switch(P.MU.msew) { \
      case e8: { \
        float16_t &accd = P.MU.acc_elt<float16_t>(accd_num, 0, i, j, true); \
        float16_t acc1  = P.MU.acc_elt<float16_t>(acc1_num, 0, i, j, false); \
        BODY16; \
        set_fp_exceptions; \
        break; \
      }\
      case e16: {\
        float32_t &accd = P.MU.acc_elt<float32_t>(accd_num, 0, i, j, true); \
        float32_t acc1  = P.MU.acc_elt<float32_t>(acc1_num, 0, i, j, false); \
        BODY32; \
        set_fp_exceptions; \
        break; \
      }\
      case e32: {\
        float64_t &accd = P.MU.acc_elt<float64_t>(accd_num, 0, i, j, true); \
        float64_t acc1  = P.MU.acc_elt<float64_t>(acc1_num, 0, i, j, false); \
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

#define MXU_VFP_VV_LOOP_WIDE(BODY16, BODY32) \
  MXU_VFP_LOOP_BASE \
  switch(P.MU.msew) { \
    case e16: {\
      float32_t &accd_w = P.MU.acc_elt<float32_t>(accd_num, 0, i, j, true); \
      float32_t ts1 = f16_to_f32(P.MU.tr_elt<float16_t>(ts1_num, 0, i, k, false)); \
      float32_t ts2 = f16_to_f32(P.MU.tr_elt<float16_t>(ts2_num, 0, k, j, false)); \
      BODY16; \
      set_fp_exceptions; \
      break; \
    }\
    case e32: {\
      float64_t &accd_w = P.MU.acc_elt<float64_t>(accd_num, 0, i, j, true); \
      float64_t ts1 = f32_to_f64(P.MU.tr_elt<float32_t>(ts1_num, 0, i, k, false)); \
      float64_t ts2 = f32_to_f64(P.MU.tr_elt<float32_t>(ts2_num, 0, k, j, false)); \
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


#define MXU_MEMUL_MX(factor) \
  require(P.MU.msew >= e8 && P.MU.msew <= e64); \
  reg_t tile_m = P.MU.tile_m;\
  reg_t tile_n = P.MU.tile_n;\
  reg_t sew = P.MU.msew; \
  reg_t accd_num = insn.rd(); \
  reg_t acc1_num = insn.rs1(); \
  int128_t res; \
  for (reg_t i = 0; i < tile_m; ++i) { \
    for (reg_t j = 0; j < tile_n; ++j) { \
      if (sew == e8){ \
        auto &accd = P.MU.acc_elt<type_sew_t<e8>::type>(accd_num, 0, i, j, true); \
        auto acc1  = P.MU.acc_elt<type_sew_t<e8>::type>(acc1_num, 0, i, j, false); \
        res = (int128_t)acc1 * (int128_t)factor; \
        MXU_CHECK_OVERFLOW(e8) \
        accd = (int8_t)res; \
      }else if(sew == e16){ \
        auto &accd = P.MU.acc_elt<type_sew_t<e16>::type>(accd_num, 0, i, j, true); \
        auto acc1  = P.MU.acc_elt<type_sew_t<e16>::type>(acc1_num, 0, i, j, false); \
        res = (int128_t)acc1 * (int128_t)factor; \
        MXU_CHECK_OVERFLOW(e16) \
        accd = (int16_t)res; \
      }else if(sew == e32){ \
        auto &accd = P.MU.acc_elt<type_sew_t<e32>::type>(accd_num, 0, i, j, true); \
        auto acc1  = P.MU.acc_elt<type_sew_t<e32>::type>(acc1_num, 0, i, j, false); \
        res = (int128_t)acc1 * (int128_t)factor; \
        MXU_CHECK_OVERFLOW(e32) \
        accd = (int32_t)res; \
      }else if(sew == e64){ \
        auto &accd = P.MU.acc_elt<type_sew_t<e64>::type>(accd_num, 0, i, j, true); \
        auto acc1  = P.MU.acc_elt<type_sew_t<e64>::type>(acc1_num, 0, i, j, false); \
        res = (int128_t)accd * (int128_t)factor; \
        MXU_CHECK_OVERFLOW(e64) \
        accd = (int64_t)res; \
      } \
    } \
  } \

#define MXU_MEMUL_MX_WIDEN(factor) \
  require(P.MU.msew >= e8 && P.MU.msew <= e32); \
  reg_t tile_m = P.MU.tile_m;\
  reg_t tile_n = P.MU.tile_n;\
  reg_t sew = P.MU.msew; \
  reg_t accd_num = insn.rd(); \
  reg_t acc1_num = insn.rs1(); \
  int128_t res; \
  for (reg_t i = 0; i < tile_m; ++i) { \
    for (reg_t j = 0; j < tile_n; ++j) { \
      if (sew == e8){ \
        auto &accd = P.MU.acc_elt<type_sew_t<e16>::type>(accd_num, 0, i, j, true); \
        auto acc1  = P.MU.acc_elt<type_sew_t<e16>::type>(acc1_num, 0, i, j, false); \
        res = (int128_t)acc1 * (int128_t)factor; \
        MXU_CHECK_OVERFLOW(e16) \
        accd = (int16_t)res; \
      }else if(sew == e16){ \
        auto &accd = P.MU.acc_elt<type_sew_t<e32>::type>(accd_num, 0, i, j, true); \
        auto acc1  = P.MU.acc_elt<type_sew_t<e32>::type>(acc1_num, 0, i, j, false); \
        res = (int128_t)acc1 * (int128_t)factor; \
        MXU_CHECK_OVERFLOW(e32) \
        accd = (int32_t)res; \
      }else if(sew == e32){ \
        auto &accd = P.MU.acc_elt<type_sew_t<e64>::type>(accd_num, 0, i, j, true); \
        auto acc1  = P.MU.acc_elt<type_sew_t<e64>::type>(acc1_num, 0, i, j, false); \
        res = (int128_t)acc1 * (int128_t)factor; \
        MXU_CHECK_OVERFLOW(e64) \
        accd = (int64_t)res; \
      } \
    } \
  } \

#define MXU_MEMUL_MX_QUAD(factor) \
  require(P.MU.msew >= e8 && P.MU.msew <= e16); \
  reg_t tile_m = P.MU.tile_m;\
  reg_t tile_n = P.MU.tile_n;\
  reg_t sew = P.MU.msew; \
  reg_t accd_num = insn.rd(); \
  reg_t acc1_num = insn.rs1(); \
  int128_t res; \
  for (reg_t i = 0; i < tile_m; ++i) { \
    for (reg_t j = 0; j < tile_n; ++j) { \
      if (sew == e8){ \
        auto &accd = P.MU.acc_elt<type_sew_t<e32>::type>(accd_num, 0, i, j, true); \
        auto acc1  = P.MU.acc_elt<type_sew_t<e32>::type>(acc1_num, 0, i, j, false); \
        res = (int128_t)acc1 * (int128_t)factor; \
        MXU_CHECK_OVERFLOW(e32) \
        accd = (int32_t)res; \
      }else if(sew == e16){ \
        auto &accd = P.MU.acc_elt<type_sew_t<e64>::type>(accd_num, 0, i, j, true); \
        auto acc1  = P.MU.acc_elt<type_sew_t<e64>::type>(acc1_num, 0, i, j, false); \
        res = (int128_t)acc1 * (int128_t)factor; \
        MXU_CHECK_OVERFLOW(e64) \
        accd = (int64_t)res; \
      } \
    } \
  } \


#define MXU_MEMUL_MFP(BODY16, BODY32, BODY64) \
  require(P.MU.msew >= e16 && P.MU.msew <= e64); \
  reg_t tile_m = P.MU.tile_m;\
  reg_t tile_n = P.MU.tile_n;\
  reg_t sew = P.MU.msew; \
  reg_t accd_num = insn.rd(); \
  reg_t acc1_num = insn.rs1(); \
  reg_t rs2_num = insn.rs2(); \
  int128_t res; \
  for (reg_t i = 0; i < tile_m; ++i) { \
    for (reg_t j = 0; j < tile_n; ++j) { \
      if(sew == e16){ \
        auto &accd = P.MU.acc_elt<float16_t>(accd_num, 0, i, j, true); \
        auto acc1  = P.MU.acc_elt<float16_t>(acc1_num, 0, i, j, false); \
        float16_t rs2 = f16(READ_FREG(rs2_num)); \
        BODY16; \
      }else if(sew == e32){ \
        auto &accd = P.MU.acc_elt<float32_t>(accd_num, 0, i, j, true); \
        auto acc1  = P.MU.acc_elt<float32_t>(acc1_num, 0, i, j, false); \
        float32_t rs2 = f32(READ_FREG(rs2_num)); \
        BODY32; \
      }else if(sew == e64){ \
        auto &accd = P.MU.acc_elt<float64_t>(accd_num, 0, i, j, true); \
        auto acc1  = P.MU.acc_elt<float64_t>(acc1_num, 0, i, j, false); \
        float64_t rs2 = f64(READ_FREG(rs2_num)); \
        BODY64; \
      } \
    } \
  } \

#define MXU_MEMUL_MFP_WIDEN(BODY32, BODY64) \
  require(P.MU.msew >= e16 && P.MU.msew <= e32); \
  reg_t tile_m = P.MU.tile_m;\
  reg_t tile_n = P.MU.tile_n;\
  reg_t sew = P.MU.msew; \
  reg_t accd_num = insn.rd(); \
  reg_t acc1_num = insn.rs1(); \
  reg_t rs2_num = insn.rs2(); \
  int128_t res; \
  for (reg_t i = 0; i < tile_m; ++i) { \
    for (reg_t j = 0; j < tile_n; ++j) { \
      if(sew == e16){ \
        auto &accd = P.MU.acc_elt<float32_t>(accd_num, 0, i, j, true); \
        auto acc1  = P.MU.acc_elt<float32_t>(acc1_num, 0, i, j, false); \
        float32_t rs2 = f32(READ_FREG(rs2_num)); \
        BODY32; \
      }else if(sew == e32){ \
        auto &accd = P.MU.acc_elt<float64_t>(accd_num, 0, i, j, true); \
        auto acc1  = P.MU.acc_elt<float64_t>(acc1_num, 0, i, j, false); \
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
  reg_t height = P.MU.tile_m;\
  reg_t width = P.MU.tile_n;\
  reg_t sew = P.MU.msew; \
  reg_t accd_num = insn.rd(); \
  reg_t acc1_num = insn.rs1(); \
  reg_t vs2_num = insn.rs2(); \
  int128_t res, vs2; \
  for (reg_t i = 0; i < height; ++i) { \
    for (reg_t j = 0; j < width; ++j) { \
      if (sew == e8){ \
        GET_VS2(e8, is_trans, i, j) \
        auto &accd = P.MU.acc_elt<type_sew_t<e8>::type>(accd_num, 0, i, j, true); \
        auto acc1  = P.MU.acc_elt<type_sew_t<e8>::type>(acc1_num, 0, i, j, false); \
        BODY \
        MXU_CHECK_OVERFLOW(e8) \
        accd = (int8_t)res; \
      }else if(sew == e16){ \
        GET_VS2(e16, is_trans, i, j) \
        auto &accd = P.MU.acc_elt<type_sew_t<e16>::type>(accd_num, 0, i, j, true); \
        auto acc1  = P.MU.acc_elt<type_sew_t<e16>::type>(acc1_num, 0, i, j, false); \
        BODY \
        MXU_CHECK_OVERFLOW(e16) \
        accd = (int16_t)res; \
      }else if(sew == e32){ \
        GET_VS2(e32, is_trans, i, j) \
        auto &accd = P.MU.acc_elt<type_sew_t<e32>::type>(accd_num, 0, i, j, true); \
        auto acc1  = P.MU.acc_elt<type_sew_t<e32>::type>(acc1_num, 0, i, j, false); \
        BODY \
        MXU_CHECK_OVERFLOW(e32) \
        accd = (int32_t)res; \
      }else if(sew == e64){ \
        GET_VS2(e64, is_trans, i, j) \
        auto &accd = P.MU.acc_elt<type_sew_t<e64>::type>(accd_num, 0, i, j, true); \
        auto acc1  = P.MU.acc_elt<type_sew_t<e64>::type>(acc1_num, 0, i, j, false); \
        BODY \
        MXU_CHECK_OVERFLOW(e64) \
        accd = (int64_t)res; \
      } \
    } \
  } \


#define MXU_MEMUL_MV_WIDEN(is_trans, BODY) \
  require(P.MU.msew >= e8 && P.MU.msew <= e32); \
  reg_t height = P.MU.tile_m;\
  reg_t width = P.MU.tile_n;\
  reg_t sew = P.MU.msew; \
  reg_t accd_num = insn.rd(); \
  reg_t acc1_num = insn.rs1(); \
  reg_t vs2_num = insn.rs2(); \
  int128_t res, vs2; \
  for (reg_t i = 0; i < height; ++i) { \
    for (reg_t j = 0; j < width; ++j) { \
      if (sew == e8){ \
        GET_VS2(e8, is_trans, i, j) \
        auto &accd = P.MU.acc_elt<type_sew_t<e16>::type>(accd_num, 0, i, j, true); \
        auto acc1  = P.MU.acc_elt<type_sew_t<e16>::type>(acc1_num, 0, i, j, false); \
        BODY \
        MXU_CHECK_OVERFLOW(e16) \
        accd = (int16_t)res; \
      }else if(sew == e16){ \
        GET_VS2(e16, is_trans, i, j) \
        auto &accd = P.MU.acc_elt<type_sew_t<e32>::type>(accd_num, 0, i, j, true); \
        auto acc1  = P.MU.acc_elt<type_sew_t<e32>::type>(acc1_num, 0, i, j, false); \
        BODY \
        MXU_CHECK_OVERFLOW(e32) \
        accd = (int32_t)res; \
      }else if(sew == e32){ \
        GET_VS2(e32, is_trans, i, j) \
        auto &accd = P.MU.acc_elt<type_sew_t<e64>::type>(accd_num, 0, i, j, true); \
        auto acc1  = P.MU.acc_elt<type_sew_t<e64>::type>(acc1_num, 0, i, j, false); \
        BODY \
        MXU_CHECK_OVERFLOW(e64) \
        accd = (int64_t)res; \
      } \
    } \
  } \

#define MXU_MEMUL_MV_QUAD(is_trans, BODY) \
  require(P.MU.msew >= e8 && P.MU.msew <= e16); \
  reg_t height = P.MU.tile_m;\
  reg_t width = P.MU.tile_n;\
  reg_t sew = P.MU.msew; \
  reg_t accd_num = insn.rd(); \
  reg_t acc1_num = insn.rs1(); \
  reg_t vs2_num = insn.rs2(); \
  int128_t res, vs2; \
  for (reg_t i = 0; i < height; ++i) { \
    for (reg_t j = 0; j < width; ++j) { \
      if (sew == e8){ \
        GET_VS2(e8, is_trans, i, j) \
        auto &accd = P.MU.acc_elt<type_sew_t<e32>::type>(accd_num, 0, i, j, true); \
        auto acc1  = P.MU.acc_elt<type_sew_t<e32>::type>(acc1_num, 0, i, j, false); \
        BODY \
        MXU_CHECK_OVERFLOW(e32) \
        accd = (int32_t)res; \
      }else if(sew == e16){ \
        GET_VS2(e16, is_trans, i, j) \
        auto &accd = P.MU.acc_elt<type_sew_t<e64>::type>(accd_num, 0, i, j, true); \
        auto acc1  = P.MU.acc_elt<type_sew_t<e64>::type>(acc1_num, 0, i, j, false); \
        BODY \
        MXU_CHECK_OVERFLOW(e64) \
        accd = (int64_t)res; \
      } \
    } \
  } \

#define MXU_MEMUL_MVFP(is_trans, BODY16, BODY32, BODY64) \
  require(P.MU.msew >= e16 && P.MU.msew <= e64); \
  reg_t tile_m = P.MU.tile_m;\
  reg_t tile_n = P.MU.tile_n;\
  reg_t sew = P.MU.msew; \
  reg_t accd_num = insn.rd(); \
  reg_t acc1_num = insn.rs1(); \
  reg_t vs2_num = insn.rs2(); \
  int128_t res; \
  for (reg_t i = 0; i < tile_m; ++i) { \
    for (reg_t j = 0; j < tile_n; ++j) { \
      if(sew == e16){ \
        auto &accd = P.MU.acc_elt<float16_t>(accd_num, 0, i, j, true); \
        auto acc1  = P.MU.acc_elt<float16_t>(acc1_num, 0, i, j, false); \
        float16_t rs2; \
        if (!is_trans) { \
          rs2 = P.VU.elt<float16_t>(vs2_num, j); \
        } else { \
          rs2 = P.VU.elt<float16_t>(vs2_num, i); \
        }\
        BODY16; \
      }else if(sew == e32){ \
        auto &accd = P.MU.acc_elt<float32_t>(accd_num, 0, i, j, true); \
        auto acc1  = P.MU.acc_elt<float32_t>(acc1_num, 0, i, j, false); \
        float32_t rs2; \
        if (!is_trans) { \
          rs2 = P.VU.elt<float32_t>(vs2_num, j); \
        } else { \
          rs2 = P.VU.elt<float32_t>(vs2_num, i); \
        }\
        BODY32; \
      }else if(sew == e64){ \
        auto &accd = P.MU.acc_elt<float64_t>(accd_num, 0, i, j, true); \
        auto acc1  = P.MU.acc_elt<float64_t>(acc1_num, 0, i, j, false); \
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
  reg_t tile_m = P.MU.tile_m;\
  reg_t tile_n = P.MU.tile_n;\
  reg_t sew = P.MU.msew; \
  reg_t accd_num = insn.rd(); \
  reg_t acc1_num = insn.rs1(); \
  reg_t vs2_num = insn.rs2(); \
  int128_t res; \
  for (reg_t i = 0; i < tile_m; ++i) { \
    for (reg_t j = 0; j < tile_n; ++j) { \
      if(sew == e16){ \
        auto &accd = P.MU.acc_elt<float32_t>(accd_num, 0, i, j, true); \
        auto acc1  = P.MU.acc_elt<float32_t>(acc1_num, 0, i, j, false); \
        float32_t rs2; \
        if (!is_trans) { \
          rs2 = f16_to_f32(P.VU.elt<float16_t>(vs2_num, j)); \
        } else { \
          rs2 = f16_to_f32(P.VU.elt<float16_t>(vs2_num, i)); \
        }\
        BODY32; \
      }else if(sew == e32){ \
        auto &accd = P.MU.acc_elt<float64_t>(accd_num, 0, i, j, true); \
        auto acc1  = P.MU.acc_elt<float64_t>(acc1_num, 0, i, j, false); \
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
  reg_t height = P.MU.tile_m;\
  reg_t width = P.MU.tile_n;\
  reg_t sew = P.MU.msew; \
  reg_t accd_num = insn.rd(); \
  reg_t vs1_num = insn.rs1(); \
  reg_t vs2_num = insn.rs2(); \
  int128_t res, vs1, vs2; \
  for (reg_t i = 0; i < height; ++i) { \
    for (reg_t j = 0; j < width; ++j) { \
      if (sew == e8){ \
        GET_VS1(e8, is_trans, i, j) \
        GET_VS2(e8, is_trans, i, j) \
        auto &accd = P.MU.acc_elt<type_sew_t<e8>::type>(accd_num, 0, i, j, true); \
        BODY \
        MXU_CHECK_OVERFLOW(e8) \
        accd = (int8_t)res; \
      }else if(sew == e16){ \
        GET_VS1(e16, is_trans, i, j) \
        GET_VS2(e16, is_trans, i, j) \
        auto &accd = P.MU.acc_elt<type_sew_t<e16>::type>(accd_num, 0, i, j, true); \
        BODY \
        MXU_CHECK_OVERFLOW(e16) \
        accd = (int16_t)res; \
      }else if(sew == e32){ \
        GET_VS1(e32, is_trans, i, j) \
        GET_VS2(e32, is_trans, i, j) \
        auto &accd = P.MU.acc_elt<type_sew_t<e32>::type>(accd_num, 0, i, j, true); \
        BODY \
        MXU_CHECK_OVERFLOW(e32) \
        accd = (int32_t)res; \
      }else if(sew == e64){ \
        GET_VS1(e64, is_trans, i, j) \
        GET_VS2(e64, is_trans, i, j) \
        auto &accd = P.MU.acc_elt<type_sew_t<e64>::type>(accd_num, 0, i, j, true); \
        BODY \
        MXU_CHECK_OVERFLOW(e64) \
        accd = (int64_t)res; \
      } \
    } \
  } \


#define MXU_MMACC_MV_WIDEN(is_trans, BODY) \
  require(P.MU.msew >= e8 && P.MU.msew <= e32); \
  reg_t height = P.MU.tile_m;\
  reg_t width = P.MU.tile_n;\
  reg_t sew = P.MU.msew; \
  reg_t accd_num = insn.rd(); \
  reg_t vs1_num = insn.rs1(); \
  reg_t vs2_num = insn.rs2(); \
  int128_t res, vs1, vs2; \
  for (reg_t i = 0; i < height; ++i) { \
    for (reg_t j = 0; j < width; ++j) { \
      if (sew == e8){ \
        GET_VS1(e8, is_trans, i, j) \
        GET_VS2(e8, is_trans, i, j) \
        auto &accd = P.MU.acc_elt<type_sew_t<e16>::type>(accd_num, 0, i, j, true); \
        BODY \
        MXU_CHECK_OVERFLOW(e16) \
        accd = (int16_t)res; \
      }else if(sew == e16){ \
        GET_VS1(e16, is_trans, i, j) \
        GET_VS2(e16, is_trans, i, j) \
        auto &accd = P.MU.acc_elt<type_sew_t<e32>::type>(accd_num, 0, i, j, true); \
        BODY \
        MXU_CHECK_OVERFLOW(e32) \
        accd = (int32_t)res; \
      }else if(sew == e32){ \
        GET_VS1(e32, is_trans, i, j) \
        GET_VS2(e32, is_trans, i, j) \
        auto &accd = P.MU.acc_elt<type_sew_t<e64>::type>(accd_num, 0, i, j, true); \
        BODY \
        MXU_CHECK_OVERFLOW(e64) \
        accd = (int64_t)res; \
      } \
    } \
  } \


#define MXU_MMACC_MV_QUAD(is_trans, BODY) \
  require(P.MU.msew >= e8 && P.MU.msew <= e16); \
  reg_t height = P.MU.tile_m;\
  reg_t width = P.MU.tile_n;\
  reg_t sew = P.MU.msew; \
  reg_t accd_num = insn.rd(); \
  reg_t vs1_num = insn.rs1(); \
  reg_t vs2_num = insn.rs2(); \
  int128_t res, vs1, vs2; \
  for (reg_t i = 0; i < height; ++i) { \
    for (reg_t j = 0; j < width; ++j) { \
      if (sew == e8){ \
        GET_VS1(e8, is_trans, i, j) \
        GET_VS2(e8, is_trans, i, j) \
        auto &accd = P.MU.acc_elt<type_sew_t<e32>::type>(accd_num, 0, i, j, true); \
        BODY \
        MXU_CHECK_OVERFLOW(e32) \
        accd = (int32_t)res; \
      }else if(sew == e16){ \
        GET_VS1(e16, is_trans, i, j) \
        GET_VS2(e16, is_trans, i, j) \
        auto &accd = P.MU.acc_elt<type_sew_t<e64>::type>(accd_num, 0, i, j, true); \
        BODY \
        MXU_CHECK_OVERFLOW(e64) \
        accd = (int64_t)res; \
      } \
    } \
  } \

#define MXU_MMACC_MVFP(is_trans, BODY16, BODY32, BODY64) \
  require(P.MU.msew >= e16 && P.MU.msew <= e64); \
  reg_t tile_m = P.MU.tile_m;\
  reg_t tile_n = P.MU.tile_n;\
  reg_t sew = P.MU.msew; \
  reg_t accd_num = insn.rd(); \
  reg_t vs1_num = insn.rs1(); \
  reg_t vs2_num = insn.rs2(); \
  int128_t res; \
  for (reg_t i = 0; i < tile_m; ++i) { \
    for (reg_t j = 0; j < tile_n; ++j) { \
      if(sew == e16){ \
        auto &accd = P.MU.acc_elt<float16_t>(accd_num, 0, i, j, true); \
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
        auto &accd = P.MU.acc_elt<float32_t>(accd_num, 0, i, j, true); \
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
        auto &accd = P.MU.acc_elt<float64_t>(accd_num, 0, i, j, true); \
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
  reg_t tile_m = P.MU.tile_m;\
  reg_t tile_n = P.MU.tile_n;\
  reg_t sew = P.MU.msew; \
  reg_t accd_num = insn.rd(); \
  reg_t vs1_num = insn.rs1(); \
  reg_t vs2_num = insn.rs2(); \
  int128_t res; \
  for (reg_t i = 0; i < tile_m; ++i) { \
    for (reg_t j = 0; j < tile_n; ++j) { \
      if(sew == e16){ \
        auto &accd = P.MU.acc_elt<float32_t>(accd_num, 0, i, j, true); \
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
        auto &accd = P.MU.acc_elt<float64_t>(accd_num, 0, i, j, true); \
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


#define MTU_LS_LEN(trans, ch) \
  switch (ch) \
  { \
  case 'c' : \
    height = trans? P.MU.tile_n : P.MU.tile_m; \
    width = trans? P.MU.tile_m : P.MU.tile_n; \
    break; \
  case 'a' : \
    height = trans? P.MU.tile_k : P.MU.tile_m; \
    width = trans? P.MU.tile_m : P.MU.tile_k; \
    break; \
  case 'b' : \
    height = trans? P.MU.tile_n : P.MU.tile_k; \
    width = trans? P.MU.tile_k : P.MU.tile_n; \
    break; \
  default : \
    break; \
  }; \


#define CLEAR_TILE(td) \
  for (reg_t i = 0; i < height; i++) { \
    for (reg_t j = 0; j < P.MU.mcols / 8; j++) { \
      P.MU.tr_elt<int8_t>(td, 0, i, j, true) = 0; \
    } \
  } \

#define CLEAR_ACC(accd) \
  for (reg_t i = 0; i < P.MU.mrows; i++) { \
    for (reg_t j = 0; j < P.MU.mcols * 2 / 8; j++) { \
      P.MU.acc_elt<int8_t>(accd, 0, i, j, true) = 0; \
    } \
  } \

#define MTU_TR_LD(is_trans, dim, elt_width) \
  const reg_t baseAddr = RS1; \
  const reg_t stride2 = RS2; \
  const reg_t td = insn.rd(); \
  reg_t height, width; \
  MTU_LS_LEN(is_trans, dim); \
  CLEAR_TILE(td); \
  for (reg_t i = 0; i < height; ++i) { \
    for (reg_t j = 0; j < width; ++j) { \
        elt_width##_t val = MMU.load<elt_width##_t>( \
                  baseAddr + i * stride2 + j * sizeof(elt_width##_t)); \
        P.MU.tr_elt<elt_width##_t>(td, is_trans, i, j, true) = val; \
    } \
  } \

#define MTU_TR_ST(is_trans, dim, elt_width) \
  const reg_t baseAddr = RS1; \
  const reg_t stride2 = RS2; \
  const reg_t td = insn.rd(); \
  reg_t height, width; \
  MTU_LS_LEN(is_trans, dim); \
  for (reg_t i = 0; i < height; ++i) { \
    for (reg_t j = 0; j < width; ++j) { \
        elt_width##_t val = P.MU.tr_elt<elt_width##_t>(td, is_trans, i, j, true); \
        MMU.store<elt_width##_t>( \
                  baseAddr + i * stride2 + j * sizeof(elt_width##_t), val); \
    } \
  } \

#define MTU_ACC_LD(is_trans, dim, elt_width) \
  const reg_t baseAddr = RS1; \
  const reg_t stride2 = RS2; \
  const reg_t accd = insn.rd(); \
  reg_t height, width; \
  MTU_LS_LEN(is_trans, dim); \
  CLEAR_ACC(accd); \
  for (reg_t i = 0; i < height; ++i) { \
    for (reg_t j = 0; j < width; ++j) { \
        elt_width##_t val = MMU.load<elt_width##_t>( \
                  baseAddr + i * stride2 + j * sizeof(elt_width##_t)); \
        P.MU.acc_elt<elt_width##_t>(accd, is_trans, i, j, true) = val; \
    } \
  } \

#define MTU_ACC_ST(is_trans, dim, elt_width) \
  const reg_t baseAddr = RS1; \
  const reg_t stride2 = RS2; \
  const reg_t accd = insn.rd(); \
  reg_t height, width; \
  MTU_LS_LEN(is_trans, dim); \
  for (reg_t i = 0; i < height; ++i) { \
    for (reg_t j = 0; j < width; ++j) { \
        elt_width##_t val = P.MU.acc_elt<elt_width##_t>(accd, is_trans, i, j, true); \
        MMU.store<elt_width##_t>( \
                  baseAddr + i * stride2 + j * sizeof(elt_width##_t), val); \
    } \
  } \

#define MTU_VECTOR_LD(is_trans, dim, elt_width) \
  const reg_t baseAddr = RS1; \
  const reg_t stride2 = RS2; \
  const reg_t vd = insn.rd(); \
  reg_t height, width; \
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


#define MTU_UF_TR_LD(is_trans, elt_width) \
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
  MTU_LS_LEN(is_trans, 'a'); \
  CLEAR_TILE(td); \
  for (reg_t i = 0; i < height; ++i) { \
    if (inposh >= 0 && (reg_t)inposh < inh && inposw >= 0 && (reg_t)inposw < inw) { \
      for (reg_t j = 0; j < width; ++j) { \
        elt_width##_t val = MMU.load<elt_width##_t>( \
                      baseAddr + j * sizeof(elt_width##_t)); \
        P.MU.tr_elt<elt_width##_t>(td, is_trans, i, j, true) = val; \
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
  const reg_t acc1_num = insn.rs1(); \
  const reg_t accd_num = insn.rd(); \
  reg_t height = P.MU.tile_m; \
  reg_t width = P.MU.tile_n; \
  softfloat_roundingMode = STATE.frm->read(); \
  for (reg_t i = 0; i < height; ++i) { \
    for (reg_t j = 0; j < width; ++j) { \


#define MU_MFP_LOOP_END \
    } \
  } \

#define MXU_MFP_CVT_SCALE(BODY8, BODY16, BODY32, \
                         CHECK8, CHECK16, CHECK32, \
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

#endif // _RISCV_M_EXT_MACROS_H