MI_VFP_MM_LOOP(
{
    td = f8e4m3_mul(ts1, ts2);
},
{
    td = f8e5m2_mul(ts1, ts2);
},
{
    td = f8e3m4_mul(ts1, ts2);
},
{
    td = f16_mul(ts1, ts2);
},
{
    td = bf16_mul(ts1, ts2);
},
{
    td = f32_mul(ts1, ts2);
},
{
    td = tf32_mul(ts1, ts2);
},
{
    td = f64_mul(ts1, ts2);
}, 1, e64)