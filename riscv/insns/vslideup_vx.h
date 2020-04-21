require_extension('V');
// FIXME: should implement ncp access trap

uint32_t start = VSTART > RS1 ? VSTART : RS1;
check_vstart{
	switch(SEW) {
		case 8:
			vector_for_each_from_zero(idx) {
				if (idx >= 0 && idx < start)
				    continue;
				else if (idx >= start && idx < VL) {
				    check_v0bmask(idx);
				    WRITE_VRD_B(VRS2.vb[idx - RS1], idx);
				}
				else {
					WRITE_VRD_B(0, idx);
				}
			}
		break;

		case 16:
			vector_for_each_from_zero(idx) {
				if (idx >= 0 && idx < start)
				    continue;
				else if (idx >= start && idx < VL) {
				    check_v0hmask(idx);
				    WRITE_VRD_H(VRS2.vh[idx - RS1], idx);
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
