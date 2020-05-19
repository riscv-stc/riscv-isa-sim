require_extension('V');
check_ncp_vill_invalid()
// FIXME: should implement ncp access trap
check_vstart{
	switch(SEW){
	case 8:
		check_rvv_misaligned_base(RS1, int8)
		check_rvv_access(RS1, VL * SEW/8)

		vector_for_each(idx){
			check_v0bmask(idx);
			WRITE_VRD_B(MMU.load_int8(RS1+idx*SEW/8), idx);
		}
	case 16:
		check_rvv_misaligned_base(RS1, int16)
		check_rvv_access(RS1, VL * SEW/8)

		vector_for_each(idx){
			check_v0hmask(idx);
			WRITE_VRD_H(MMU.load_int16(RS1+idx*SEW/8), idx);
		}
		break;
	default:
		break;
	}

	WRITE_RS1(RS1 + RS2);
}
