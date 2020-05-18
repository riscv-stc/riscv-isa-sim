require_extension('V');

check_vstart{
	switch(SEW){
	case 16:
		check_rvv_misaligned_base(RS1, int8)

		vector_for_each(idx){
			check_rvv_misaligned_offset(VRS2.vh[idx], int8)
			check_rvv_access(RS1+VRS2.vh[idx], SEW/8)
            check_v0bmask(idx);

			WRITE_VRD_H((uint16_t)MMU.load_uint8(RS1+VRS2.vh[idx]), idx);
		}
		break;
	default:
		break;
	}
}
