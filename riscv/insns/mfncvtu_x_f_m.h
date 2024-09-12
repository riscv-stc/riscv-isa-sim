MXU_MFP_CVT
(
{
    if (P.MU.mfp8 == MTYPE_FP8E4M3) {
        auto ts1 = P.MU.acc_elt<float8_e4m3_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false);
        P.MU.acc_elt<uint16_t>(td_num + m, 0, i, j, mmax, nmax * amul, reg_rename, true) = f8e4m3_to_ui8(ts1, softfloat_roundingMode, true);
    } else if (P.MU.mfp8 == MTYPE_FP8E5M2) {
        auto ts1 = P.MU.acc_elt<float8_e5m2_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false);
        P.MU.acc_elt<uint16_t>(td_num + m, 0, i, j, mmax, nmax * amul, reg_rename, true) = f8e5m2_to_ui8(ts1, softfloat_roundingMode, true);
    } else if (P.MU.mfp8 == MTYPE_FP8E3M4) {
        auto ts1 = P.MU.acc_elt<float8_e3m4_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false);
        P.MU.acc_elt<uint16_t>(td_num + m, 0, i, j, mmax, nmax * amul, reg_rename, true) = f8e3m4_to_ui8(ts1, softfloat_roundingMode, true);
    } else {
        require(0);
    }
},
{
    if (P.MU.mfp16 == MTYPE_FP16) {
        auto ts1 = P.MU.acc_elt<float16_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false);
        P.MU.acc_elt<uint16_t>(td_num + m, 0, i, j, mmax, nmax * amul, reg_rename, true) = f16_to_ui16(ts1, softfloat_roundingMode, true);
    } else if (P.MU.mfp16 == MTYPE_BF16) {
        auto ts1 = P.MU.acc_elt<bfloat16_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false);
        P.MU.acc_elt<uint16_t>(td_num + m, 0, i, j, mmax, nmax * amul, reg_rename, true) = bf16_to_ui16(ts1, softfloat_roundingMode, true);
    } else {
        require(0);
    }
},
{
    if (P.MU.mfp32 == MTYPE_FP32) {
        auto ts1 = P.MU.acc_elt<float32_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false);
        P.MU.acc_elt<uint32_t>(td_num + m, 0, i, j, mmax, nmax * amul, reg_rename, true) = f32_to_ui32(ts1, softfloat_roundingMode, true);
    } else if (P.MU.mfp32 == MTYPE_TFP32) {
        auto ts1 = P.MU.acc_elt<tfloat32_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false);
        P.MU.acc_elt<uint32_t>(td_num + m, 0, i, j, mmax, nmax * amul, reg_rename, true) = tf32_to_ui32(ts1, softfloat_roundingMode, true);
    } else {
        require(0);
    }
}, 
{
    if (P.MU.mfp64) {
        auto ts1 = P.MU.acc_elt<float64_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false);
        P.MU.acc_elt<uint64_t>(td_num + m, 0, i, j, mmax, nmax * amul, reg_rename, true) = f64_to_ui64(ts1, softfloat_roundingMode, true);
    } else {
        require(0);
    }
}, 1, P.MU.msew)