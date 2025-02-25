VM_CHECK_DSS(true);
MXU_MM_LOOP_WIDEN
({
  MXU_WIDE_OP_AND_ASSIGN(ts2, ts1, td_w, *, +, int, e32);
}, 2, e32)