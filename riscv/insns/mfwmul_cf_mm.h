MI_VFP_MM_LOOP(
{
  if (P.MU.mfp16 == MTYPE_FP16){
    float16_t &td_w = P.MU.acc_elt<float16_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true); \
    td_w = f16_mul(f8e4m3_to_f16(ts1), f8e4m3_to_f16(ts2));
  }
  else if (P.MU.mfp16 == MTYPE_BF16) {
    bfloat16_t &td_w = P.MU.acc_elt<bfloat16_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true); \
    td_w = bf16_mul(f8e4m3_to_bf16(ts1), f8e4m3_to_bf16(ts2));
  }
  else {
    require(0);
  }
},
{
  if (P.MU.mfp16 == MTYPE_FP16){
    float16_t &td_w = P.MU.acc_elt<float16_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true); \
    td_w = f16_mul(f8e5m2_to_f16(ts1), f8e5m2_to_f16(ts2));
  }
  else if (P.MU.mfp16 == MTYPE_BF16) {
    bfloat16_t &td_w = P.MU.acc_elt<bfloat16_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true); \
    td_w = bf16_mul(f8e5m2_to_bf16(ts1), f8e5m2_to_bf16(ts2));
  }
  else {
    require(0);
  }
},
{
  if (P.MU.mfp16 == MTYPE_FP16){
    float16_t &td_w = P.MU.acc_elt<float16_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true); \
    td_w = f16_mul(f8e4m3_to_f16(ts1), f8e4m3_to_f16(ts2));
  }
  else if (P.MU.mfp16 == MTYPE_BF16) {
    bfloat16_t &td_w = P.MU.acc_elt<bfloat16_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true); \
    td_w = bf16_mul(f8e3m4_to_bf16(ts1), f8e3m4_to_bf16(ts2));
  }
  else {
    require(0);
  }
},
{
    if (P.MU.mfp32 == MTYPE_FP32) {
      float32_t &td_w = P.MU.acc_elt<float32_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true); \
      td_w = f32_mul(f16_to_f32(ts1), f16_to_f32(ts2));
    } else if (P.MU.mfp32 == MTYPE_TFP32){
      tfloat32_t &td_w = P.MU.acc_elt<tfloat32_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true); \
      td_w = tf32_mul(f16_to_tf32(ts1), f16_to_tf32(ts2));
    } else {
      require(0);
    }
} ,
{
    if (P.MU.mfp32 == MTYPE_FP32) {
      float32_t &td_w = P.MU.acc_elt<float32_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true); \
      td_w = f32_mul(bf16_to_f32(ts1), bf16_to_f32(ts2));
    } else if (P.MU.mfp32 == MTYPE_TFP32){
      tfloat32_t &td_w = P.MU.acc_elt<tfloat32_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true); \
      td_w = tf32_mul(bf16_to_tf32(ts1), bf16_to_tf32(ts2));
    } else {
      require(0);
    }
} ,
{
    float64_t &td_w = P.MU.acc_elt<float64_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true);
    td_w = f64_mul(f32_to_f64(ts1), f32_to_f64(ts2));
},
{
    float64_t &td_w = P.MU.acc_elt<float64_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true);
    td_w = f64_mul(tf32_to_f64(ts1), tf32_to_f64(ts2));
},
{
 ;   
}, 2, e8)
