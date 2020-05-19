require_extension('V');
check_ncp_vill_invalid()
// FIXME: should implement ncp access trap

if (unlikely((insn.rs2() == insn.rd()) || (insn.vm() == 0 && ((p->get_csr(CSR_VTYPE) & 0x3) > 0) && insn.vm() == insn.rd())))
  throw trap_ncp_rvv_invalid_same_rdrs();

uint32_t start = VSTART > 1 ? VSTART : 1;

check_vstart{
	switch(SEW) {
		case 8:
			vector_for_each(idx) {
				if (idx == VSTART && idx == 0) {
				    check_v0bmask(idx);

				    if (xlen < SEW)
					WRITE_VRD_B(0, idx);
				    WRITE_VRD_B(RS1, idx);
				}
				else if (idx >= start && idx < VL) {
				    check_v0bmask(idx);
				    WRITE_VRD_B(VRS2.vb[idx - 1], idx);
				}
				else {
				    WRITE_VRD_B(0, idx);
				}
			}
		break;

		case 16:
			vector_for_each(idx) {
				if (idx == VSTART && idx == 0) {
				    check_v0hmask(idx);
				    if (xlen < SEW)
					WRITE_VRD_H(0, idx);
				    WRITE_VRD_H(RS1, idx);
				}
				else if (idx >= start && idx < VL) {
				    check_v0hmask(idx);
				    WRITE_VRD_H(VRS2.vh[idx - 1], idx);
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
