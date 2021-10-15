// vfsub.vf vd, vs2, rs1
VI_VFP_VF_LOOP
({
  vd = bf16_sub(rs1, vs2);
},
{
  vd = f16_sub(rs1, vs2);
},
{
  vd = f32_sub(rs1, vs2);
},
{
  vd = f64_sub(rs1, vs2);
})
