  MXU_MFP_CVT_SCALE
  ({
    ;
  },
  {
    auto acc1 = P.MU.acc_elt<int64_t>(acc1_num, 0, i, j, false);
    P.MU.acc_elt<float32_t>(accd_num, 0, i, j, true) = i64_to_f32(acc1);
  },
  {

  },
  {
    ;
  },
  {
    require(p->supports_extension(EXT_ZFH));
  },
  {
    ;
  },
  false, (P.MU.msew >= 16))