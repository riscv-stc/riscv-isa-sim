  MXU_MFP_CVT_SCALE
  ({
    auto ts1 = P.MU.acc_elt<uint8_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false);
    if (P.MU.mfp16 == MTYPE_FP16) {
        P.MU.acc_elt<float16_t>(td_num + m, 0, i, j * 2, mmax, des_nmax * amul, reg_rename, true) = ui32_to_f16((uint32_t)(ts1 & 0xFF));
        P.MU.acc_elt<float16_t>(td_num + m, 0, i, j * 2 + 1, mmax, des_nmax * amul, reg_rename, true) = ui32_to_f16((uint32_t)((ts1 >> 4)& 0xFF));
    } else if (P.MU.mfp16 == MTYPE_BF16) {
        // P.MU.acc_elt<bfloat16_t>(td_num + m, 0, i, j * 2, mmax, des_nmax * amul, reg_rename, true) = ui32_to_bf16((uint32_t)(ts1 & 0xFF));
        P.MU.acc_elt<bfloat16_t>(td_num + m, 0, i, j * 2 + 1, mmax, des_nmax * amul, reg_rename, true) = ui32_to_bf16((uint32_t)((ts1 >> 4) & 0xFF));
    } else {
        require(0);
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
  2, {;}, e8)