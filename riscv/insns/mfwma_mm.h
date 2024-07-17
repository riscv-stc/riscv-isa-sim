MXU_VFP_VV_LOOP_WIDE
({
  td_w = f32_mulAdd(ts1, ts2, td_w);
},
{
  td_w = f64_mulAdd(ts1, ts2, td_w);
}, 2)