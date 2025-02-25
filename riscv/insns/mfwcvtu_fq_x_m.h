  MXU_MFP_CVT_SCALE
  ({
    auto acc1 = P.MU.acc_elt<uint8_t>(ts1_num + m, 0, i, j / 2, mmax, nmax * amul / 2, false, false);
    bit4_pair_t <uint8_t> bit4_paire_acc1(acc1);

    if (P.MU.mfp16 == MTYPE_FP16) {
      if (j %2 == 0){
        P.MU.acc_elt<float16_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true) = ui32_to_f16((uint32_t)(bit4_paire_acc1.high));
      } else {
        P.MU.acc_elt<float16_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true) = ui32_to_f16((uint32_t)(bit4_paire_acc1.low));
      }
        
    } else if (P.MU.mfp16 == MTYPE_BF16) {
      if (j % 2 == 0) {
        P.MU.acc_elt<bfloat16_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true) = ui32_to_bf16((uint32_t)(bit4_paire_acc1.high));
      } else {
        P.MU.acc_elt<bfloat16_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true) = ui32_to_bf16((uint32_t)(bit4_paire_acc1.low));
      }
    } else {
        P.MU.mill = true;
    }
  },
  {
    auto ts1 = P.MU.acc_elt<uint8_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false);

    if (P.MU.mfp32 == MTYPE_FP32) {
        P.MU.acc_elt<float32_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true) = ui32_to_f32((uint32_t)ts1);
    } else {
        P.MU.mill = true;
    }
  },
  {
    auto ts1 = P.MU.acc_elt<uint16_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false);

    if (!P.MU.mfp64) {
        P.MU.mill = true;
    } 
    P.MU.acc_elt<float64_t>(td_num + m, 0, i, j, mmax, des_nmax * amul, reg_rename, true) = ui32_to_f64((uint32_t)ts1);
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

// reg_t sew = P.MU.msew;
  // MU_MFP_LOOP_SCALE_BASE(4, sew)
  // switch(sew) { 
  //   case e4: {
  //     BODY4 
  //       BODY8 
  //       set_fp_exceptions; 
  //     } 
  //     break; 
  //   case e8: {
  //     CHECK8 
  //       BODY8 
  //       set_fp_exceptions; 
  //     } 
  //     break; 
  //   case e16: {
  //     CHECK16 
  //       BODY16 
  //       set_fp_exceptions; 
  //     } 
  //     break; 
  //   case e32: {
  //     CHECK32 
  //       BODY32 
  //       set_fp_exceptions; 
  //     } 
  //     break; 
  //   case e64: {
  //     CHECK64 
  //       BODY64 
  //       set_fp_exceptions; 
  //     } 
  //     break; 
  //   default: 
  //     require(0); 
  //     break; 
  // } 
  // MU_MFP_LOOP_END 
  // REGNAME_WRITE_BAKE(mmax, nmax * amul, reg_sum, true, sew) 

