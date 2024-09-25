VM_CHECK_DSS(true);
MXU_VV_LOOP_OCT
({
  MXU_OCT_OP_AND_ASSIGN(ts2, ts1, td_o, *, +, int, e4);
}, 8, P.MU.msew)