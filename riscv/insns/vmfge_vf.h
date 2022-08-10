// vmfge.vf vd, vs2, rs1
require(P.VU.vstart == 0);

VI_VFP_LOOP_CMP
({
  res = bf16_le(rs1, vs2);
},
{
  res = f16_le(rs1, vs2);
},
{
  res = f32_le(rs1, vs2);
},
{
  res = f64_le(rs1, vs2);
},
false)
