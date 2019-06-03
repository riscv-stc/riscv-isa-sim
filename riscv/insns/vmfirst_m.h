require_extension('V');

unsigned int index = 0;

check_vstart{
        switch(SEW) {
                case 8:
			vector_for_each_from_zero(idx) {
				index = idx;
				check_v0bmask(idx);

				if (VRS2.vb[idx]) {
				    WRITE_RD(idx);
				    break;
				}
			}
                break;

                case 16:
			vector_for_each_from_zero(idx) {
				index = idx;
				check_v0hmask(idx);

				if (VRS2.vh[idx]) {
					WRITE_RD(idx);
					break;
				}
			}
                break;

                default:
                break;
        }
	if (index >= VLMAX) {
	    WRITE_RD(-1);
	}
}
