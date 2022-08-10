// vfcvt.f.xu.v vd, vd2, vm
require(P.VU.vstart == 0);

VI_VFP_VF_LOOP
({
  check_tcp_csr_bf16(STATE.bf16);
},
{
  auto vs2_u = P.VU.elt<uint16_t>(rs2_num, i);
  vd = ui32_to_f16(vs2_u);
},
{
  auto vs2_u = P.VU.elt<uint32_t>(rs2_num, i);
  vd = ui32_to_f32(vs2_u);
},
{
  auto vs2_u = P.VU.elt<uint64_t>(rs2_num, i);
  vd = ui64_to_f64(vs2_u);
})
