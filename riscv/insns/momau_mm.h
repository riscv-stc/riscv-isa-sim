VM_CHECK_DSS(true);
MXU_MM_LOOP_OCT
({
  MXU_OCT_OP_AND_ASSIGN(ts2, ts1, td_o, *, +, uint, P.MU.msew);
}, 8, P.MU.msew)