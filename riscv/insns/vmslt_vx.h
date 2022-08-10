// vslt.vx  vd, vs2, vs1
require(P.VU.vstart == 0);

VI_VX_LOOP_CMP
({
  res = vs2 < rs1;
})
