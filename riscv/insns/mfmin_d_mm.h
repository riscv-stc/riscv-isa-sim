
MI_VFP_MM_LOOP(
{
    td = f8e4m3_min(ts1, ts2);
},
{
    td = f8e5m2_min(ts1, ts2);
},
{
    td = f8e3m4_min(ts1, ts2);
},
{
    td = f16_min(ts1, ts2);
},
{
    td = bf16_min(ts1, ts2);
},
{
    td = f32_min(ts1, ts2);
},
{
    td = tf32_min(ts1, ts2);
},
{
    td = f64_min(ts1, ts2);
}, 1, e64)