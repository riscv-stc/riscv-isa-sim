  MXU_MFP_CVT_SCALE
  ({
    auto ts1 = P.MU.acc_elt<int8_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false);
    int8_t temp_low = ts1 & 0xFF;
    int8_t temp_high = (ts1 >> 4) & 0xFF;
    BIT4_SIGN_EXTEND_INT8(int8_t, temp_low)
    BIT4_SIGN_EXTEND_INT8(int8_t, temp_high)
    if (P.MU.mfp16 == MTYPE_FP16) {
        P.MU.acc_elt<float16_t>(td_num + m, 0, i, j * 2, mmax, des_nmax * amul, reg_rename, true) = ui32_to_f16(temp_low);
        P.MU.acc_elt<float16_t>(td_num + m, 0, i, j * 2 + 1, mmax, des_nmax * amul, reg_rename, true) = ui32_to_f16(temp_high);
    } else if (P.MU.mfp16 == MTYPE_BF16) {
        P.MU.acc_elt<bfloat16_t>(td_num + m, 0, i, j * 2, mmax, des_nmax * amul, reg_rename, true) = ui32_to_bf16(temp_low);
        P.MU.acc_elt<bfloat16_t>(td_num + m, 0, i, j * 2 + 1, mmax, des_nmax * amul, reg_rename, true) = ui32_to_bf16(temp_high);
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