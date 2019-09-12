require_extension('V');
NCP_AUNIT();

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
			    sum += (v0b_mask(idx) ? 0 : (VRS2.vb[idx-1] & 0x1));
			    WRITE_VRD_B((char)sum, idx);
			}
		}
		else {
			    WRITE_VRD_B((char)0, idx);
		}

	break;

	case 16:
		if (idx < VL) {
			if (idx == 0) {
			    WRITE_VRD_H(0, idx);
			}
			else if (v0h_mask(idx)) {
			    continue;
			}
			else {
			    sum += (v0h_mask(idx) ? 0 : (VRS2.vh[idx-1] & 0x1));
			    WRITE_VRD_H((short)sum, idx);
			}
		}
		else {
			    WRITE_VRD_H((short)0, idx);
		}
	break;

	default:
	break;
	}
}
