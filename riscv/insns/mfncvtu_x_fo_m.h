  MXU_MFP_CVT_SCALE
  ({
  ;
  },
  {
    ;
  },
  {
    ;
  },
  { 
    if (P.MU.mfp32 == MTYPE_FP32){
        auto ts1 = P.MU.acc_elt<float32_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false);
        auto temp = f32_to_ui4(ts1, softfloat_roundingMode, true);
        auto &accd = P.MU.acc_elt<uint8_t>(td_num + m, 0, i, j / 2 , mmax, nmax * amul * 4, reg_rename, true);
        accd = j % 2 == 0 ? ((temp & 0xF) << 4) | (accd & 0xF) : (temp & 0xF) | (accd & 0xF0);
    } else {
        P.MU.mill = true;
    }
  },
  {
    if (!P.MU.mfp64)
        require(0);
    
    auto ts1 = P.MU.acc_elt<float64_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false);
    P.MU.acc_elt<uint8_t>(td_num + m, 0, i, j, mmax, nmax * amul * 8, reg_rename, true) = f64_to_i8(ts1, softfloat_roundingMode, true);
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
  1, {require(P.MU.msew >= e32 && P.MU.msew <= e64);}, P.MU.msew)