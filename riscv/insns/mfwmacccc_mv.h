MXU_MMACC_MVFP_WIDEN(
1,
{
  accd = f32_mulAdd(accd, vs1, vs2);
},
{
  accd = f64_mulAdd(accd, vs1, vs2);
}
)