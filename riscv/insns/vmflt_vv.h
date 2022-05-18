// vmflt.vv vd, vs2, vs1
require(P.VU.vstart == 0);

VI_VFP_LOOP_CMP
({
  res = bf16_lt(vs2, vs1);
},
{
  res = f16_lt(vs2, vs1);
},
{
  res = f32_lt(vs2, vs1);
},
{
  res = f64_lt(vs2, vs1);
},
true)
