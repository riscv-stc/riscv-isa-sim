  MXU_MFP_CVT_SCALE
  ({
    ;
  },
  {
    ;
  },
  {
    reg_t des_nmax = nmax / 2;
    if (!only_one_fix_reg_sum){
      reg_sum += (tile_n - 1) / des_nmax;
      only_one_fix_reg_sum = true;
    }
    auto ts1 = P.MU.tr_elt<float32_t>(ts1_num + m, 0, i, j, mmax, nmax, false, false);
    P.MU.tr_elt<int64_t>(td_num + m + j / des_nmax, 0, i, j % des_nmax, mmax, des_nmax, reg_rename, true) = f32_to_i64(ts1, softfloat_roundingMode, true);
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
    require(p->extension_enabled('D'));
  },
  true, (P.MU.msew >= 16))