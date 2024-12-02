MI_VFP_MM_LOOP(
{
    td = f8e4m3_sqrt(ts1);
},
{
    td = f8e5m2_sqrt(ts1);
},
{
    td = f8e3m4_sqrt(ts1);
},
{
    td = f16_sqrt(ts1);
},
{
    td = bf16_sqrt(ts1);
},
{
    td = f32_sqrt(ts1);
},
{
    td = tf32_sqrt(ts1);
},
{
    td = f64_sqrt(ts1);
}, 1, P.MU.msew)