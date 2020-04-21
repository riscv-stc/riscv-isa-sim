require_extension('V');
// FIXME: should implement ncp access trap

reg_t sum = 0;
check_vstart{
        switch(SEW) {
                case 8:
			vector_for_each_from_zero(idx) {
				if (idx < VL) {
					check_v0bmask(idx);
					sum += (VRS2.vb[idx] & 0x1);
				}
			}
			WRITE_RD(sum);
                break;

                case 16:
			vector_for_each_from_zero(idx) {
				if (idx < VL) {
					check_v0hmask(idx);
					sum += (VRS2.vh[idx] & 0x1);
				}
			}
			WRITE_RD(sum);
                break;

                default:
                break;
        }
}

