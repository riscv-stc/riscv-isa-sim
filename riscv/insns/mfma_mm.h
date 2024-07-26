MXU_VFP_VV_LOOP
({
  td = f16_mulAdd(ts1, ts2, td);
},
{
  td = f32_mulAdd(ts1, ts2, td);
},
{
  td = f64_mulAdd(ts1, ts2, td);
}, 1)
