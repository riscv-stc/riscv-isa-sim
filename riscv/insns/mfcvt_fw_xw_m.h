MXU_MFP_CVT
({
    ;
},
{
    auto ts1 = P.MU.tr_elt<int16_t>(ts1_num + m , 0, i, j, mmax, nmax, false);
    P.MU.tr_elt<float16_t>(td_num + m , 0, i, j, mmax, nmax, true) = i32_to_f16((int32_t)ts1);
},
{
    auto ts1 = P.MU.tr_elt<int32_t>(ts1_num + m, 0, i, j, mmax, nmax, false);
    P.MU.tr_elt<float32_t>(td_num + m, 0, i, j, mmax, nmax, true) = i32_to_f32(ts1);
}, 
{
    auto ts1 = P.MU.tr_elt<int64_t>(ts1_num + m, 0, i, j, mmax, nmax, false);
    P.MU.tr_elt<float64_t>(td_num + m, 0, i, j, mmax, nmax, true) = i64_to_f64(ts1);
}
)