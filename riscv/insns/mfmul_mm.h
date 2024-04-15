MI_VFP_MM_LOOP({
    td = f16_mul(ts1, ts2);
},
{
    td = f32_mul(ts1, ts2);
},
{
    td = f64_mul(ts1, ts2);
})