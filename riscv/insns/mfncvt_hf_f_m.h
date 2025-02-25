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
    switch (P.MU.mfp16)
    {
    case MTYPE_FP16:
      if (P.MU.mfp32 == MTYPE_FP32) {
        auto ts1 = P.MU.acc_elt<float32_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false);
        P.MU.acc_elt<float16_t>(td_num + m, 0, i, j, mmax, nmax * 2 * amul, reg_rename, true) = f32_to_f16(ts1);

      } else {
        P.MU.mill = true;
      }
      break;
    case MTYPE_BF16:
      if (P.MU.mfp32 == MTYPE_FP32) {
        auto rs1 = P.MU.acc_elt<float32_t>(ts1_num + m , 0, i, j, mmax, nmax * amul, false, false);
        P.MU.acc_elt<bfloat16_t>(td_num + m, 0, i, j, mmax, nmax * 2 * amul, reg_rename, true) = f32_to_bf16(rs1);
      } else {
        P.MU.mill = true;
      }
      break;
    default:
      require(0);
      break;
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
  1, {;}, e32)
