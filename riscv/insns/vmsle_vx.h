// vsle.vx vd, vs2, rs1
require(P.VU.vstart == 0);

VI_VX_LOOP_CMP
({
  res = vs2 <= rs1;
})
