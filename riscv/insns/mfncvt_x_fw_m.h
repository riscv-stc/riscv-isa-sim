  MXU_MFP_CVT_SCALE
  ({
    ;
  },
    {
    auto ts1 = P.MU.tr_elt<float16_t>(ts1_num + m, 0, i, j, mmax, nmax, false, false);
    P.MU.tr_elt<int8_t>(td_num + m / 2, 0, i, j + tile_n * (m % 2), mmax, nmax * 2, reg_rename, true) = f16_to_i8(ts1, softfloat_roundingMode, true);
  },
  {
    auto ts1 = P.MU.tr_elt<float32_t>(ts1_num + m, 0, i, j, mmax, nmax, false, false);
    P.MU.tr_elt<int16_t>(td_num + m / 2, 0, i, j + tile_n * (m % 2), mmax, nmax * 2, reg_rename, true) = f32_to_i16(ts1, softfloat_roundingMode, true);
  },
  {
    auto ts1 = P.MU.tr_elt<float64_t>(ts1_num + m, 0, i, j, mmax, nmax, false, false);
    P.MU.tr_elt<int32_t>(td_num + m / 2, 0, i, j + tile_n * (m % 2), mmax, nmax * 2, reg_rename, true) = f64_to_i32(ts1, softfloat_roundingMode, true);
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
  1, (P.MU.msew >= 8))
