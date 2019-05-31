require_extension('V');

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