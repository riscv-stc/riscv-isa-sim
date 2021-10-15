// vfrdiv.vf vd, vs2, rs1, vm  # scalar-vector, vd[i] = f[rs1]/vs2[i]
VI_VFP_VF_LOOP
({
  check_tcp_csr_bf16(STATE.bf16);
},
{
  vd = f16_div(rs1, vs2);
},
{
  vd = f32_div(rs1, vs2);
},
{
  vd = f64_div(rs1, vs2);
})
