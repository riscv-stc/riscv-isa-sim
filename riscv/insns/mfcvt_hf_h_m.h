MXU_MFP_CVT
(
{
 ;
},
{
    auto ts1 = P.MU.acc_elt<int16_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false);
    if (P.MU.mfp16 == MTYPE_FP16){
        P.MU.acc_elt<float16_t>(td_num + m, 0, i, j, mmax, nmax * amul, reg_rename, true) = i32_to_f16((int32_t)ts1);
    } else if (P.MU.mfp16 == MTYPE_BF16){
        P.MU.acc_elt<bfloat16_t>(td_num + m, 0, i, j, mmax, nmax * amul, reg_rename, true) = i32_to_bf16((int32_t)ts1);
    } else {
        require(0);
    }
},
{
   ;
},
{
   ;
}, 1, e16)
