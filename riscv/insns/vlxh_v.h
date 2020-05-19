require_extension('V');
check_ncp_vill_invalid()

check_vstart{
	switch(SEW){
	case 16:
		check_rvv_misaligned_base(RS1, int16)

		vector_for_each(idx){
			check_rvv_misaligned_offset(VRS2.vh[idx], int16)
			check_rvv_access(RS1+VRS2.vh[idx], SEW/8)
            check_v0hmask(idx);

			WRITE_VRD_H(MMU.load_int16(RS1+VRS2.vh[idx]), idx);
		}
		break;
	default:
		break;
	}
}
