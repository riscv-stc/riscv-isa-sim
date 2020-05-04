require_extension('V');

check_vstart{
	switch(SEW){
	case 16:
		check_rvv_misaligned_base(RS1, int16)

		vector_for_each(idx){
			check_v0hmask(idx);

			check_rvv_misaligned_offset(VRS2.vh[idx], int16)
			check_rvv_access(RS1+VRS2.vh[idx], SEW/8)

			MMU.store_uint16(RS1+VRS2.vh[idx], VRS3.vh[idx]);
		}
		break;
	default:
		break;
	}
}
