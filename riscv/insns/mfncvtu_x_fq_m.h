  MXU_MFP_CVT_SCALE
  ({
    ;
  },
  {
    if (P.MU.mfp16 == MTYPE_FP16){
        auto ts1 = P.MU.acc_elt<float16_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false);
        auto temp = f16_to_ui4(ts1, softfloat_roundingMode, true);
        auto &accd = P.MU.acc_elt<uint8_t>(td_num + m, 0, i, j / 2 , mmax, nmax * amul * 2, reg_rename, true);
        accd = j % 2 ? ((temp & 0xF) << 4) | accd : temp & 0xF;
    } else if (P.MU.mfp16 == MTYPE_BF16) {
        auto ts1 = P.MU.acc_elt<bfloat16_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false);
        auto temp = bf16_to_ui4(ts1, softfloat_roundingMode, true);
        auto &accd = P.MU.acc_elt<uint8_t>(td_num + m, 0, i, j / 2 , mmax, nmax * amul * 2, reg_rename, true);
        accd = j % 2 ? ((temp & 0xF) << 4) | accd : temp & 0xF;
    } else {
        require(0);
    }
  },
  { 
    if (P.MU.mfp32 == MTYPE_FP32){
        auto ts1 = P.MU.acc_elt<float32_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false);
        P.MU.acc_elt<uint8_t>(td_num + m, 0, i, j, mmax, nmax * amul * 4, reg_rename, true) = f32_to_ui8(ts1, softfloat_roundingMode, true);
    } else if (P.MU.mfp32 == MTYPE_TFP32) {
        auto ts1 = P.MU.acc_elt<tfloat32_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false);
        P.MU.acc_elt<uint8_t>(td_num + m, 0, i, j, mmax, nmax * amul * 4, reg_rename, true) = tf32_to_ui8(ts1, softfloat_roundingMode, true);
    } else {
        require(0);
    }
  },
  {
    if (!P.MU.mfp64)
        require(0);
    
    auto ts1 = P.MU.acc_elt<float64_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false);
    P.MU.acc_elt<uint16_t>(td_num + m, 0, i, j, mmax, nmax * amul * 4, reg_rename, true) = f64_to_ui16(ts1, softfloat_roundingMode, true);
  },
  {
    ;
  },
  {
    require(p->extension_enabled(EXT_ZFH));
  },
  {
    require(p->extension_enabled('D'));
  },
  {
    ;
  },
  1, {require(P.MU.msew >= e16 && P.MU.msew <= e64);}, P.MU.msew)