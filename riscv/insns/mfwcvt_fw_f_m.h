  MXU_MFP_CVT_SCALE
  ({
    switch (P.MU.mfp8)
    {
    case MTYPE_FP8E4M3:
      if (P.MU.mfp16 == MTYPE_FP16){
        auto rs1 = P.MU.acc_elt<float8_e4m3_t>(ts1_num + m , 0, i, j, mmax, nmax * amul, false, false);
        P.MU.acc_elt<float16_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true) = f8e4m3_to_f16(rs1);
      } else if (P.MU.mfp16 == MTYPE_BF16){
        auto rs1 = P.MU.acc_elt<float8_e4m3_t>(ts1_num + m , 0, i, j, mmax, nmax * amul, false, false);
        P.MU.acc_elt<bfloat16_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true) = f8e4m3_to_bf16(rs1);
      } else {
        require(0);
      }
      break;
    case MTYPE_FP8E5M2:
      if (P.MU.mfp16 == MTYPE_FP16){
        auto rs1 = P.MU.acc_elt<float8_e5m2_t>(ts1_num + m , 0, i, j, mmax, nmax * amul, false, false);
        P.MU.acc_elt<float16_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true) = f8e5m2_to_f16(rs1);
      } else if (P.MU.mfp16 == MTYPE_BF16){
        auto rs1 = P.MU.acc_elt<float8_e5m2_t>(ts1_num + m , 0, i, j, mmax, nmax * amul, false, false);
        P.MU.acc_elt<bfloat16_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true) = f8e5m2_to_bf16(rs1);
      } else {
        require(0);
      }
      break;
    case MTYPE_FP8E3M4:
      if (P.MU.mfp16 == MTYPE_FP16){
        auto rs1 = P.MU.acc_elt<float8_e3m4_t>(ts1_num + m , 0, i, j, mmax, nmax * amul, false, false);
        P.MU.acc_elt<float16_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true) = f8e3m4_to_f16(rs1);
      } else if (P.MU.mfp16 == MTYPE_BF16){
        auto rs1 = P.MU.acc_elt<float8_e3m4_t>(ts1_num + m , 0, i, j, mmax, nmax * amul, false, false);
        P.MU.acc_elt<bfloat16_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true) = f8e3m4_to_bf16(rs1);
      } else {
        require(0);
      }
    default:
      require(0);
      break;
    }
  },
  {
    switch (P.MU.mfp16)
    {
    case MTYPE_FP16:
      if (P.MU.mfp32 == MTYPE_FP32) {
        auto rs1 = P.MU.acc_elt<float16_t>(ts1_num + m , 0, i, j, mmax, nmax * amul, false, false);
        P.MU.acc_elt<float32_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true) = f16_to_f32(rs1);
      } else if (P.MU.mfp16 == MTYPE_TFP32) {
        auto rs1 = P.MU.acc_elt<float16_t>(ts1_num + m , 0, i, j, mmax, nmax * amul, false, false);
        P.MU.acc_elt<tfloat32_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true) = f16_to_tf32(rs1);
      } else {
        require(0); 
      }
      break;
    case MTYPE_BF16:
      if (P.MU.mfp32 == MTYPE_FP32) {
        auto rs1 = P.MU.acc_elt<bfloat16_t>(ts1_num + m , 0, i, j, mmax, nmax * amul, false, false);
        P.MU.acc_elt<float32_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true) = bf16_to_f32(rs1);
      } else if (P.MU.mfp16 == MTYPE_TFP32) {
        auto rs1 = P.MU.acc_elt<bfloat16_t>(ts1_num + m , 0, i, j, mmax, nmax * amul, false, false);
        P.MU.acc_elt<tfloat32_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true) = bf16_to_tf32(rs1);
      } else {
        require(0); 
      }
    default:
      require(0);
      break;
    }
  },
  {
    if (P.MU.mfp32 == MTYPE_FP32) {
      auto rs1 = P.MU.acc_elt<float32_t>(ts1_num + m , 0, i, j, mmax, nmax * amul, false, false);
      P.MU.acc_elt<float64_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true) = f32_to_f64(rs1);
    } else if (P.MU.mfp32 == MTYPE_TFP32) {
      auto rs1 = P.MU.acc_elt<tfloat32_t>(ts1_num + m , 0, i, j, mmax, nmax * amul, false, false);
      P.MU.acc_elt<float64_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true) = tf32_to_f64(rs1);
    } else {
      require(0);
    }
    
  },
  {
    ;
  },
  {
    ;
  },
  {
    require(p->extension_enabled(EXT_ZFH));
  },
  {
    require(p->extension_enabled('D'));
  },
  {

  },
  2, {require(P.MU.msew >= 8 && P.MU.msew <= e32);}, P.MU.msew)
