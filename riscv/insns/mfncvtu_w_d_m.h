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
    ;
  },
  {
    if (!P.MU.mfp64)
      require(0);
    auto ts1 = P.MU.acc_elt<float64_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false);
    P.MU.acc_elt<uint32_t>(td_num + m, 0, i, j, mmax, nmax * 2 * amul, reg_rename, true) = f64_to_ui32(ts1, softfloat_roundingMode, true);
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
  1, {;}, e64)