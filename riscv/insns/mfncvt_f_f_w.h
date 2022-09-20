  MXU_VFP_CVT_SCALE
  ({
    ;
  },
  {
    auto ts1 = P.MU.acc_elt<float32_t>(ts1_num, 0, i, j, true);
    P.MU.acc_elt<float16_t>(accd_num, 0, i, j, true) = f32_to_f16(ts1);
  },
  {
    auto ts1 = P.MU.acc_elt<float64_t>(ts1_num, 0, i, j, true);
    P.MU.acc_elt<float32_t>(accd_num, 0, i, j, true) = f64_to_f32(ts1);
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
  false, (P.MU.msew >= 16))