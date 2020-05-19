require_extension('V');
check_ncp_vill_invalid()

check_vstart{
	switch(SEW){
	case 16:
		check_rvv_misaligned_base(RS1, int8)
		check_rvv_misaligned_offset(RS2, int8)
		check_rvv_access(RS1, VL * RS2 * SEW/8)

		vector_for_each(idx){
			check_v0bmask(idx);
			WRITE_VRD_H(MMU.load_uint8(RS1+idx*RS2), idx);
		}
		break;
	default:
		break;
	}
}
