MXU_MMACC_MVFP(
0,
{
  accd = f16_mulAdd(vs1, vs2, accd);
},
{
  accd = f32_mulAdd(vs1, vs2, accd);
},
{
  accd = f64_mulAdd(vs1, vs2, accd);
}
)