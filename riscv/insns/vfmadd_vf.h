// vfmadd: vd[i] = +(vd[i] * f[rs1]) + vs2[i]
VI_VFP_VF_LOOP
({
  vd = f32_to_bf16( f32_mulAdd(bf16_to_f32(vd), bf16_to_f32(rs1), bf16_to_f32(vs2)) );
},
{
  vd = f16_mulAdd(vd, rs1, vs2);
},
{
  vd = f32_mulAdd(vd, rs1, vs2);
},
{
  vd = f64_mulAdd(vd, rs1, vs2);
})
