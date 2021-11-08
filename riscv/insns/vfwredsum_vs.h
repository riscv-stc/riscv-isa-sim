// // vfwredsum.vs vd, vs2, vs1
// bool is_propagate = true;
// VI_VFP_VV_LOOP_WIDE_REDUCTION
// ({
//   vd_0 = f32_add(vd_0, vs2);
// },
// {
//   vd_0 = f64_add(vd_0, vs2);
// })


#define SET  (16)
#define GET_W16_UNORDER_VS2_VALUE(vs2, idx)\
  if (idx >= vl) {\
    vs2 = f32(0);\
  } else {\
    if (insn.v_vm() == 0) {\
      bool skip = ((P.VU.elt<uint64_t>(0, (idx/64)) >> (idx%64)) & 0x1) == 0;\
      if (skip) {\
        vs2 = f32(0);\
      } else {\
        vs2 = (STATE.bf16) ? bf16_to_f32(P.VU.elt<bfloat16_t>(rs2_num,idx)) : f16_to_f32(P.VU.elt<float16_t>(rs2_num,idx));\
        is_active = true;\
      }\
    } else {\
      vs2 = (STATE.bf16) ? bf16_to_f32(P.VU.elt<bfloat16_t>(rs2_num,idx)) : f16_to_f32(P.VU.elt<float16_t>(rs2_num,idx));\
    }\
  }

