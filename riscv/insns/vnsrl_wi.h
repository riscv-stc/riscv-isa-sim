// vnsrl.vi vd, vs2, zimm5
require(P.VU.vstart == 0);

VI_VI_LOOP_NSHIFT
({
  vd = vs2_u >> (zimm5 & (sew * 2 - 1));
}, false)
