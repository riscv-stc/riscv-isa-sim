// vmfeq.vv vd, vs2, vs1
VI_VFP_LOOP_CMP
({
  res = bf16_eq(vs2, vs1);
},
{
  res = f16_eq(vs2, vs1);
},
{
  res = f32_eq(vs2, vs1);
},
{
  res = f64_eq(vs2, vs1);
},
true)
