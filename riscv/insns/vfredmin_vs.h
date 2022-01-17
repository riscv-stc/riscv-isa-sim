// vfredmin vd, vs2, vs1
bool is_propagate = true;
VI_VFP_VV_LOOP_REDUCTION
({
  vd_0 = bf16_min(vd_0, vs2);
},
{
  vd_0 = f16_min(vd_0, vs2);
},
{
  vd_0 = f32_min(vd_0, vs2);
},
{
  vd_0 = f64_min(vd_0, vs2);
})
