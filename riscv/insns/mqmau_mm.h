VM_CHECK_DSS(true);
MXU_VV_LOOP_WIDEN
({
  MXU_QUAD_OP_AND_ASSIGN(ts2, ts1, td_q, *, +, uint);
}, 4)