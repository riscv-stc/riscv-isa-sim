// vmacc.vv: vd[i] = +(vs1[i] * vs2[i]) + vd[i]
require(P.VU.vstart == 0);

VI_VV_LOOP
({
  vd = vs1 * vs2 + vd;
})
