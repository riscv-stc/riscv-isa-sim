require_extension('V');

/* TODO: VS1 byte or half ? */
check_vstart{
	switch(SEW) {
		case 8:
			vector_for_each(idx){
				if (idx < VL) {
				    check_v0bmask(idx);
				    WRITE_VRD_B((VRS1.vb[idx] >= VLMAX) ? 0 : VRS2.vb[VRS1.vb[idx]]);
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
				    WRITE_VRD_H((VRS1.vh[idx] >= VLMAX) ? 0 : VRS2.vh[VRS1.vh[idx]]);
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
