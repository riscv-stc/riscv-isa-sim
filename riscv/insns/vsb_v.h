require_extension('V');
check_ncp_vill_invalid()

check_vstart{
	switch(SEW){
	case 8:
		check_rvv_misaligned_base(RS1, int8)
		check_rvv_access(RS1, VL * SEW/8)

		vector_for_each(idx){
			check_v0bmask(idx);
			MMU.store_uint8(RS1+idx, VRS3.vb[idx]);
		}
		break;
	case 16:
		abort();
		break;
	default:
		break;
	}
}
