require_extension('V');

int flag = 0;
check_vstart{
        switch(SEW) {
		case 8:
			vector_for_each_from_zero(idx) {
				check_v0bmask(idx);
				if (idx < VL && (flag == 0) && ((VRS2.vb[idx] & 0x1 )== 1)) {
				    flag = 1;
				    WRITE_VRD_B(1, idx);
				}
				else {
				    WRITE_VRD_B(0, idx);
				}
			}
		break;

		case 16:
			vector_for_each_from_zero(idx) {
				check_v0hmask(idx);
				if (idx < VL && (flag == 0) && ((VRS2.vh[idx] & 0x1) == 1)) {
				    flag = 1;
				    WRITE_VRD_H(1, idx);
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
