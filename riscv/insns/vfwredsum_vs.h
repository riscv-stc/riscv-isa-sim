// // vfwredsum.vs vd, vs2, vs1
// bool is_propagate = true;
// VI_VFP_VV_LOOP_WIDE_REDUCTION
// ({
//   vd_0 = f32_add(vd_0, vs2);
// },
// {
//   vd_0 = f64_add(vd_0, vs2);
// })

/*######################
  g0 = (vs1+vs2[8k])，
  g1 = (vs2[8k+1])，
  …，
  g8 = (vs2[8k+8])，
    (vs2[8k+1])= vs2[1] + vs2[9] + … + vs2[8k+1], k=0-vl/8
  vd = ((g0+g1) + (g2+g3)) + ((g4+g5) + (g6+g7))
######################*/
require(P.VU.vstart == 0);

#define SET  (8)
#define GET_W16_UNORDER_VS2_VALUE(vs2, idx, zero)\
  if (idx >= vl) {\
    vs2 = zero;\
  } else {\
    if (insn.v_vm() == 0) {\
      bool skip = ((P.VU.elt<uint64_t>(0, (idx/64)) >> (idx%64)) & 0x1) == 0;\
      if (skip) {\
        vs2 = zero;\
      } else {\
        vs2 = (STATE.bf16) ? bf16_to_f32(P.VU.elt<bfloat16_t>(rs2_num,idx)) : f16_to_f32(P.VU.elt<float16_t>(rs2_num,idx));\
        is_active = true;\
      }\
    } else {\
      vs2 = (STATE.bf16) ? bf16_to_f32(P.VU.elt<bfloat16_t>(rs2_num,idx)) : f16_to_f32(P.VU.elt<float16_t>(rs2_num,idx));\
    }\
  }

