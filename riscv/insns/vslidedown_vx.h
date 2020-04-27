require_extension('V');
// FIXME: should implement ncp access trap
if (unlikely((insn.rs2() == insn.rd()) || (insn.vm() == 0 && ((p->get_csr(CSR_VTYPE) & 0x3) > 0) && insn.vm() == insn.rd())))
  throw trap_ncp_rvv_invalid_same_rdrs();

check_vstart{
	switch(SEW) {
		case 8:
			vector_for_each(idx){
				if (idx < VL) {
				    check_v0bmask(idx);
				    WRITE_VRD_B((idx + RS1) < VLMAX ? VRS2.vb[idx + RS1] : 0, idx);
				}
				else if (idx >= VL && idx < VLMAX)
					WRITE_VRD_B(0, idx);
			}
		break;

		case 16:
			vector_for_each(idx){
				if (idx < VL) {
				    check_v0hmask(idx);
				    WRITE_VRD_H((idx + RS1) < VLMAX ? VRS2.vh[idx + RS1] : 0, idx);
				}
				else if (idx >= VL && idx < VLMAX)
				    WRITE_VRD_H(0, idx);
			}

		break;
		default:
		break;
	}

}
