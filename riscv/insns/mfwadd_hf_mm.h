MXU_VFP_W_MM_ADD
({
  td = f16_add(ts1, ts2);
},
{
  td = bf16_add(ts1, ts2);
},
{
  td = f32_add(ts1, ts2);
},
{
  td = tf32_add(ts1, ts2);
},
{
  td = f64_add(ts1, ts2);
}, e16)