// vnsrl.vv vd, vs2, vs1
require(P.VU.vstart == 0);

VI_VV_LOOP_NSHIFT
({
  vd = vs2_u >> (vs1 & (sew * 2 - 1));
}, true)
