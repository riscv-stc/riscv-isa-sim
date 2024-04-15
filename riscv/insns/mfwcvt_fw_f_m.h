  MXU_MFP_CVT_SCALE
  ({
    ;
  },
  {
    auto rs1 = P.MU.tr_elt<float16_t>(ts1_num, 0, i, j, mmax, nmax, false);
    P.MU.tr_elt<float32_t>(td_num, 0, i, j, mmax, nmax, true) = f16_to_f32(rs1);
  },
  {
    auto rs1 = P.MU.tr_elt<float32_t>(ts1_num, 0, i, j, mmax, nmax, false);
    P.MU.tr_elt<float64_t>(td_num, 0, i, j, mmax, nmax, true) = f32_to_f64(rs1);
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