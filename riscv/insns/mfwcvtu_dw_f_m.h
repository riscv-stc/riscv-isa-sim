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
    if (P.MU.mfp32 == MTYPE_FP32) {
      auto ts1 = P.MU.acc_elt<float32_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false);
      P.MU.acc_elt<uint64_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true) = f32_to_ui64(ts1, softfloat_roundingMode, true);
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
    require(p->extension_enabled('D'));
  },
  {
    ;
  },
  2, {;}, e32)