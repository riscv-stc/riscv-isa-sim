MXU_MFP_CVT
(
{
    ;
},
{
    ;
},
{
    ;
}, 
{
    if (P.MU.mfp64) {
        auto ts1 = P.MU.acc_elt<float64_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false);
        P.MU.acc_elt<uint64_t>(td_num + m, 0, i, j, mmax, nmax * amul, reg_rename, true) = f64_to_ui64(ts1, softfloat_roundingMode, true);
    } else {
        require(0);
    }
}, 1, e64)