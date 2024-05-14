  MXU_MFP_CVT_SCALE
  ({
    ;
  },
  {
    ;
  },
  {
    auto ts1 = P.MU.tr_elt<int32_t>(ts1_num + m, 0, i, j, mmax, nmax, false);
    P.MU.tr_elt<float16_t>(td_num + m, 0, i, j, mmax, nmax * 2, true) = i32_to_f16(ts1);
  },
  {
    auto ts1 = P.MU.tr_elt<int64_t>(ts1_num + m, 0, i, j, mmax, nmax, false);
    P.MU.tr_elt<float32_t>(td_num + m, 0, i, j, mmax, nmax * 2, true) = i64_to_f32(ts1);
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
  false, (P.MU.msew >= 16))
