MXU_MFP_CVT
(
{
    ;
},
{
    auto acc1 = P.MU.acc_elt<float16_t>(acc1_num, 0, i, j, false);
    P.MU.acc_elt<int16_t>(accd_num, 0, i, j, true) = f16_to_i16(acc1, softfloat_roundingMode, true);
},
{
    auto acc1 = P.MU.acc_elt<float32_t>(acc1_num, 0, i, j, false);
    P.MU.acc_elt<int32_t>(accd_num, 0, i, j, true) = f32_to_i32(acc1, softfloat_roundingMode, true);
}, 
{
    auto acc1 = P.MU.acc_elt<float64_t>(acc1_num, 0, i, j, false);
    P.MU.acc_elt<int64_t>(accd_num, 0, i, j, true) = f64_to_i64(acc1, softfloat_roundingMode, true);
})