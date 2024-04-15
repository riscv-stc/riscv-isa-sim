  MXU_MFP_CVT_SCALE
  ({
    ;
  },
  {
    auto ts1 = P.MU.tr_elt<float32_t>(ts1_num, 0, i, j, mmax, nmax, false);
    P.MU.tr_elt<float16_t>(td_num, 0, i, j, mmax, nmax, true) = f32_to_f16(ts1);
  },
  {
    auto ts1 = P.MU.tr_elt<float64_t>(ts1_num, 0, i, j, mmax, nmax, false);
    P.MU.tr_elt<float32_t>(td_num, 0, i, j, mmax, nmax, true) = f64_to_f32(ts1);
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