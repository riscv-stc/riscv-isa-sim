// vwsub.wv vd, vs2, vs1
require(P.VU.vstart == 0);

VI_CHECK_DDS(true);
VI_VV_LOOP_WIDEN
({
  VI_WIDE_WVX_OP(vs1, -, int);
})
