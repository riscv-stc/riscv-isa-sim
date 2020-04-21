require_extension('V');
// FIXME: should implement ncp access trap

check_vstart{
	switch(SEW) {
		case 8:
			vector_for_each(idx){
				if (idx < VL) {
				    check_v0bmask(idx);
				    WRITE_VRD_B((idx + VUIMM) < VLMAX ? VRS2.vb[idx + VUIMM] : 0, idx);
				}
				else if (idx >= VL && idx < VLMAX)
					WRITE_VRD_B(0, idx);
			}
		break;

		case 16:
			vector_for_each(idx){
				if (idx < VL) {
				    check_v0hmask(idx);
				    WRITE_VRD_H((idx + VUIMM) < VLMAX ? VRS2.vh[idx + VUIMM] : 0, idx);
				}
				else if (idx >= VL && idx < VLMAX)
					WRITE_VRD_H(0, idx);
			}

		break;
		default:
		break;
	}

}
