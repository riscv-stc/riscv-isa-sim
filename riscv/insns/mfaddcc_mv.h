MXU_MEMUL_MVFP(
1,
{
  accd = f16_add(acc1, rs2);
},
{
  accd = f32_add(acc1, rs2);
},
{
  accd = f64_add(acc1, rs2);
}
)