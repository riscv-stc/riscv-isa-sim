  MXU_MFP_CVT_SCALE
  ({
    ;
  },
  {
    if (!only_one_fix_reg_sum){
      reg_sum += (tile_n - 1) / des_nmax;
      only_one_fix_reg_sum = true;
    }
    auto rs1 = P.MU.tr_elt<float16_t>(ts1_num + m , 0, i, j, mmax, nmax, false, false);
    P.MU.tr_elt<float32_t>(td_num + m + td_num_lmul + j / des_nmax, 0, i, j % des_nmax, mmax, des_nmax, reg_rename, true) = f16_to_f32(rs1);
  },
  {
    if (!only_one_fix_reg_sum){
      reg_sum += (tile_n - 1) / des_nmax;
      only_one_fix_reg_sum = true;
    }
    auto rs1 = P.MU.tr_elt<float32_t>(ts1_num + m , 0, i, j, mmax, nmax, false, false);
    P.MU.tr_elt<float64_t>(td_num + m + td_num_lmul + j / des_nmax, 0, i, j % des_nmax, mmax, des_nmax, reg_rename, true) = f32_to_f64(rs1);
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

  },
  2, (P.MU.msew >= 16))
