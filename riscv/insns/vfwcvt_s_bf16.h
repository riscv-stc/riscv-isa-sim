// vfwcvt.f.f.v vd, vs2, vm
VI_VFP_CVT_SCALE
({
  ;
},
{
  auto vs2 = P.VU.elt<bfloat16_t>(rs2_num, i);
  P.VU.elt<float32_t>(rd_num, i, true) = bf16_to_f32(vs2);
},
{
  ;
},
{
  ;
},
{
  ;
},
{
  require(p->supports_extension(EXT_ZFH));
},
{
  ;
},
true, (P.VU.vsew == 16))