require_fp;
MMV_TR_ACC_BASE({
    switch(P.MU.mfp8){
        case 1: {//e4m3
            auto &ts1 = P.MU.acc_elt<float8_e4m3_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, true);
            ts1 = FRS1_HF8_E4M3;
            break;
        }
        case 2: {//e5m2
            auto &ts1 = P.MU.acc_elt<float8_e5m2_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, true);
            ts1 = FRS1_HF8_E5M2;
            break;
        }
        case 3: {//e3m4
            auto &ts1 = P.MU.acc_elt<float8_e3m4_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, true);
            ts1 = FRS1_HF8_E3M4;
            break;
        }
        default: {
            require(0);
            break;
        }
    }
   
}
, insn.rs1(), e8);