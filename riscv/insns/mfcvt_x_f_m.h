MXU_MFP_CVT
(
{
    ;
},
{
    auto ts1 = P.MU.tr_elt<float16_t>(ts1_num, 0, i, j, mmax, nmax, false);
    P.MU.tr_elt<int16_t>(td_num, 0, i, j, mmax, nmax, true) = f16_to_i16(ts1, softfloat_roundingMode, true);
},
{
    auto ts1 = P.MU.tr_elt<float32_t>(ts1_num, 0, i, j, mmax, nmax, false);
    P.MU.tr_elt<int32_t>(td_num, 0, i, j, mmax, nmax, true) = f32_to_i32(ts1, softfloat_roundingMode, true);
}, 
{
    auto ts1 = P.MU.tr_elt<float64_t>(ts1_num, 0, i, j, mmax, nmax, false);
    P.MU.tr_elt<int64_t>(td_num, 0, i, j, mmax, nmax, true) = f64_to_i64(ts1, softfloat_roundingMode, true);
})