#define VI_VFP_REDUCTION_W16_SUM_UNORDER()\
  uint32_t val0 = 0x80000000; \
  float32_t *f0 = reinterpret_cast<float32_t*>(&val0);\
  float32_t vd_0  = P.VU.elt<float32_t>(rd_num, 0); \
  float32_t vs1_0 = P.VU.elt<float32_t>(rs1_num, 0);\
  float32_t vs2;\
  float32_t zero  = STATE.frm == 0x2 ? f32(0) : *f0;\
  float32_t sum_g0 = zero;\
  float32_t sum_g1 = zero;\
  float32_t sum_g2 = zero;\
  float32_t sum_g3 = zero;\
  float32_t sum_g4 = zero;\
  float32_t sum_g5 = zero;\
  float32_t sum_g6 = zero;\
  float32_t sum_g7 = zero;\
  bool is_first  = true; \
  reg_t cnt = vl/SET;\
  reg_t rem = vl%SET;\
  if(rem) { cnt += 1;}\
  if(cnt) {\
    for(reg_t i=0; i<cnt; i++) {\
      reg_t idx = SET*i;\
      if (is_first) {\
        GET_W16_UNORDER_VS2_VALUE(vs2, idx, zero)   \
        sum_g0 = f32_add(vs1_0,vs2);                \
        is_first = false;                           \
      } else {\
        GET_W16_UNORDER_VS2_VALUE(vs2, idx, zero)   \
        sum_g0 = f32_add(sum_g0,vs2);               \
      }\
      GET_W16_UNORDER_VS2_VALUE(vs2, (idx+1), zero) \
      sum_g1 = f32_add(sum_g1, vs2);                \
      GET_W16_UNORDER_VS2_VALUE(vs2, (idx+2), zero) \
      sum_g2 = f32_add(sum_g2, vs2);                \
      GET_W16_UNORDER_VS2_VALUE(vs2, (idx+3), zero) \
      sum_g3 = f32_add(sum_g3, vs2);                \
      GET_W16_UNORDER_VS2_VALUE(vs2, (idx+4), zero) \
      sum_g4 = f32_add(sum_g4, vs2);                \
      GET_W16_UNORDER_VS2_VALUE(vs2, (idx+5), zero) \
      sum_g5 = f32_add(sum_g5, vs2);                \
      GET_W16_UNORDER_VS2_VALUE(vs2, (idx+6), zero) \
      sum_g6 = f32_add(sum_g6, vs2);                \
      GET_W16_UNORDER_VS2_VALUE(vs2, (idx+7), zero) \
      sum_g7 = f32_add(sum_g7, vs2);                \
      set_fp_exceptions;                            \
    }\
    sum_g0 = f32_add(sum_g0 , sum_g1);              \
    sum_g2 = f32_add(sum_g2 , sum_g3);              \
    sum_g4 = f32_add(sum_g4 , sum_g5);              \
    sum_g6 = f32_add(sum_g6 , sum_g7);              \
    sum_g0 = f32_add(sum_g0 , sum_g2);              \
    sum_g4 = f32_add(sum_g4 , sum_g6);              \
    vd_0   = f32_add(sum_g0 , sum_g4);              \
    set_fp_exceptions;                              \


#define GET_W32_UNORDER_VS2_VALUE(vs2, idx, zero) \
  if (idx >= vl) {\
    vs2 = zero;\
  } else {\
    if (insn.v_vm() == 0) {\
      bool skip = ((P.VU.elt<uint64_t>(0, (idx/64)) >> (idx%64)) & 0x1) == 0;\
      if (skip) {\
        vs2 = zero;\
      } else {\
        vs2 = f32_to_f64(P.VU.elt<float32_t>(rs2_num,idx));\
        is_active = true;\
      }\
    } else {\
      vs2 = f32_to_f64(P.VU.elt<float32_t>(rs2_num,idx));\
    }\
  }

#define VI_VFP_REDUCTION_W32_SUM_UNORDER()\
  uint64_t val0 = 0x8000000000000000; \
  float64_t *f0 = reinterpret_cast<float64_t*>(&val0);\
  float64_t vd_0  = P.VU.elt<float64_t>(rd_num, 0); \
  float64_t vs1_0 = P.VU.elt<float64_t>(rs1_num, 0);\
  float64_t vs2;\
  float64_t zero  = STATE.frm == 0x2 ? f64(0) : *f0;\
  float64_t sum_g0 = zero;\
  float64_t sum_g1 = zero;\
  float64_t sum_g2 = zero;\
  float64_t sum_g3 = zero;\
  float64_t sum_g4 = zero;\
  float64_t sum_g5 = zero;\
  float64_t sum_g6 = zero;\
  float64_t sum_g7 = zero;\
  bool is_first  = true; \
  reg_t cnt = vl/SET;\
  reg_t rem = vl%SET;\
  if(rem) { cnt += 1;}\
  if(cnt) {\
    for(reg_t i=0; i<cnt; i++) {\
      reg_t idx = SET*i;\
      if (is_first) {\
        GET_W32_UNORDER_VS2_VALUE(vs2, idx, zero)   \
        sum_g0 = f64_add(vs1_0,vs2);                \
        is_first = false;\
      } else {\
        GET_W32_UNORDER_VS2_VALUE(vs2, idx, zero)   \
        sum_g0 = f64_add(sum_g0, vs2);              \
      }\
      GET_W32_UNORDER_VS2_VALUE(vs2, (idx+1), zero) \
      sum_g1 = f64_add(sum_g1, vs2);                \
      GET_W32_UNORDER_VS2_VALUE(vs2, (idx+2), zero) \
      sum_g2 = f64_add(sum_g2, vs2);                \
      GET_W32_UNORDER_VS2_VALUE(vs2, (idx+3), zero) \
      sum_g3 = f64_add(sum_g3, vs2);                \
      GET_W32_UNORDER_VS2_VALUE(vs2, (idx+4), zero) \
      sum_g4 = f64_add(sum_g4, vs2);                \
      GET_W32_UNORDER_VS2_VALUE(vs2, (idx+5), zero) \
      sum_g5 = f64_add(sum_g5, vs2);                \
      GET_W32_UNORDER_VS2_VALUE(vs2, (idx+6), zero) \
      sum_g6 = f64_add(sum_g6, vs2);                \
      GET_W32_UNORDER_VS2_VALUE(vs2, (idx+7), zero) \
      sum_g7 = f64_add(sum_g7, vs2);                \
      set_fp_exceptions;                            \
      }\
    sum_g0 = f64_add(sum_g0 , sum_g1);              \
    sum_g2 = f64_add(sum_g2 , sum_g3);              \
    sum_g4 = f64_add(sum_g4 , sum_g5);              \
    sum_g6 = f64_add(sum_g6 , sum_g7);              \
    sum_g0 = f64_add(sum_g0 , sum_g2);              \
    sum_g4 = f64_add(sum_g4 , sum_g6);              \
    vd_0   = f64_add(sum_g0 , sum_g4);              \
    set_fp_exceptions;                              \


/*########### Begin ###########*/    
bool is_propagate = true;
bool is_active = false;

VI_CHECK_REDUCTION(true) 
VI_VFP_COMMON 
require((P.VU.vsew == e16 && p->supports_extension('F')) || 
        (P.VU.vsew == e32 && p->supports_extension('D'))); 

switch(P.VU.vsew) {
  case e16: {
    VI_VFP_REDUCTION_W16_SUM_UNORDER() 
    VI_VFP_LOOP_REDUCTION_END(e32)       
    break; 
  }
  case e32: {
    VI_VFP_REDUCTION_W32_SUM_UNORDER()
    VI_VFP_LOOP_REDUCTION_END(e64) 
    break; 
  }
  default: {
    require(0); 
    break; 
  }
}; 
