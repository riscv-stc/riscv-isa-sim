// // vfredsum: vd[0] =  sum( vs2[*] , vs1[0] )
// bool is_propagate = true;
// VI_VFP_VV_LOOP_REDUCTION
// ({
//   vd_0 = bf16_add(vd_0, vs2);
// },
// {
//   vd_0 = f16_add(vd_0, vs2);
// },
// {
//   vd_0 = f32_add(vd_0, vs2);
// },
// {
//   vd_0 = f64_add(vd_0, vs2);
// })


#define SET  (16)
#define GET_UNORDER_VS2_VALUE(vs2, idx, width)\
  if (idx >= vl) {\
    vs2 = f##width(0);\
  } else {\
    if (insn.v_vm() == 0) {\
      bool skip = ((P.VU.elt<uint64_t>(0, (idx/64)) >> (idx%64)) & 0x1) == 0;\
      if (skip) {\
        vs2 = f##width(0);\
      } else {\
        vs2 = P.VU.elt<float##width##_t>(rs2_num, idx);\
        is_active = true;\
      }\
    } else {\
      vs2 = P.VU.elt<float##width##_t>(rs2_num, idx);\
    }\
  }

#define VI_VFP_REDUCTION_SUM_UNORDER(width)\
  float##width##_t vd_0  = P.VU.elt<float##width##_t>(rd_num, 0); \
  float##width##_t vs1_0 = P.VU.elt<float##width##_t>(rs1_num, 0);\
  float##width##_t vs2;    \
  float##width##_t sum_g0; \
  float##width##_t sum_g1; \
  float##width##_t sum_g2; \
  float##width##_t sum_g3; \
  float##width##_t sum_t0; \
  float##width##_t sum_t1; \
  float##width##_t sum_l0; \
  float##width##_t sum_l1; \
  float##width##_t sum16;  \
  bool is_active = false;  \
  bool is_first  = true;   \
  int cnt = vl/SET;\
  int rem = vl%SET;\
  if(rem) { cnt += 1;}\
  if(cnt) { vd_0 = f##width(0);}\
  for(reg_t i=0; i<cnt; i++) {\
    is_active = true;\
    int idx = SET*i;\
    if (is_first) {\
      GET_UNORDER_VS2_VALUE(vs2, idx, width)   \
      sum_g0 = f##width##_add(vs1_0,vs2);      \
      is_first = false;\
    } else {\
      GET_UNORDER_VS2_VALUE(vs2, idx, width)   \
      sum_g0 = vs2;\
    }\
    GET_UNORDER_VS2_VALUE(vs2, (idx+1), width) \
    sum_g0 = f##width##_add(sum_g0, vs2);      \
    GET_UNORDER_VS2_VALUE(vs2, (idx+2), width) \
    sum_g1 = vs2;\
    GET_UNORDER_VS2_VALUE(vs2, (idx+3), width) \
    sum_g1 = f##width##_add(sum_g1, vs2);      \
    GET_UNORDER_VS2_VALUE(vs2, (idx+4), width) \
    sum_g2 = vs2;\
    GET_UNORDER_VS2_VALUE(vs2, (idx+5), width) \
    sum_g2 = f##width##_add(sum_g2, vs2);      \
    GET_UNORDER_VS2_VALUE(vs2, (idx+6), width) \
    sum_g3 = vs2;\
    GET_UNORDER_VS2_VALUE(vs2, (idx+7), width) \
    sum_g3 = f##width##_add(sum_g3, vs2);      \
    sum_t0 = f##width##_add(sum_g0, sum_g1);   \
    sum_t1 = f##width##_add(sum_g2, sum_g3);   \
    sum_l0 = f##width##_add(sum_t0, sum_t1);   \
    GET_UNORDER_VS2_VALUE(vs2, (idx+8), width) \
    sum_g0 = vs2;\
    GET_UNORDER_VS2_VALUE(vs2, (idx+9), width) \
    sum_g0 = f##width##_add(sum_g0, vs2);      \
    GET_UNORDER_VS2_VALUE(vs2, (idx+10), width)\
    sum_g1 = vs2;\
    GET_UNORDER_VS2_VALUE(vs2, (idx+11), width)\
    sum_g1 = f##width##_add(sum_g1, vs2);      \
    GET_UNORDER_VS2_VALUE(vs2, (idx+12), width)\
    sum_g2 = vs2;\
    GET_UNORDER_VS2_VALUE(vs2, (idx+13), width)\
    sum_g2 = f##width##_add(sum_g2, vs2);      \
    GET_UNORDER_VS2_VALUE(vs2, (idx+14), width)\
    sum_g3 = vs2;\
    GET_UNORDER_VS2_VALUE(vs2, (idx+15), width)\
    sum_g3 = f##width##_add(sum_g3, vs2);      \
    sum_t0 = f##width##_add(sum_g0, sum_g1);   \
    sum_t1 = f##width##_add(sum_g2, sum_g3);   \
    sum_l1 = f##width##_add(sum_t0, sum_t1);   \
    sum16  = f##width##_add(sum_l0, sum_l1);   \
    vd_0   = f##width##_add(vd_0, sum16);      \
  

#define GET_UNORDER_VS2_VALUE_BF(vs2, idx)\
  if (idx >= vl) {\
    vs2 = bf16(0);\
  } else {\
    if (insn.v_vm() == 0) {\
      bool skip = ((P.VU.elt<uint64_t>(0, (idx/64)) >> (idx%64)) & 0x1) == 0;\
      if (skip) {\
        vs2 = bf16(0);\
      } else {\
        vs2 = P.VU.elt<bfloat16_t>(rs2_num, idx);\
        is_active = true;\
      }\
    } else {\
      vs2 = P.VU.elt<bfloat16_t>(rs2_num, idx);\
    }\
  }

#define VI_VFP_REDUCTION_SUM_UNORDER_BF()\
  bfloat16_t vd_0  = P.VU.elt<bfloat16_t>(rd_num, 0); \
  bfloat16_t vs1_0 = P.VU.elt<bfloat16_t>(rs1_num, 0);\
  bfloat16_t vs2;    \
  bfloat16_t sum_g0; \
  bfloat16_t sum_g1; \
  bfloat16_t sum_g2; \
  bfloat16_t sum_g3; \
  bfloat16_t sum_t0; \
  bfloat16_t sum_t1; \
  bfloat16_t sum_l0; \
  bfloat16_t sum_l1; \
  bfloat16_t sum16;  \
  bool is_active = false;\
  bool is_first  = true; \
  int cnt = vl/SET;\
  int rem = vl%SET;\
  if(rem) { cnt += 1;}\
  if(cnt) { vd_0 = bf16(0);/*vl>0*/ }\
  for(reg_t i=0; i<cnt; i++) {\
    int idx = SET*i;\
    if (is_first) {\
      GET_UNORDER_VS2_VALUE_BF(vs2, idx)   \
      sum_g0 = bf16_add(vs1_0, vs2);       \
      is_first = false;\
    } else {\
      GET_UNORDER_VS2_VALUE_BF(vs2, idx)   \
      sum_g0 = vs2;\
    }\
    GET_UNORDER_VS2_VALUE_BF(vs2, (idx+1)) \
    sum_g0 = bf16_add(sum_g0, vs2);\
    GET_UNORDER_VS2_VALUE_BF(vs2, (idx+2)) \
    sum_g1 = vs2;\
    GET_UNORDER_VS2_VALUE_BF(vs2, (idx+3)) \
    sum_g1 = bf16_add(sum_g1, vs2);\
    GET_UNORDER_VS2_VALUE_BF(vs2, (idx+4)) \
    sum_g2 = vs2;\
    GET_UNORDER_VS2_VALUE_BF(vs2, (idx+5)) \
    sum_g2 = bf16_add(sum_g2, vs2);\
    GET_UNORDER_VS2_VALUE_BF(vs2, (idx+6)) \
    sum_g3 = vs2;\
    GET_UNORDER_VS2_VALUE_BF(vs2, (idx+7)) \
    sum_g3 = bf16_add(sum_g3, vs2);        \
    sum_t0 = bf16_add(sum_g0, sum_g1);     \
    sum_t1 = bf16_add(sum_g2, sum_g3);     \
    sum_l0 = bf16_add(sum_t0, sum_t1);     \
    GET_UNORDER_VS2_VALUE_BF(vs2, (idx+8)) \
    sum_g0 = vs2;\
    GET_UNORDER_VS2_VALUE_BF(vs2, (idx+9)) \
    sum_g0 = bf16_add(sum_g0, vs2);        \
    GET_UNORDER_VS2_VALUE_BF(vs2, (idx+10))\
    sum_g1 = vs2;\
    GET_UNORDER_VS2_VALUE_BF(vs2, (idx+11))\
    sum_g1 = bf16_add(sum_g1, vs2);        \
    GET_UNORDER_VS2_VALUE_BF(vs2, (idx+12))\
    sum_g2 = vs2;\
    GET_UNORDER_VS2_VALUE_BF(vs2, (idx+13))\
    sum_g2 = bf16_add(sum_g2, vs2);        \
    GET_UNORDER_VS2_VALUE_BF(vs2, (idx+14))\
    sum_g3 = vs2;\
    GET_UNORDER_VS2_VALUE_BF(vs2, (idx+15))\
    sum_g3 = bf16_add(sum_g3, vs2);        \
    sum_t0 = bf16_add(sum_g0, sum_g1);     \
    sum_t1 = bf16_add(sum_g2, sum_g3);     \
    sum_l1 = bf16_add(sum_t0, sum_t1);     \
    sum16  = bf16_add(sum_l0, sum_l1);     \
    vd_0   = bf16_add(vd_0, sum16);        \


/*########### Begin ###########*/
bool is_propagate = true;

VI_CHECK_REDUCTION(false);
VI_VFP_COMMON
  switch(P.VU.vsew) { 
    case e16: {
      if (STATE.bf16) {
        VI_VFP_REDUCTION_SUM_UNORDER_BF()  
          set_fp_exceptions; //note
        VI_VFP_LOOP_REDUCTION_END(e16) 
      } else {
        VI_VFP_REDUCTION_SUM_UNORDER(16)  
          set_fp_exceptions; 
        VI_VFP_LOOP_REDUCTION_END(e16) 
      }
      break; 
    }
    case e32: {
      VI_VFP_REDUCTION_SUM_UNORDER(32) 
        set_fp_exceptions; 
      VI_VFP_LOOP_REDUCTION_END(e32) 
      break; 
    }
    case e64: {
      VI_VFP_REDUCTION_SUM_UNORDER(64) 
        set_fp_exceptions; 
      VI_VFP_LOOP_REDUCTION_END(e64) 
      break; 
    }
    default: {
      require(0); 
      break;
    }       
  }; 
