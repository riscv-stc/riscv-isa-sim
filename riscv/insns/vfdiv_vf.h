// vfdiv.vf vd, vs2, rs1
require(P.VU.vstart == 0);

VI_VFP_VF_LOOP
({
  check_tcp_csr_bf16(STATE.bf16);
},
{
  vd = f16_div(vs2, rs1);
},
{
  vd = f32_div(vs2, rs1);
},
{
  vd = f64_div(vs2, rs1);
})
