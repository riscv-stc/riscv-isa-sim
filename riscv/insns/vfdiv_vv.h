// vfdiv.vv  vd, vs2, vs1
require(P.VU.vstart == 0);

VI_VFP_VV_LOOP
({
  check_tcp_csr_bf16(STATE.bf16);
},
{
  vd = f16_div(vs2, vs1);
},
{
  vd = f32_div(vs2, vs1);
},
{
  vd = f64_div(vs2, vs1);
})
