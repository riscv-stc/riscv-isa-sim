// vfsub.vv vd, vs2, vs1
VI_VFP_VV_LOOP
({
  vd = f32_to_bf16( f32_sub(bf16_to_f32(vs2), bf16_to_f32(vs1)) );
},
{
  vd = f16_sub(vs2, vs1);
},
{
  vd = f32_sub(vs2, vs1);
},
{
  vd = f64_sub(vs2, vs1);
})
