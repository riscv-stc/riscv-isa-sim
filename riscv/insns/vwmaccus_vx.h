// vwmaccus.vx vd, vs2, rs1
require(P.VU.vstart == 0);

VI_CHECK_DSS(false);
VI_VX_LOOP_WIDEN
({
  VI_WIDE_OP_AND_ASSIGN_MIX(vs2, rs1, vd_w, *, +, int, int, uint);
})
