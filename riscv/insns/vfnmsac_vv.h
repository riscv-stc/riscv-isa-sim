// vfnmsac.vv vd, vs1, vs2, vm   # vd[i] = -(vs2[i] * vs1[i]) + vd[i]
VI_VFP_VV_LOOP
({
  vd = f32_to_bf16( f32_mulAdd(bf16_to_f32(bf16(vs1.v ^ BF16_SIGN)), bf16_to_f32(vs2), bf16_to_f32(vd)) );
},
{
  vd = f16_mulAdd(f16(vs1.v ^ F16_SIGN), vs2, vd);
},
{
  vd = f32_mulAdd(f32(vs1.v ^ F32_SIGN), vs2, vd);
},
{
  vd = f64_mulAdd(f64(vs1.v ^ F64_SIGN), vs2, vd);
})
