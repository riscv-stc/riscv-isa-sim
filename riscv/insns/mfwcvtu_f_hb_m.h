  MXU_MFP_CVT_SCALE
  ({
    auto acc1 = P.MU.acc_elt<uint8_t>(ts1_num + m, 0, i, j / 2, mmax, nmax * amul / 2, false, false);
    bit4_pair_t <uint8_t> bit4_paire_acc1(acc1);

    if (P.MU.mfp32 == MTYPE_FP32) {
      if (j %2 == 0){
        P.MU.acc_elt<float32_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true) = ui32_to_f32((uint32_t)(bit4_paire_acc1.high));
      } else {
        P.MU.acc_elt<float32_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true) = ui32_to_f32((uint32_t)(bit4_paire_acc1.low));
      }
        
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
  8, {;}, e4)