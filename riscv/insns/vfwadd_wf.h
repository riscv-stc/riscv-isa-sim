// vfwadd.wf vd, vs2, vs1
require(P.VU.vstart == 0);

VI_VFP_WF_LOOP_WIDE
({
  vd = f32_add(vs2, rs1);
},
{
  vd = f64_add(vs2, rs1);
})
