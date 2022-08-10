// vand.vx vd, rs1, vs2, vm
require(P.VU.vstart == 0);

VI_VX_LOOP
({
  vd = rs1 & vs2;
})
