// vsaddu vd, vs2, zimm5
require(P.VU.vstart == 0);

VI_VI_ULOOP
({
  bool sat = false;
  vd = vs2 + (insn.v_simm5() & (UINT64_MAX >> (64 - P.VU.vsew)));

  sat = vd < vs2;
  vd |= -(vd < vs2);

  P.VU.vxsat |= sat;
})
