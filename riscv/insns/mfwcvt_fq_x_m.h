  MXU_MFP_CVT_SCALE
  ({
    auto acc1 = P.MU.acc_elt<int8_t>(ts1_num + m, 0, i, j / 2, mmax, nmax * amul / 2, false, false);
    bit4_pair_t <int8_t> bit4_paire_acc1(acc1);

    if (P.MU.mfp16 == MTYPE_FP16) {
      if (j %2 == 0){
        P.MU.acc_elt<float16_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true) = i32_to_f16((uint32_t)(bit4_paire_acc1.high));
      } else {
        P.MU.acc_elt<float16_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true) = i32_to_f16((uint32_t)(bit4_paire_acc1.low));
      }
        
    } else if (P.MU.mfp16 == MTYPE_BF16) {
      if (j % 2 == 0) {
        P.MU.acc_elt<bfloat16_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true) = i32_to_bf16((uint32_t)(bit4_paire_acc1.high));
      } else {
        P.MU.acc_elt<bfloat16_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true) = i32_to_bf16((uint32_t)(bit4_paire_acc1.low));
      }
    } else {
        P.MU.mill = true;
    }
  },
  {
    auto ts1 = P.MU.acc_elt<int8_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false);

    if (P.MU.mfp32 == MTYPE_FP32) {
        P.MU.acc_elt<float32_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true) = i32_to_f32((int32_t)ts1);
    } else {
        P.MU.mill = true;
    }
  },
  {
    auto ts1 = P.MU.acc_elt<int16_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false);

    if (!P.MU.mfp64) {
        P.MU.mill = true;
    } else {
      P.MU.acc_elt<float64_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true) = i32_to_f64((int32_t)ts1);
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
  4, {require(P.MU.msew >= e4 && P.MU.msew <= e16);}, P.MU.msew)