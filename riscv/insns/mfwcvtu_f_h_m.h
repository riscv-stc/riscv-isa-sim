  MXU_MFP_CVT_SCALE
  ({
  ;
  },
  {
   ;
  },
  {
    
    auto ts1 = P.MU.acc_elt<uint16_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false);
    if (P.MU.mfp32 == MTYPE_FP32) {
      P.MU.acc_elt<float32_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true) = ui32_to_f32((uint32_t)ts1);
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
  2, {;}, e16)