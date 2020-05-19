require_extension('V');
check_ncp_vill_invalid()
// FIXME: should implement ncp access trap

check_vstart{
        switch(SEW) {
                case 8:
			vector_for_each_from_zero(idx) {
				if (idx < VL) {
				    check_v0bmask(idx);
				    WRITE_VRD_B(idx, idx);
				}
				else
				    WRITE_VRD_B(0, idx);
			}
                break;

                case 16:
			vector_for_each_from_zero(idx) {
				if (idx < VL) {
				    check_v0hmask(idx);
				    WRITE_VRD_H(idx, idx);
				}
				else
				    WRITE_VRD_H(0, idx);
			}
                break;

                default:
                break;
        }
}
