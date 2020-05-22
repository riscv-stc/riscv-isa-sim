require_extension('V');
check_ncp_vill_invalid()

check_vstart{
	switch(SEW){
	case 16:
		check_rvv_misaligned_base(RS1, int16)
		check_rvv_access(RS1, VL * SEW/8)

		vector_for_each(idx){
			check_v0hmask(idx);
			MMU.store_uint16(RS1+idx*SEW/8, VRS3.vh[idx]);
		}
		break;
	default:
		break;
	}

	WRITE_RS1(RS1 + RS2);
}