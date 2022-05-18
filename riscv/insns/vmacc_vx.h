// vmacc.vx: vd[i] = +(x[rs1] * vs2[i]) + vd[i]
require(P.VU.vstart == 0);

VI_VX_LOOP
({
  vd = rs1 * vs2 + vd;
})
