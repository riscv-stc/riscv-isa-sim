// vfmsac: vd[i] = +(vs1[i] * vs2[i]) - vd[i]
require(P.VU.vstart == 0);

VI_VFP_VV_LOOP
({
  vd = f32_to_bf16( f32_mulAdd(bf16_to_f32(vs1), bf16_to_f32(vs2), bf16_to_f32(bf16(vd.v ^ BF16_SIGN))) );
},
{
  vd = f16_mulAdd(vs1, vs2, f16(vd.v ^ F16_SIGN));
},
{
  vd = f32_mulAdd(vs1, vs2, f32(vd.v ^ F32_SIGN));
},
{
  vd = f64_mulAdd(vs1, vs2, f64(vd.v ^ F64_SIGN));
})
