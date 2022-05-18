// vsrl.vi vd, vs2, zimm5
require(P.VU.vstart == 0);

VI_VI_ULOOP
({
  vd = vs2 >> (zimm5 & (sew - 1) & 0x1f);
})
