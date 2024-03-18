MXU_VFP_VV_LOOP_WIDE
({
  accd_w = f32_mulAdd(ts1, ts2, accd_w);
},
{
  accd_w = f64_mulAdd(ts1, ts2, accd_w);
})