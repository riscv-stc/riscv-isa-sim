MXU_MFP_CVT
(
{
    ;
},
{
    auto acc1 = P.MU.tr_elt<float16_t>(ts1_num + m, 0, i, j, mmax, nmax, false, false);
    P.MU.tr_elt<int16_t>(td_num + m, 0, i, j, mmax, nmax, reg_rename, true) = f16_to_i16(acc1, softfloat_roundingMode, true);
},
{
    auto acc1 = P.MU.tr_elt<float32_t>(ts1_num + m, 0, i, j, mmax, nmax, false, false);
    P.MU.tr_elt<int32_t>(td_num + m, 0, i, j, mmax, nmax, reg_rename, true) = f32_to_i32(acc1, softfloat_roundingMode, true);
}, 
{
    auto acc1 = P.MU.tr_elt<float64_t>(ts1_num + m, 0, i, j, mmax, nmax, false, false);
    P.MU.tr_elt<int64_t>(td_num + m, 0, i, j, mmax, nmax, reg_rename, true) = f64_to_i64(acc1, softfloat_roundingMode, true);
}
)