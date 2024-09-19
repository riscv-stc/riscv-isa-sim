MXU_MFP_CVT
(
{
    auto ts1 = P.MU.acc_elt<int8_t>(ts1_num + m , 0, i, j, mmax, nmax * amul, false, false);
    switch (P.MU.mfp8)
    {
    case MTYPE_FP8E4M3:
      P.MU.acc_elt<float8_e4m3_t>(td_num + m, 0, i, j, mmax, nmax * amul, reg_rename, true) = i8_to_f8e4m3(ts1);
      break;
    case MTYPE_FP8E5M2:
      P.MU.acc_elt<float8_e5m2_t>(td_num + m, 0, i, j, mmax, nmax * amul, reg_rename, true) = i8_to_f8e5m2(ts1);
      break;
    case MTYPE_FP8E3M4:
      P.MU.acc_elt<float8_e3m4_t>(td_num + m, 0, i, j, mmax, nmax * amul, reg_rename, true) = i8_to_f8e3m4(ts1);
      break;
    default:
      require(0);
      break;
    }
},
{
    auto ts1 = P.MU.acc_elt<int16_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false);
    if (P.MU.mfp16 == MTYPE_FP16){
        P.MU.acc_elt<float16_t>(td_num + m, 0, i, j, mmax, nmax * amul, reg_rename, true) = i32_to_f16((int32_t)ts1);
    } else if (P.MU.mfp16 == MTYPE_BF16){
        P.MU.acc_elt<bfloat16_t>(td_num + m, 0, i, j, mmax, nmax * amul, reg_rename, true) = i32_to_bf16((int32_t)ts1);
    } else {
        require(0);
    }
},
{
    auto ts1 = P.MU.acc_elt<int32_t>(ts1_num + m, 0, i, j, mmax, nmax * amul,  false, false);
    if (P.MU.mfp32 == MTYPE_FP32) {
        P.MU.acc_elt<float32_t>(td_num + m, 0, i, j, mmax, nmax * amul, reg_rename, true) = i32_to_f32(ts1);
      } else if (P.MU.mfp16 == MTYPE_TFP32) {
        P.MU.acc_elt<tfloat32_t>(td_num + m, 0, i, j, mmax, nmax * amul, reg_rename, true) = i32_to_tf32(ts1);
      } else {
        require(0); 
      } 
},
{
    if (P.MU.mfp64){
        auto ts1 = P.MU.acc_elt<int64_t>(ts1_num + m, 0, i, j, mmax, nmax * amul,  false, false);
        P.MU.acc_elt<float64_t>(td_num + m, 0, i, j, mmax, nmax * amul, reg_rename, true) = i64_to_f64(ts1);
    } else {
      require(0);
    }
    
}, 1, P.MU.msew)
