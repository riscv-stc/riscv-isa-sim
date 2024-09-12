  MXU_MFP_CVT_SCALE
  ({
    ;
  },
  {
    if (P.MU.mfp16 == MTYPE_FP16){
        auto ts1 = P.MU.acc_elt<float16_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false);
        auto temp = f16_to_i4(ts1, softfloat_roundingMode, true);
        auto &accd = P.MU.acc_elt<int8_t>(td_num + m, 0, i, j / 2 , mmax, nmax * amul * 2, reg_rename, true);
        accd = j % 2 ? ((temp & 0xF) << 4) | accd : temp & 0xF;
    } else if (P.MU.mfp16 == MTYPE_BF16) {
        auto ts1 = P.MU.acc_elt<bfloat16_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false);
        auto temp = bf16_to_i4(ts1, softfloat_roundingMode, true);
        auto &accd = P.MU.acc_elt<int8_t>(td_num + m, 0, i, j / 2 , mmax, nmax * amul * 2, reg_rename, true);
        accd = j % 2 ? ((temp & 0xF) << 4) | accd : temp & 0xF;
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
    require(p->extension_enabled(EXT_ZFH));
  },
  {
    require(p->extension_enabled('D'));
  },
  {
    ;
  },
  1, {;}, e16)