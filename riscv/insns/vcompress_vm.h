require_extension('V');
// FIXME: should implement ncp access trap
if (unlikely((insn.rs2() == insn.rd()) || (insn.rs1() == insn.rd())))
  throw trap_ncp_rvv_invalid_same_rdrs();

check_vstart{
	int vd_idx = 0;
        switch(SEW) {
                case 8:
			vector_for_each(idx){
				if (VRS1.vb[idx] & 0x1) {
				    WRITE_VRD_B(VRS2.vb[idx], vd_idx++);
				}
			}
                break;

                case 16:
			vector_for_each(idx){
				if (VRS1.vh[idx] & 0x1) {
				    WRITE_VRD_H(VRS2.vh[idx], vd_idx++);
				}
			}
                break;

                default:
                break;
        }
}

