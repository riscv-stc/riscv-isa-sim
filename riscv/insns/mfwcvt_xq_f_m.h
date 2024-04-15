  MXU_MFP_CVT_SCALE
  ({
    ;
  },
  {
    auto ts1 = P.MU.tr_elt<float16_t>(ts1_num, 0, i, j, mmax, nmax, false);
    P.MU.tr_elt<int64_t>(td_num, 0, i, j, mmax, nmax, true) = f16_to_i64(ts1, softfloat_roundingMode, true);
  },
  {

  },
  {
    ;
  },
  {
    require(p->extension_enabled(EXT_ZFH));
  },
  {
    ;
  },
  true, (P.MU.msew >= 16))