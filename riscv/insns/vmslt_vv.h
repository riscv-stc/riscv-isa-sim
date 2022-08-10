// vslt.vv  vd, vd2, vs1
require(P.VU.vstart == 0);

VI_VV_LOOP_CMP
({
  res = vs2 < vs1;
})
