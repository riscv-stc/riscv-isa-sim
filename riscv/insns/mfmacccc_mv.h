MXU_MMACC_MVFP(
1,
{
  accd = f16_mulAdd(accd, vs1, vs2);
},
{
  accd = f32_mulAdd(accd, vs1, vs2);
},
{
  accd = f64_mulAdd(accd, vs1, vs2);
}
)