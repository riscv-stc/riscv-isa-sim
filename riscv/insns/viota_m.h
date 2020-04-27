require_extension('V');
// FIXME: should implement ncp access trap

if (unlikely((insn.rd() == insn.rs1())
	|| (insn.rd() == insn.rs2())
	|| (insn.vm() == 0 &&(insn.rd() == 0)))) {
	throw trap_ncp_rvv_invalid_same_rdrs();
	}
int sum = 0;
vector_for_each_from_zero(idx) {
	switch(SEW) {
	case 8:
		if (idx < VL) {
			if (v0b_mask(idx)) {
			    continue;
			}
			else if (idx == 0) {
			    WRITE_VRD_B(0, idx);
			}
			else {
			    WRITE_VRD_B((char)sum, idx);
			}
			sum += VRS2.vb[idx] & 0x1;
		}
		else {
			    WRITE_VRD_B((char)0, idx);
		}

	break;

	case 16:
		if (idx < VL) {
			if (v0h_mask(idx)) {
				continue;
			}
			else if (idx == 0) {
			    WRITE_VRD_H(0, idx);
			}
			else {
			    WRITE_VRD_H((short)sum, idx);
			}
			sum += VRS2.vh[idx] & 0x1;
		}
		else {
			    WRITE_VRD_H((short)0, idx);
		}
	break;

	default:
	break;
	}
}
