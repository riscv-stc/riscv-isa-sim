VM_CHECK_DSS(true);
MXU_VV_LOOP_WIDEN
({
  MXU_WIDE_OP_AND_ASSIGN(ts2, ts1, td_w, *, +, uint, e16);
}, 2, e16)