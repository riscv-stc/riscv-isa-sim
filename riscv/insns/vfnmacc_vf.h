// vfnmacc: vd[i] = -(f[rs1] * vs2[i]) - vd[i]
require(P.VU.vstart == 0);

VI_VFP_VF_LOOP
({
  vd = f32_to_bf16( f32_mulAdd(bf16_to_f32(rs1), bf16_to_f32(bf16(vs2.v ^ BF16_SIGN)), bf16_to_f32(bf16(vd.v ^ BF16_SIGN))) );
},
{
  vd = f16_mulAdd(rs1, f16(vs2.v ^ F16_SIGN), f16(vd.v ^ F16_SIGN));
},
{
  vd = f32_mulAdd(rs1, f32(vs2.v ^ F32_SIGN), f32(vd.v ^ F32_SIGN));
},
{
  vd = f64_mulAdd(rs1, f64(vs2.v ^ F64_SIGN), f64(vd.v ^ F64_SIGN));
})
