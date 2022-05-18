// vsltu.vv  vd, vs2, vs1
require(P.VU.vstart == 0);

VI_VV_ULOOP_CMP
({
  res = vs2 < vs1;
})
