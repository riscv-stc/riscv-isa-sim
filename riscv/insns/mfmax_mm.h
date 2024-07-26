
MI_VFP_MM_LOOP({
    td = f16_max(ts1, ts2);
},
{
    td = f32_max(ts1, ts2);
},
{
    td = f64_max(ts1, ts2);
}, 1)