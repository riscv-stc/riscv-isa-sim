require_extension('V');

check_vstart{
	switch(SEW) {
		case 8:
			vector_for_each(idx){
				if (idx < VL) {
				    check_v0bmask(idx);
				    WRITE_VRD_B(VRS2.vh[idx + 1], idx);
				}
				else if (idx == (VL -1)) {
				    check_v0hmask(idx);
				    WRITE_VRD_B(xlen > SEW ? RS1 & ((0x1 << SEW) -1) : RS1, idx);
				}
				else if (idx >= VL && idx < VLMAX)
					WRITE_VRD_B(0, idx);
			}
		break;

		case 16:
			vector_for_each(idx){
				if (idx < VL - 1) {
				    check_v0hmask(idx);
				    WRITE_VRD_H(VRS2.vh[idx + 1], idx);
				}
				else if (idx == (VL -1)) {
				    check_v0hmask(idx);
				    WRITE_VRD_H(xlen > SEW ? RS1 & ((0x1 << SEW) -1) : RS1, idx);
				}
				else if (idx >= VL && idx < VLMAX)
				    WRITE_VRD_H(0, idx);
			}

		break;
		default:
		break;
	}

}
