// vfcvt.rtz.x.f.v vd, vd2, vm
require(P.VU.vstart == 0);

VI_VFP_VF_LOOP
({
  check_tcp_csr_bf16(STATE.bf16);
},
{
  P.VU.elt<int16_t>(rd_num, i) = f16_to_i16(vs2, softfloat_round_minMag, true);
},
{
  P.VU.elt<int32_t>(rd_num, i) = f32_to_i32(vs2, softfloat_round_minMag, true);
},
{
  P.VU.elt<int64_t>(rd_num, i) = f64_to_i64(vs2, softfloat_round_minMag, true);
})
