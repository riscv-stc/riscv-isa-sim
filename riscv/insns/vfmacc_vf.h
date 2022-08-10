// vfmacc.vf vd, rs1, vs2, vm    # vd[i] = +(vs2[i] * x[rs1]) + vd[i]
require(P.VU.vstart == 0);

VI_VFP_VF_LOOP
({
  vd = f32_to_bf16( f32_mulAdd(bf16_to_f32(rs1), bf16_to_f32(vs2), bf16_to_f32(vd)) );
},
{
  vd = f16_mulAdd(rs1, vs2, vd);
},
{
  vd = f32_mulAdd(rs1, vs2, vd);
},
{
  vd = f64_mulAdd(rs1, vs2, vd);
})
