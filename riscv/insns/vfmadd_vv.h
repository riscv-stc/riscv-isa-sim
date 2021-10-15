// vfmadd: vd[i] = +(vd[i] * vs1[i]) + vs2[i]
VI_VFP_VV_LOOP
({
  vd = bf16_mulAdd(vd, vs1, vs2);
},
{
  vd = f16_mulAdd(vd, vs1, vs2);
},
{
  vd = f32_mulAdd(vd, vs1, vs2);
},
{
  vd = f64_mulAdd(vd, vs1, vs2);
})
