  MXU_MFP_CVT_SCALE
  ({
    ;
  },
  {
    auto ts1 = P.MU.tr_elt<float32_t>(ts1_num, 0, i, j, mmax, nmax, false);
    P.MU.tr_elt<int64_t>(td_num, 0, i, j, mmax, nmax, true) = f32_to_i64(ts1, softfloat_roundingMode, true);
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
  true, (P.MU.msew >= 16))