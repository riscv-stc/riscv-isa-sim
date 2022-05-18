// vfclass.v vd, vs2, vm
require(P.VU.vstart == 0);

VI_VFP_V_LOOP
({
  check_tcp_csr_bf16(STATE.bf16);
},
{
  vd = f16_recip7(vs2);
},
{
  vd = f32_recip7(vs2);
},
{
  vd = f64_recip7(vs2);
})