#define VI_VFP_REDUCTION_W16_SUM_UNORDER()\
  float32_t vd_0  = P.VU.elt<float32_t>(rd_num, 0); \
  float32_t vs1_0 = P.VU.elt<float32_t>(rs1_num, 0);\
  float32_t vs2;   \
  float32_t sum_g0;\
  float32_t sum_g1;\
  float32_t sum_g2;\
  float32_t sum_g3;\
  float32_t sum_t0;\
  float32_t sum_t1;\
  float32_t sum_l0;\
  float32_t sum_l1;\
  float32_t sum16; \
  bool is_active = false;\
  bool is_first  = true; \
  int cnt = vl/SET;\
  int rem = vl%SET;\
  if(rem) { cnt += 1;}\
  if(cnt) { vd_0 = f32(0);}\
  for(reg_t i=0; i<cnt; i++) {\
    int idx = SET*i;\
    if (is_first) {\
      GET_W16_UNORDER_VS2_VALUE(vs2, idx)   \
      sum_g0 = f32_add(vs1_0,vs2);          \
      is_first = false;\
    } else {\
      GET_W16_UNORDER_VS2_VALUE(vs2, idx)   \
      sum_g0 = vs2;\
    }\
    GET_W16_UNORDER_VS2_VALUE(vs2, (idx+8)) \
    sum_g0 = f32_add(sum_g0, vs2);      \
    GET_W16_UNORDER_VS2_VALUE(vs2, (idx+1)) \
    sum_g1 = vs2;\
    GET_W16_UNORDER_VS2_VALUE(vs2, (idx+9)) \
    sum_g1 = f32_add(sum_g1, vs2);      \
    GET_W16_UNORDER_VS2_VALUE(vs2, (idx+2)) \
    sum_g2 = vs2;\
    GET_W16_UNORDER_VS2_VALUE(vs2, (idx+10))\
    sum_g2 = f32_add(sum_g2, vs2);      \
    GET_W16_UNORDER_VS2_VALUE(vs2, (idx+3)) \
    sum_g3 = vs2;\
    GET_W16_UNORDER_VS2_VALUE(vs2, (idx+11))\
    sum_g3 = f32_add(sum_g3, vs2);          \
    sum_t0 = f32_add(sum_g0, sum_g1);       \
    sum_t1 = f32_add(sum_g2, sum_g3);       \
    sum_l0 = f32_add(sum_t0, sum_t1);       \
    GET_W16_UNORDER_VS2_VALUE(vs2, (idx+4)) \
    sum_g0 = vs2;\
    GET_W16_UNORDER_VS2_VALUE(vs2, (idx+12))\
    sum_g0 = f32_add(sum_g0, vs2);          \
    GET_W16_UNORDER_VS2_VALUE(vs2, (idx+5)) \
    sum_g1 = vs2;\
    GET_W16_UNORDER_VS2_VALUE(vs2, (idx+13))\
    sum_g1 = f32_add(sum_g1, vs2);      \
    GET_W16_UNORDER_VS2_VALUE(vs2, (idx+6)) \
    sum_g2 = vs2;\
    GET_W16_UNORDER_VS2_VALUE(vs2, (idx+14))\
    sum_g2 = f32_add(sum_g2, vs2);          \
    GET_W16_UNORDER_VS2_VALUE(vs2, (idx+7)) \
    sum_g3 = vs2;\
    GET_W16_UNORDER_VS2_VALUE(vs2, (idx+15))\
    sum_g3 = f32_add(sum_g3, vs2);          \
    sum_t0 = f32_add(sum_g0, sum_g1);       \
    sum_t1 = f32_add(sum_g2, sum_g3);       \
    sum_l1 = f32_add(sum_t0, sum_t1);       \
    sum16  = f32_add(sum_l0, sum_l1);       \
    vd_0   = f32_add(vd_0, sum16);          \


#define GET_W32_UNORDER_VS2_VALUE(vs2, idx) \
  if (idx >= vl) {\
    vs2 = f64(0);\
  } else {\
    if (insn.v_vm() == 0) {\
      bool skip = ((P.VU.elt<uint64_t>(0, (idx/64)) >> (idx%64)) & 0x1) == 0;\
      if (skip) {\
        vs2 = f64(0);\
      } else {\
        vs2 = f32_to_f64(P.VU.elt<float32_t>(rs2_num,idx));\
        is_active = true;\
      }\
    } else {\
      vs2 = f32_to_f64(P.VU.elt<float32_t>(rs2_num,idx));\
    }\
  }

#define VI_VFP_REDUCTION_W32_SUM_UNORDER()\
  float64_t vd_0  = P.VU.elt<float64_t>(rd_num, 0); \
  float64_t vs1_0 = P.VU.elt<float64_t>(rs1_num, 0);\
  float64_t vs2;   \
  float64_t sum_g0;\
  float64_t sum_g1;\
  float64_t sum_g2;\
  float64_t sum_g3;\
  float64_t sum_t0;\
  float64_t sum_t1;\
  float64_t sum_l0;\
  float64_t sum_l1;\
  float64_t sum16; \
  bool is_active = false;\
  bool is_first  = true; \
  int cnt = vl/SET;\
  int rem = vl%SET;\
  if(rem) { cnt += 1;}\
  if(cnt) { vd_0 = f64(0);}\
  for(reg_t i=0; i<cnt; i++) {\
    int idx = SET*i;\
    if (is_first) {\
      GET_W32_UNORDER_VS2_VALUE(vs2, idx)   \
      sum_g0 = f64_add(vs1_0,vs2);          \
      is_first = false;\
    } else {\
      GET_W32_UNORDER_VS2_VALUE(vs2, idx)   \
      sum_g0 = vs2;\
    }\
    GET_W32_UNORDER_VS2_VALUE(vs2, (idx+8)) \
    sum_g0 = f64_add(sum_g0, vs2);          \
    GET_W32_UNORDER_VS2_VALUE(vs2, (idx+1)) \
    sum_g1 = vs2;\
    GET_W32_UNORDER_VS2_VALUE(vs2, (idx+9)) \
    sum_g1 = f64_add(sum_g1, vs2);          \
    GET_W32_UNORDER_VS2_VALUE(vs2, (idx+2)) \
    sum_g2 = vs2;\
    GET_W32_UNORDER_VS2_VALUE(vs2, (idx+10))\
    sum_g2 = f64_add(sum_g2, vs2);          \
    GET_W32_UNORDER_VS2_VALUE(vs2, (idx+3)) \
    sum_g3 = vs2;\
    GET_W32_UNORDER_VS2_VALUE(vs2, (idx+11))\
    sum_g3 = f64_add(sum_g3, vs2);          \
    sum_t0 = f64_add(sum_g0, sum_g1);       \
    sum_t1 = f64_add(sum_g2, sum_g3);       \
    sum_l0 = f64_add(sum_t0, sum_t1);       \
    GET_W32_UNORDER_VS2_VALUE(vs2, (idx+4)) \
    sum_g0 = vs2;\
    GET_W32_UNORDER_VS2_VALUE(vs2, (idx+12))\
    sum_g0 = f64_add(sum_g0, vs2);      \
    GET_W32_UNORDER_VS2_VALUE(vs2, (idx+5)) \
    sum_g1 = vs2;\
    GET_W32_UNORDER_VS2_VALUE(vs2, (idx+13))\
    sum_g1 = f64_add(sum_g1, vs2);      \
    GET_W32_UNORDER_VS2_VALUE(vs2, (idx+6)) \
    sum_g2 = vs2;\
    GET_W32_UNORDER_VS2_VALUE(vs2, (idx+14))\
    sum_g2 = f64_add(sum_g2, vs2);      \
    GET_W32_UNORDER_VS2_VALUE(vs2, (idx+7)) \
    sum_g3 = vs2;\
    GET_W32_UNORDER_VS2_VALUE(vs2, (idx+15))\
    sum_g3 = f64_add(sum_g3, vs2);          \
    sum_t0 = f64_add(sum_g0, sum_g1);       \
    sum_t1 = f64_add(sum_g2, sum_g3);       \
    sum_l1 = f64_add(sum_t0, sum_t1);       \
    sum16  = f64_add(sum_l0, sum_l1);       \
    vd_0   = f64_add(vd_0, sum16);          \


/*########### Begin ###########*/    
bool is_propagate = true;

VI_CHECK_REDUCTION(true) 
VI_VFP_COMMON 
require((P.VU.vsew == e16 && p->supports_extension('F')) || 
        (P.VU.vsew == e32 && p->supports_extension('D'))); 
bool is_active = false; 
switch(P.VU.vsew) {
  case e16: {
    VI_VFP_REDUCTION_W16_SUM_UNORDER() 
      set_fp_exceptions; 
    VI_VFP_LOOP_REDUCTION_END(e32)       
    break; 
  }
  case e32: {
    VI_VFP_REDUCTION_W32_SUM_UNORDER()
      set_fp_exceptions; 
    VI_VFP_LOOP_REDUCTION_END(e64) 
    break; 
  }
  default: {
    require(0); 
    break; 
  }
}; 
