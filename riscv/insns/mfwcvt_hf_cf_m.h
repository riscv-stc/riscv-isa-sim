  MXU_MFP_CVT_SCALE
  ({
  ;
  },
  {
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
        P.MU.mill = true;
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
        P.MU.mill = true;
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
        P.MU.mill = true;
      }
      break;
    default:
      P.MU.mill = true;
      break;
    }
  },
  {
    ;
  },
  {
    ;
    
  },
  {
    ;
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
  2, {;}, e8)

