// vmflt.vf vd, vs2, rs1
VI_VFP_LOOP_CMP
({
  res = bf16_lt(vs2, rs1);
},
{
  res = f16_lt(vs2, rs1);
},
{
  res = f32_lt(vs2, rs1);
},
{
  res = f64_lt(vs2, rs1);
},
false)
