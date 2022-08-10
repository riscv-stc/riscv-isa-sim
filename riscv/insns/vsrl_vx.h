// vsrl.vx vd, vs2, rs1
require(P.VU.vstart == 0);

VI_VX_ULOOP
({
  vd = vs2 >> (rs1 & (sew - 1));
})
