MXU_VFP_VV_LOOP
({
  accd = f16_mulAdd(ts1, ts2, accd);
},
{
  accd = f32_mulAdd(ts1, ts2, accd);
},
{
  accd = f64_mulAdd(ts1, ts2, accd);
})