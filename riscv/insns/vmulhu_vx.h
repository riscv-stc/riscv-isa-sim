// vmulhu vd ,vs2, rs1
require(P.VU.vstart == 0);

VI_VX_ULOOP
({
  vd = ((uint128_t)vs2 * rs1) >> sew;
})
