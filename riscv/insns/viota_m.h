require_extension('V');

int sum = 0;
vector_for_each_from_zero(idx) {
	switch(SEW) {
	case 8:
		if (idx == 0) {
		    WRITE_VRD_B(0, idx);
		}
		else if (v0b_mask(idx)) {
		    continue;
		}
		else {
		    sum += (v0b_mask(idx) ? 0 : VRS2.vb[idx-1]);
		    WRITE_VRD_B((char)sum, idx);
		}

	break;

	case 16:
		if (idx == 0) {
		    WRITE_VRD_H(0, idx);
		}
		else if (v0h_mask(idx)) {
		    continue;
		}
		else {
		    sum += (v0h_mask(idx) ? 0 : VRS2.vh[idx-1]);
		    WRITE_VRD_H((short)sum, idx);
		}
	break;

	default:
	break;
	}
}
