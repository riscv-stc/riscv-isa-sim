MXU_MEMUL_MFP
({
  accd = f16_mul(acc1, rs2);
},
{
  accd = f32_mul(acc1, rs2);
},
{
  accd = f64_mul(acc1, rs2);
})