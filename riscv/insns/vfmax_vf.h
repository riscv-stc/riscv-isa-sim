// vfmax
require(P.VU.vstart == 0);

VI_VFP_VF_LOOP
({
  vd = bf16_max(vs2, rs1);
},
{
  vd = f16_max(vs2, rs1);
},
{
  vd = f32_max(vs2, rs1);
},
{
  vd = f64_max(vs2, rs1);
})
