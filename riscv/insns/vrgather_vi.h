require_extension('V');
check_ncp_vill_invalid()
// FIXME: should implement ncp access trap
if (unlikely((insn.rs2() == insn.rd()) || (insn.vm() == 0 && insn.vm() == insn.rd())))
  throw trap_ncp_rvv_invalid_same_rdrs();

check_vstart{
	switch(SEW) {
		case 8:
			vector_for_each(idx){
				if (idx < VL) {
				    check_v0bmask(idx);
				    WRITE_VRD_B((VUIMM >= VLMAX) ? 0 : VRS2.vb[VUIMM], idx);
				}
				else {
				    WRITE_VRD_B(0, idx);
				}
			}
		break;

		case 16:
			vector_for_each(idx){
				if (idx < VL) {
				    check_v0hmask(idx);
				    WRITE_VRD_H((VUIMM >= VLMAX) ? 0 : VRS2.vh[VUIMM], idx);
				}
				else {
				    WRITE_VRD_H(0, idx);
				}
			}
		break;

		default:
		break;
	}
}
