require_extension('V');
check_ncp_vill_invalid()
check_rvv_misaligned_base(RS1, int8)
check_rvv_access(RS1, SEW/8)

check_vstart{
	switch(SEW){
	case 8:
		vector_for_each(idx){
			check_v0bmask(idx);
			uint8_t tmp = MMU.load_int8(RS1+idx);
			if(tmp == 0)
				break;
			WRITE_VRD_B(tmp, idx);
			p->set_csr(CSR_VL, idx);
		}
		break;	
	case 16:
		vector_for_each(idx){
			check_v0hmask(idx);
			uint8_t tmp = MMU.load_int8(RS1+idx);
			if(tmp == 0)
				break;
			WRITE_VRD_H(tmp, idx);
			p->set_csr(CSR_VL, idx);
		}
		break;
	default:
		break;
	}
}
