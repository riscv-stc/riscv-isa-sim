VI_VFP_CVT_SCALE
({
  ;
},
{
  auto vs2 = P.VU.elt<float32_t>(rs2_num, i);
  P.VU.elt<bfloat16_t>(rd_num, i, true) = f32_to_bf16(vs2);
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