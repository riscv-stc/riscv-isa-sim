// vfadd.vf vd, vs2, rs1
require(P.VU.vstart == 0);

VI_VFP_VF_LOOP
({
  vd = f32_to_bf16( f32_add(bf16_to_f32(rs1), bf16_to_f32(vs2)) );
},
{
  vd = f16_add(rs1, vs2);
},
{
  vd = f32_add(rs1, vs2);
},
{
  vd = f64_add(rs1, vs2);
})
