// vmax.vv vd, vs2, vs1, vm   # Vector-vector
require(P.VU.vstart == 0);

VI_VV_LOOP
({
  if (vs1 >= vs2) {
    vd = vs1;
  } else {
    vd = vs2;
  }

})
