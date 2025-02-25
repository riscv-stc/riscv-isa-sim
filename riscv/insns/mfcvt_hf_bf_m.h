MXU_MFP_CVT
({
    ;
},
{
    auto ts1 = P.MU.acc_elt<bfloat16_t>(ts1_num + m , 0, i, j, mmax, nmax * amul, false, false);
    P.MU.acc_elt<float16_t>(td_num + m , 0, i, j, mmax, nmax * amul, reg_rename, true) = bf16_to_f16(ts1);
},
{
    ;
}, 
{
    ;
},
1, e16)