// vmfle.vv vd, vs2, rs1
require(P.VU.vstart == 0);

VI_VFP_LOOP_CMP
({
  res = bf16_le(vs2, vs1);
},
{
  res = f16_le(vs2, vs1);
},
{
  res = f32_le(vs2, vs1);
},
{
  res = f64_le(vs2, vs1);
},
true)
