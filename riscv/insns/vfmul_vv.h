// vfmul.vv vd, vs1, vs2, vm
VI_VFP_VV_LOOP
({
  vd = f32_to_bf16( f32_mul(bf16_to_f32(vs1), bf16_to_f32(vs2)) );
},
{
  vd = f16_mul(vs1, vs2);
},
{
  vd = f32_mul(vs1, vs2);
},
{
  vd = f64_mul(vs1, vs2);
})
