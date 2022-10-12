  MXU_MFP_CVT_SCALE
  ({
    auto acc1 = P.MU.acc_elt<int8_t>(acc1_num, 0, i, j, true);
    P.MU.acc_elt<float16_t>(accd_num, 0, i, j, true) = i32_to_f16((int32_t)acc1);
  },
  {
    auto acc1 = P.MU.acc_elt<int16_t>(acc1_num, 0, i, j, true);
    P.MU.acc_elt<float32_t>(accd_num, 0, i, j, true) = i32_to_f32((int32_t)acc1);
  },
  {
    auto acc1 = P.MU.acc_elt<int32_t>(acc1_num, 0, i, j, true);
    P.MU.acc_elt<float64_t>(accd_num, 0, i, j, true) = i64_to_f64((int32_t)acc1);
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
  true, (P.MU.msew >= 8))