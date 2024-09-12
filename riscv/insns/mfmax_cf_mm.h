
MI_VFP_MM_LOOP(
{
    td = f8e4m3_max(ts1, ts2);
},
{
    td = f8e5m2_max(ts1, ts2);
},
{
    td = f8e3m4_max(ts1, ts2);
},
{
    td = f16_max(ts1, ts2);
},
{
    td = bf16_max(ts1, ts2);
},
{
    td = f32_max(ts1, ts2);
},
{
    td = tf32_max(ts1, ts2);
},
{
    td = f64_max(ts1, ts2);
}, 1, e8)