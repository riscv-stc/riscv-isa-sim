// vsle.vi vd, vs2, simm5
require(P.VU.vstart == 0);

VI_VI_LOOP_CMP
({
  res = vs2 <= simm5;
})
