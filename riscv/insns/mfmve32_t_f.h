require_fp;
MMV_TR_ACC_BASE({
    switch(P.MU.mfp32){
        case 1: //fp32
            auto &ts1 = P.MU.tr_elt<float32_t>(ts1_num + m, 0, i, j, mmax, nmax, false, true);
            ts1 = FRS1_F;
            break;
        case 2: //tf32
            auto &ts1 = P.MU.tr_elt<tfloat32_t>(ts1_num + m, 0, i, j, mmax, nmax, false, true);
            ts1 = FRS1_TF;
            break;
        default:
            require(0);
    }
   
}
, insn.rs1(), e32);