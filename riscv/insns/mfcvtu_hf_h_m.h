  MXU_MFP_CVT_SCALE
  ({
  ;
  },
  {
    auto rs1 = P.MU.acc_elt<uint16_t>(ts1_num + m , 0, i, j, mmax, nmax * amul, false, false);
    if (P.MU.mfp16 == MTYPE_FP16){
        P.MU.acc_elt<float16_t>(td_num + m, 0, i, j, mmax, nmax * amul, reg_rename, true) = ui32_to_f16((uint32_t)rs1);
    } else if (P.MU.mfp16 == MTYPE_BF16){
        P.MU.acc_elt<bfloat16_t>(td_num + m, 0, i, j, mmax, nmax * amul, reg_rename, true) = ui32_to_bf16((uint32_t)rs1);
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
  1, {;}, e16)

