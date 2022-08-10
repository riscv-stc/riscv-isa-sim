// vrsub.vi vd, vs2, imm, vm   # vd[i] = imm - vs2[i]
require(P.VU.vstart == 0);

VI_VI_LOOP
({
  vd = simm5 - vs2;
})
