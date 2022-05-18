// vmv.v.i vd, simm5
require(P.VU.vstart == 0);

require_vector(true);
VI_CHECK_SSS(false);
VI_VVXI_MERGE_LOOP
({
  vd = simm5;
})
