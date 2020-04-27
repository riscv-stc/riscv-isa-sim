require_extension('V');
// FIXME: should implement ncp access trap
if (unlikely((insn.rs1() == insn.rd()) || (insn.rs2() == insn.rd()) || (insn.vm() == 0 && insn.vm() == insn.rd())))
  throw trap_ncp_rvv_invalid_same_rdrs();

/* TODO: VS1 byte or half ? */
check_vstart{
	switch(SEW) {
		case 8:
			vector_for_each(idx){
				if (idx < VL) {
				    check_v0bmask(idx);
				    WRITE_VRD_B((VRS1.vb[idx] >= VLMAX) ? 0 : VRS2.vb[VRS1.vb[idx]], idx);
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
				    WRITE_VRD_H((VRS1.vh[idx] >= VLMAX) ? 0 : VRS2.vh[VRS1.vh[idx]], idx);
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
