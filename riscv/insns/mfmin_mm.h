
MI_VFP_MM_LOOP({
    td = f16_min(ts1, ts2);
},
{
    td = f32_min(ts1, ts2);
},
{
    td = f64_min(ts1, ts2);
})