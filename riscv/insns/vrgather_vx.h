require_extension('V');
NCP_AUNIT();

check_vstart{
	switch(SEW) {
		case 8:
			vector_for_each(idx){
				if (idx < VL) {
				    check_v0bmask(idx);
				    WRITE_VRD_B((RS1 >= VLMAX) ? 0 : VRS2.vb[RS1], idx);
				}
				else {
				    WRITE_VRD_B(0, idx);
				}
			}
		break;

		case 16:
			vector_for_each(idx){
				if (idx < VL) {
				    check_v0hmask(idx);
				    WRITE_VRD_H((RS1 >= VLMAX) ? 0 : VRS2.vh[RS1], idx);
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
