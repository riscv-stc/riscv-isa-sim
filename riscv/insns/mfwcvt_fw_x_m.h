  MXU_MFP_CVT_SCALE
  ({
    auto ts1 = P.MU.tr_elt<int8_t>(ts1_num, 0, i, j, mmax, nmax, false);
    P.MU.tr_elt<float16_t>(td_num, 0, i, j, mmax, nmax, true) = i32_to_f16((int32_t)ts1);
  },
  {
    auto ts1 = P.MU.tr_elt<int16_t>(ts1_num, 0, i, j, mmax, nmax, false);
    P.MU.tr_elt<float32_t>(td_num, 0, i, j, mmax, nmax, true) = i32_to_f32((int32_t)ts1);
  },
  {
    auto ts1 = P.MU.tr_elt<int32_t>(ts1_num, 0, i, j, mmax, nmax, false);
    P.MU.tr_elt<float64_t>(td_num, 0, i, j, mmax, nmax, true) = i64_to_f64((int32_t)ts1);
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
  true, (P.MU.msew >= 8))