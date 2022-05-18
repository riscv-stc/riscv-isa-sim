// vfmax
require(P.VU.vstart == 0);

VI_VFP_VV_LOOP
({
  vd = bf16_max(vs2, vs1);
},
{
  vd = f16_max(vs2, vs1);
},
{
  vd = f32_max(vs2, vs1);
},
{
  vd = f64_max(vs2, vs1);
})
