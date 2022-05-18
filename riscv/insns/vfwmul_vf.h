// vfwmul.vf vd, vs2, rs1
require(P.VU.vstart == 0);

VI_VFP_VF_LOOP_WIDE
({
  vd = f32_mul(vs2, rs1);
},
{
  vd = f64_mul(vs2, rs1);
})
