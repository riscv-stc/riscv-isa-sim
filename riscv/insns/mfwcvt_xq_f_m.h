  MXU_MFP_CVT_SCALE
  ({
    ;
  },
  {
    reg_t des_nmax = nmax / 4;
    auto ts1 = P.MU.tr_elt<float16_t>(ts1_num + m, 0, i, j, mmax, nmax, false);
    P.MU.tr_elt<int64_t>(td_num + m + j / des_nmax, 0, i, j % des_nmax, mmax, des_nmax, true) = f16_to_i64(ts1, softfloat_roundingMode, true);
  },
  {
    ;
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
    ;
  },
  {
    ;
  },
  true, (P.MU.msew >= 16))