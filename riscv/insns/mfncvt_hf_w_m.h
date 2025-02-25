  MXU_MFP_CVT_SCALE
  ({
  ;
  },
  {
    ;
  },
  {
    ;
  },
  {
    auto ts1 = P.MU.acc_elt<int32_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false);
    if (P.MU.mfp16 == MTYPE_FP16) {
      P.MU.acc_elt<float16_t>(td_num + m, 0, i, j, mmax, nmax * 2 * amul, reg_rename, true) = i32_to_f16(ts1);
    } else if (P.MU.mfp16 == MTYPE_BF16) {
      P.MU.acc_elt<bfloat16_t>(td_num + m, 0, i, j, mmax, nmax * 2 * amul, reg_rename, true) = i32_to_bf16(ts1);
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
    ;
  },
  {
    require(p->extension_enabled(EXT_ZFH));
  },
  {
    require(p->extension_enabled('D'));
  },
  1, {;}, e32)