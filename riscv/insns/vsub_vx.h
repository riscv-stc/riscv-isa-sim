// vsub: vd[i] = (vd[i] * x[rs1]) - vs2[i]
require(P.VU.vstart == 0);

VI_VX_LOOP
({
  vd = vs2 - rs1;
})
