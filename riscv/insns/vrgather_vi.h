require_extension('V');

check_vstart{
	switch(SEW) {
		case 8:
			vector_for_each(idx){
				if (idx < VL) {
				    check_v0bmask(idx);
				    WRITE_VRD_B((VUIMM >= VLMAX) ? 0 : VRS2.vb[VUIMM]);
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
				    WRITE_VRD_H((VUIMM >= VLMAX) ? 0 : VRS2.vh[VUIMM]);
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
