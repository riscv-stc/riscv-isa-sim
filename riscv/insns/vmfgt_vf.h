// vmfgt.vf vd, vs2, rs1
VI_VFP_LOOP_CMP
({
  res = bf16_lt(rs1, vs2);
},
{
  res = f16_lt(rs1, vs2);
},
{
  res = f32_lt(rs1, vs2);
},
{
  res = f64_lt(rs1, vs2);
},
false)
