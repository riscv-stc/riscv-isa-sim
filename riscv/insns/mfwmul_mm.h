MI_VFP_MM_LOOP(
{
    if (!only_one_fix_reg_sum){
      reg_sum += (tile_n - 1) / des_nmax;
      only_one_fix_reg_sum = true;
    }
    float32_t &td_w = P.MU.tr_elt<float32_t>(td_num + m + td_num_lmul + j / des_nmax, 0, i, j % des_nmax, mmax, des_nmax, reg_rename, true); \
    td_w = f32_mul(f16_to_f32(ts1), f16_to_f32(ts2));
} ,
{
    if (!only_one_fix_reg_sum){
      reg_sum += (tile_n - 1) / des_nmax;
      only_one_fix_reg_sum = true;
    }
    float64_t &td_w = P.MU.tr_elt<float64_t>(td_num + m + td_num_lmul + j / des_nmax, 0, i, j % des_nmax, mmax, des_nmax, reg_rename, true);
    td_w = f64_mul(f32_to_f64(ts1), f32_to_f64(ts2));
},
{
 ;   
}, 2)
