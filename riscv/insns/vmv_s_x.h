require_extension('V');
NCP_AUNIT();

check_vstart{
	check_vl();

        switch(SEW) {
                case 8:
			vector_for_each_no_lmlu(idx) {
				if (idx == 0) {
				    if (xlen < SEW)
				        WRITE_VRD_B(0, idx);
				    WRITE_VRD_B(RS1, idx);
				}
				else {
				    WRITE_VRD_B(0, idx);
				}
			}
                break;

                case 16:
			vector_for_each_no_lmlu(idx) {
				if (idx == 0) {
				    if (xlen < SEW)
				        WRITE_VRD_H(0, idx);
				    WRITE_VRD_H(RS1, idx);
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
