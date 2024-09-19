  MXU_MFP_CVT_SCALE
  ({
    if (!P.MU.mfp64)
        require(0);
    auto ts1 = P.MU.acc_elt<uint8_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false);
    P.MU.acc_elt<float64_t>(td_num + m, 0, i, j * 2, mmax, des_nmax * amul, reg_rename, true) = ui32_to_f64((uint32_t)ts1);
    
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
  8, {;}, e8)