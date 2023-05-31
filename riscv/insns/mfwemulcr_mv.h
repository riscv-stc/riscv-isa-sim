MXU_MEMUL_MVFP_WIDEN(
0,
{
  accd = f32_mul(acc1, rs2);
},
{
  accd = f64_mul(acc1, rs2);
}
)