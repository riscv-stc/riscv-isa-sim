// vfwnmacc.vf vd, vs2, rs1
require(P.VU.vstart == 0);

VI_VFP_VF_LOOP_WIDE
({
  vd = f32_mulAdd(f32(rs1.v ^ F32_SIGN), vs2, vd);
},
{
  vd = f64_mulAdd(f64(rs1.v ^ F64_SIGN), vs2, vd);
})
