MXU_MEMUL_MFP_WIDEN
(
{
  accd = f32_mul(acc1, rs2);
},
{
  accd = f64_mul(acc1, rs2);
})