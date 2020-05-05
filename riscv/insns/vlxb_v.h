require_extension('V');

check_vstart{
	switch(SEW){
	case 16:
		check_rvv_misaligned_base(RS1, int8)

		vector_for_each(idx){
			check_v0bmask(idx);

			check_rvv_misaligned_offset(VRS2.vh[idx], int8)
			check_rvv_access(RS1+VRS2.vh[idx], SEW/8)

			WRITE_VRD_H((int16_t)MMU.load_int8(RS1+VRS2.vh[idx]), idx);
		}
		break;
	default:
		break;
	}
}
