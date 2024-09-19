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
    if (P.MU.mfp64){
        auto ts1 = P.MU.acc_elt<uint64_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, false);
        P.MU.acc_elt<float64_t>(td_num + m, 0, i, j, mmax, nmax * amul, reg_rename, true) = ui64_to_f64(ts1);
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
    require(p->extension_enabled(EXT_ZFH));
  },
  {
    require(p->extension_enabled('D'));
  },
  1, {;}, e64)

