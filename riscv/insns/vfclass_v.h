// vfclass.v vd, vs2, vm
require(P.VU.vstart == 0);

VI_VFP_V_LOOP
({
  vd.v = bf16_classify(vs2);
},
{
  vd.v = f16_classify(vs2);
},
{
  vd.v = f32_classify(vs2);
},
{
  vd.v = f64_classify(vs2);
})
