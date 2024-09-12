MXU_VFP_W_MM_ADD
({
  td = f16_sub(ts1, ts2);
},
{
  td = bf16_sub(ts1, ts2);
},
{
  td = f32_sub(ts1, ts2);
},
{
  td = tf32_sub(ts1, ts2);
},
{
  td = f64_sub(ts1, ts2);
}, P.MU.msew)
