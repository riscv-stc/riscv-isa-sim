// vfmul.vf vd, vs2, rs1, vm
require(P.VU.vstart == 0);

VI_VFP_VF_LOOP
({
  vd = f32_to_bf16( f32_mul(bf16_to_f32(vs2), bf16_to_f32(rs1)) );
},
{
  vd = f16_mul(vs2, rs1);
},
{
  vd = f32_mul(vs2, rs1);
},
{
  vd = f64_mul(vs2, rs1);
})
