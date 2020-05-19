require_extension('V');
check_ncp_vill_invalid()
check_rvv_misaligned_base(RS1, int16)
check_rvv_access(RS1, VL * SEW/8)

check_vstart{
	switch(SEW){
	case 8:
		abort();
		break;
	case 16:
		vector_for_each(idx){
			check_v0hmask(idx);
			WRITE_VRD_H(MMU.load_int16(RS1+idx * 2), idx);
		}
		break;
	default:
		break;
	}
}
