  MXU_MFP_CVT_SCALE
  ({
    ;
  },
  {
    auto acc1 = P.MU.acc_elt<int32_t>(acc1_num, 0, i, j, false);
    P.MU.acc_elt<float16_t>(accd_num, 0, i, j, true) = i32_to_f16(acc1);
  },
  {
    auto acc1 = P.MU.acc_elt<int64_t>(acc1_num, 0, i, j, false);
    P.MU.acc_elt<float32_t>(accd_num, 0, i, j, true) = i64_to_f32(acc1);
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