require_fp;
MMV_TR_ACC_BASE({
    switch(P.MU.mfp16){
        case 1:{ //fp16
            auto &ts1 = P.MU.acc_elt<float16_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, true);
            ts1 = FRS1_H;
            break;
        }
        case 2: {//bf16
            auto &ts1 = P.MU.acc_elt<bfloat16_t>(ts1_num + m, 0, i, j, mmax, nmax * amul, false, true);
            ts1 = FRS1_BF;
            break;
        }
        default: {
            require(0);
            break;
        }
    }
   
}
, insn.rs1(), e16);