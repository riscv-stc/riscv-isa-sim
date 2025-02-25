VM_CHECK_DSS(true);
MXU_MM_LOOP_WIDEN
({
  MXU_QUAD_OP_AND_ASSIGN(ts2, ts1, td_q, *, +, uint, P.MU.msew);
}, 4, P.MU.msew)