require_fp;
MMV_TR_ACC_BASE({
   if (P.MU.mf64){
       MFU_PARAMS_TR1(e64);
       ts1 = FRS1_D;
   }
   else 
    require(0);
}
, insn.rs1(), e64);