// vfsub.vf vd, vs2, rs1
require(P.VU.vstart == 0);

VI_VFP_VF_LOOP
({
  vd = f32_to_bf16( f32_sub(bf16_to_f32(rs1), bf16_to_f32(vs2)) );
},
{
  vd = f16_sub(rs1, vs2);
},
{
  vd = f32_sub(rs1, vs2);
},
{
  vd = f64_sub(rs1, vs2);
})
