
MXU_VFP_VV_LOOP({
    td = f16_min(ts1, ts2);
},
{
    td = f32_min(ts1, ts2);
},
{
    td = f64_min(ts1, ts2);
})