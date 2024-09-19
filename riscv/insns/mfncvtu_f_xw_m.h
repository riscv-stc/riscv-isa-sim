  MXU_MFP_CVT_SCALE
  ({
    ;
  },
  {
    auto ts1 = P.MU.acc_elt<uint16_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false);
    if (P.MU.mfp8 == MTYPE_FP8E4M3){
      P.MU.acc_elt<float8_e4m3_t>(td_num + m, 0, i, j, mmax, nmax * 2 * amul, reg_rename, true) = ui16_to_f8e4m3(ts1);
    } else if (P.MU.mfp8 == MTYPE_FP8E5M2) {
      P.MU.acc_elt<float8_e5m2_t>(td_num + m, 0, i, j, mmax, nmax * 2 * amul, reg_rename, true) = ui16_to_f8e5m2(ts1);
    } else if (P.MU.mfp8 == MTYPE_FP8E3M4) {
      P.MU.acc_elt<float8_e3m4_t>(td_num + m, 0, i, j, mmax, nmax * 2 * amul, reg_rename, true) = ui16_to_f8e3m4(ts1);
    } else {
      require(0);
    }
  },
  {
    auto ts1 = P.MU.acc_elt<uint32_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false);
    if (P.MU.mfp16 == MTYPE_FP16) {
      P.MU.acc_elt<float16_t>(td_num + m, 0, i, j, mmax, nmax * 2 * amul, reg_rename, true) = ui32_to_f16(ts1);
    } else if (P.MU.mfp16 == MTYPE_BF16) {
      P.MU.acc_elt<bfloat16_t>(td_num + m, 0, i, j, mmax, nmax * 2 * amul, reg_rename, true) = ui32_to_bf16(ts1);
    } else {
      require(0);
    }
  },
  {
    auto ts1 = P.MU.acc_elt<uint64_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false);
    if (P.MU.mfp32 == MTYPE_FP32) {
      P.MU.acc_elt<float32_t>(td_num + m, 0, i, j, mmax, nmax * 2 * amul, reg_rename, true) = ui64_to_f32(ts1);
    } else if (P.MU.mfp32 == MTYPE_TFP32) {
      P.MU.acc_elt<tfloat32_t>(td_num + m, 0, i, j, mmax, nmax * 2 * amul, reg_rename, true) = ui64_to_tf32(ts1);
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
  1, {require(P.MU.msew >= 16 && P.MU.msew <= 64);}, P.MU.msew)