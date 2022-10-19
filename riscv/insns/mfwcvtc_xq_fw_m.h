  MXU_MFP_CVT_SCALE
  ({
    ;
  },
  {
    auto acc1 = P.MU.acc_elt<float32_t>(acc1_num, 0, i, j, true);
    P.MU.acc_elt<int64_t>(accd_num, 0, i, j, true) = f32_to_i64(acc1, softfloat_roundingMode, true);
  },
  {
    ;
  },
  {
    ;
  },
  {
    require(p->supports_extension(EXT_ZFH));
  },
  {
    require(p->supports_extension('D'));
  },
  true, (P.MU.msew >= 16))