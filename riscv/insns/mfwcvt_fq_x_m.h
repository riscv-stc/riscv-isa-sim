  MXU_MFP_CVT_SCALE
  ({
    auto ts1 = P.MU.acc_elt<int8_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false);

    if (P.MU.mfp32 == MTYPE_FP32) {
        P.MU.acc_elt<float32_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true) = i32_to_f32((int32_t)ts1);
    } else if (P.MU.mfp32 == MTYPE_TFP32) {
        P.MU.acc_elt<tfloat32_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true) = i32_to_tf32((int32_t)ts1);
    } else {
        require(0);
    }
  },
  {
    auto ts1 = P.MU.acc_elt<int16_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false);

    if (!P.MU.mfp64) {
        require(0);
    } 
    P.MU.acc_elt<float64_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true) = i32_to_f64((int32_t)ts1);
  },
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
    require(p->extension_enabled(EXT_ZFH));
  },
  {
    require(p->extension_enabled('D'));
  },
  {
    ;
  },
  4, {require(P.MU.msew >= 8 && P.MU.msew <= e16);}, P.MU.msew)