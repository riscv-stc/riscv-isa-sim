// vredmaxu.vs vd, vs2 ,vs1
require(P.VU.vstart == 0);

VI_VV_ULOOP_REDUCTION
({
  vd_0_res = (vd_0_res >= vs2) ? vd_0_res : vs2;
})
