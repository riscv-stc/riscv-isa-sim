MI_VFP_MM_LOOP({
    td = f16_div(ts1, ts2);
},
{
    td = f32_div(ts1, ts2);
},
{
    td = f64_div(ts1, ts2);
})