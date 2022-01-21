// vfnmsub: vd[i] = -(vd[i] * vs1[i]) + vs2[i]
VI_VFP_VV_LOOP
({
  vd = f32_to_bf16( f32_mulAdd(bf16_to_f32(bf16(vd.v ^ BF16_SIGN)), bf16_to_f32(vs1), bf16_to_f32(vs2)) );
},
{
  vd = f16_mulAdd(f16(vd.v ^ F16_SIGN), vs1, vs2);
},
{
  vd = f32_mulAdd(f32(vd.v ^ F32_SIGN), vs1, vs2);
},
{
  vd = f64_mulAdd(f64(vd.v ^ F64_SIGN), vs1, vs2);
})
