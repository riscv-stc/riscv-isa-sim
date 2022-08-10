// vfmin vd, vs2, rs1
require(P.VU.vstart == 0);

VI_VFP_VF_LOOP
({
  vd = bf16_min(vs2, rs1);
},
{
  vd = f16_min(vs2, rs1);
},
{
  vd = f32_min(vs2, rs1);
},
{
  vd = f64_min(vs2, rs1);
})
