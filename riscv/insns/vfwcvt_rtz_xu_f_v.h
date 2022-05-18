// vfwcvt.rtz,xu.f.v vd, vs2, vm
require(P.VU.vstart == 0);

VI_VFP_CVT_SCALE
({
  ;
},
{
  check_tcp_csr_bf16(STATE.bf16);
},
{
  auto vs2 = P.VU.elt<float16_t>(rs2_num, i);
  P.VU.elt<uint32_t>(rd_num, i, true) = f16_to_ui32(vs2, softfloat_round_minMag, true);
},
{
  auto vs2 = P.VU.elt<float32_t>(rs2_num, i);
  P.VU.elt<uint64_t>(rd_num, i, true) = f32_to_ui64(vs2, softfloat_round_minMag, true);
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
true, (P.VU.vsew >= 16))
