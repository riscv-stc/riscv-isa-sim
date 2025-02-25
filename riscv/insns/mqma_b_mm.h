VM_CHECK_DSS(true);
MXU_MM_LOOP_WIDEN
({
  MXU_QUAD_OP_AND_ASSIGN(ts2, ts1, td_q, *, +, int, e8);
}, 4, e8)
