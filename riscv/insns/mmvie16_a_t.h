MMV_TR_ACC_INTERTRANS({
    auto &accd  = P.MU.acc_elt<uint16_t>(td_num + m, 0, i, j + start_width, mmax, nmax * amul, reg_rename, true); 
    auto ts1  = P.MU.tr_elt<uint16_t>(ts1_num + m, 0, i, j, mmax, nmax, false, false); 
    accd = ts1;
}, e16, insn.p_imm5() * P.MU.mlenb / P.MU.msew)