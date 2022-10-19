MXU_MFP_CVT
(
{
    ;
},
{
    auto acc1 = P.MU.acc_elt<int16_t>(acc1_num, 0, i, j, true);
    P.MU.acc_elt<float16_t>(accd_num, 0, i, j, true) = i32_to_f16((int32_t)acc1);
},
{
    auto acc1 = P.MU.acc_elt<int32_t>(acc1_num, 0, i, j, true);
    P.MU.acc_elt<float32_t>(accd_num, 0, i, j, true) = i32_to_f32(acc1);
},
{
    auto acc1 = P.MU.acc_elt<int64_t>(acc1_num, 0, i, j, true);
    P.MU.acc_elt<float64_t>(accd_num, 0, i, j, true) = i64_to_f64(acc1);
})
