MMV_TR_ACC_INTERTRANS({
    auto acc1  = P.MU.acc_elt<uint16_t>(ts1_num + m, 0, i, j + start_width, mmax, nmax * amul, reg_rename, false); 
    auto &td  = P.MU.tr_elt<uint16_t>(td_num + m, 0, i, j, mmax, nmax, false, true); 
    td = acc1;
}, e16, RS2 * P.MU.mlenb / P.MU.msew)