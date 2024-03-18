  MXU_MFP_CVT_SCALE
  ({
    ;
  },
  {
    auto acc1 = P.MU.acc_elt<float32_t>(acc1_num, 0, i, j, false);
    P.MU.acc_elt<int64_t>(accd_num, 0, i, j, true) = f32_to_i64(acc1, softfloat_roundingMode, true);
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