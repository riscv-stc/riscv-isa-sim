  MXU_MFP_CVT_SCALE
  ({
  ;
  },
  {
    auto ts1 = P.MU.acc_elt<uint8_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false);
    if (P.MU.mfp16 == MTYPE_FP16) {
      P.MU.acc_elt<float16_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true) = ui32_to_f16((uint32_t)ts1);
    } else if (P.MU.mfp16 == MTYPE_BF16) {
      P.MU.acc_elt<bfloat16_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true) = ui32_to_bf16((uint32_t)ts1);
    }

  },
  {
    
    auto ts1 = P.MU.acc_elt<uint16_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false);
    if (P.MU.mfp32 == MTYPE_FP32) {
      P.MU.acc_elt<float32_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true) = ui32_to_f32((uint32_t)ts1);
    } else {
      P.MU.mill = true;
    }
  },
  {
    
    auto ts1 = P.MU.acc_elt<uint32_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false);
    P.MU.acc_elt<float64_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true) = ui64_to_f64((uint32_t)ts1);
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
    ;
  },
  2, {require(P.MU.msew >= 8 && P.MU.msew <= 64);}, P.MU.msew)
