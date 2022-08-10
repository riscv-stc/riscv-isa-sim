// vnsrl.vx vd, vs2, rs1
require(P.VU.vstart == 0);

VI_VX_LOOP_NSHIFT
({
  vd = vs2_u >> (rs1 & (sew * 2 - 1));
}, false)
