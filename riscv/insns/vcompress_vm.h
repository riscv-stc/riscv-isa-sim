require_extension('V');
NCP_AUNIT();

check_vstart{
	int vd_idx = 0;
        switch(SEW) {
                case 8:
			vector_for_each(idx){
				if (VRS1.vb[idx] & 0x1) {
				    WRITE_VRD_B(VRS2.vb[idx], vd_idx++);
				}
			}
                break;

                case 16:
			vector_for_each(idx){
				if (VRS1.vh[idx] & 0x1) {
				    WRITE_VRD_H(VRS2.vh[idx], vd_idx++);
				}
			}
                break;

                default:
                break;
        }
}

