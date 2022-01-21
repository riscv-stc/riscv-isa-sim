// vfadd.vv vd, vs2, vs1
VI_VFP_VV_LOOP
({
  vd = f32_to_bf16( f32_add(bf16_to_f32(vs1), bf16_to_f32(vs2)) );
},
{
  vd = f16_add(vs1, vs2);
},
{
  vd = f32_add(vs1, vs2);
},
{
  vd = f64_add(vs1, vs2);
})
