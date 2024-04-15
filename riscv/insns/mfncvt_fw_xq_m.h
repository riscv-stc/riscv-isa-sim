  MXU_MFP_CVT_SCALE
  ({
    ;
  },
  {
    auto ts1 = P.MU.tr_elt<int64_t>(ts1_num, 0, i, j, mmax, nmax, false);
    P.MU.tr_elt<float32_t>(td_num, 0, i, j, mmax, nmax, true) = i64_to_f32(ts1);
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
  false, (P.MU.msew >= 16))