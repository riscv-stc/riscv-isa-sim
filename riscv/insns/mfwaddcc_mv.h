MXU_MEMUL_MVFP_WIDEN(
1,
{
  accd = f32_add(acc1, rs2);
},
{
  accd = f64_add(acc1, rs2);
}
)