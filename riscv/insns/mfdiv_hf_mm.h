MI_VFP_MM_LOOP(
{
    td = f8e4m3_div(ts1, ts2);
},
{
    td = f8e5m2_div(ts1, ts2);
},
{
    td = f8e3m4_div(ts1, ts2);
},
{
    td = f16_div(ts1, ts2);
},
{
    td = bf16_div(ts1, ts2);
},
{
    td = f32_div(ts1, ts2);
},
{
    td = tf32_div(ts1, ts2);
},
{
    td = f64_div(ts1, ts2);
}, 1, e16)