require_extension('V');
check_ncp_vill_invalid()
// FIXME: should implement ncp access trap

unsigned int index = 0;

check_vstart{
        switch(SEW) {
			int idx;
                case 8:
			for (idx = 0; idx < VL; idx++) {
				check_v0bmask(idx);

				if (idx < VL &&  VRS2.vb[idx] & 0x1) {
				    WRITE_RD(idx);
				    break;
				}
			}
			if (idx == VL)
			{
				WRITE_RD(-1);
			}
                break;

                case 16:
			for (idx = 0; idx < VL; idx++) {
				check_v0hmask(idx);

				if (idx < VL && VRS2.vh[idx] & 0x1) {
					WRITE_RD(idx);
					break;
				}
			}
			if (idx == VL)
			{
				WRITE_RD(-1);
			}
                break;

                default:
                break;
        }
	if (index >= VLMAX) {
	    WRITE_RD(-1);
	}
}
