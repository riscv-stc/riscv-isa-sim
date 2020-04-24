require_extension('V');
// FIXME: should implement ncp access trap

check_vstart{
	check_vl();

        switch(SEW) {
                case 8:
				    if (xlen < SEW)
				        WRITE_VRD_B(0, 0);
				    WRITE_VRD_B(RS1, 0);
                break;

                case 16:
				    if (xlen < SEW)
				        WRITE_VRD_H(0, 0);
				    WRITE_VRD_H(RS1, 0);
                break;

                default:
                break;
        }
}
