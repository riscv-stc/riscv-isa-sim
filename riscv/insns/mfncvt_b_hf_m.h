  MXU_MFP_CVT_SCALE
  ({
    ;
  },
  {
    if (P.MU.mfp16 == MTYPE_FP16) {
      auto ts1 = P.MU.acc_elt<float16_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false);
      P.MU.acc_elt<int8_t>(td_num + m, 0, i, j, mmax, nmax * 2 * amul, reg_rename, true) = f16_to_i8(ts1, softfloat_roundingMode, true);
    } else if (P.MU.mfp16 == MTYPE_BF16) {
      auto ts1 = P.MU.acc_elt<bfloat16_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false);
      P.MU.acc_elt<int8_t>(td_num + m, 0, i, j, mmax, nmax * 2 * amul, reg_rename, true) = bf16_to_i8(ts1, softfloat_roundingMode, true);
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
    ;
  },
  {
    require(p->extension_enabled(EXT_ZFH));
  },
  {
    require(p->extension_enabled(EXT_ZFH));
  },
  {
    require(p->extension_enabled('D'));
  },
  1, {;}, e16)