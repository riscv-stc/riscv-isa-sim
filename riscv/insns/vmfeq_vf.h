// vmfeq.vf vd, vs2, fs1
require(P.VU.vstart == 0);

VI_VFP_LOOP_CMP
({
  res = bf16_eq(vs2, rs1);
},
{
  res = f16_eq(vs2, rs1);
},
{
  res = f32_eq(vs2, rs1);
},
{
  res = f64_eq(vs2, rs1);
},
false)
