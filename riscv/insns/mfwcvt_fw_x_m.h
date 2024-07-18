  MXU_MFP_CVT_SCALE
  ({
    if (!only_one_fix_reg_sum){
      reg_sum += (tile_n - 1) / des_nmax;
      only_one_fix_reg_sum = true;
    }
    auto ts1 = P.MU.tr_elt<int8_t>(ts1_num + m, 0, i, j, mmax, nmax, false, false);
    P.MU.tr_elt<float16_t>(td_num + m + td_num_lmul + j / des_nmax, 0, i, j % des_nmax, mmax, des_nmax, reg_rename, true) = i32_to_f16((int32_t)ts1);
  },
  {
    if (!only_one_fix_reg_sum){
      reg_sum += (tile_n - 1) / des_nmax;
      only_one_fix_reg_sum = true;
    }
    auto ts1 = P.MU.tr_elt<int16_t>(ts1_num + m, 0, i, j, mmax, nmax, false, false);
    P.MU.tr_elt<float32_t>(td_num + m + td_num_lmul + j / des_nmax, 0, i, j % des_nmax, mmax, des_nmax, reg_rename, true) = i32_to_f32((int32_t)ts1);
  },
  {
    if (!only_one_fix_reg_sum){
      reg_sum += (tile_n - 1) / des_nmax;
      only_one_fix_reg_sum = true;
    }
    auto ts1 = P.MU.tr_elt<int32_t>(ts1_num + m, 0, i, j, mmax, nmax, false, false);
    P.MU.tr_elt<float64_t>(td_num + m + td_num_lmul + j / des_nmax, 0, i, j % des_nmax, mmax, des_nmax, reg_rename, true) = i64_to_f64((int32_t)ts1);
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
  {
    ;
  },
  2, (P.MU.msew >= 8))