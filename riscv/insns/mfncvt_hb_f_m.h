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
        auto temp = f32_to_i4(ts1, softfloat_roundingMode, true);
        auto &accd = P.MU.acc_elt<int8_t>(td_num + m, 0, i, j / 2 , mmax, nmax * amul * 4, reg_rename, true);
        accd = j % 2 == 0 ? ((temp & 0xF) << 4) | (accd & 0xF) : (temp & 0xF) | (accd & 0xF0);
    } else {
        P.MU.mill = true;
    }
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
  1, {;}, e32)