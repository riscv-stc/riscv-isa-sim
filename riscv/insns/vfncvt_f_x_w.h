// vfncvt.f.x.v vd, vs2, vm
require(P.VU.vstart == 0);

VI_VFP_CVT_SCALE
({
  ;
},
{
  check_tcp_csr_bf16(STATE.bf16);
},
{
  auto vs2 = P.VU.elt<int32_t>(rs2_num, i);
  P.VU.elt<float16_t>(rd_num, i, true) = i32_to_f16(vs2);
},
{
  auto vs2 = P.VU.elt<int64_t>(rs2_num, i);
  P.VU.elt<float32_t>(rd_num, i, true) = i64_to_f32(vs2);
},
{
  ;
},
{
  require(p->supports_extension(EXT_ZFH));
},
{
  require(p->supports_extension('F'));
},
false, (P.VU.vsew >= 16))
