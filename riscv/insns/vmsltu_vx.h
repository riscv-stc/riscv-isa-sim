// vsltu.vx  vd, vs2, vs1
require(P.VU.vstart == 0);

VI_VX_ULOOP_CMP
({
  res = vs2 < rs1;
})
