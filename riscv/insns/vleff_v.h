require_extension('V');
// FIXME: should implement ncp access trap
check_vstart{
	switch(SEW){
	case 8:
		check_rvv_misaligned_base(RS1, int8)
		check_rvv_access(RS1, SEW/8)

		vector_for_each(idx){
			if(check_rvv_access_without_exception(RS1+idx * 2, SEW/8)){
				reg_t vl = p->get_csr(CSR_VL);
				vl = idx;
				p->set_csr(CSR_VL, idx);
				break;
			}
			check_v0hmask(idx);
			int16_t tmp = (int16_t)MMU.load_int8(RS1+idx * 2);
			WRITE_VRD_H(tmp, idx);
		}
		break;	
	case 16:
		check_rvv_misaligned_base(RS1, int16)
		check_rvv_access(RS1, SEW/8);

		vector_for_each(idx){
			if(check_rvv_access_without_exception(RS1+idx * 2, SEW/8)){
				reg_t vl = p->get_csr(CSR_VL);
				vl = idx;
				p->set_csr(CSR_VL, idx);
				break;
			}
			check_v0hmask(idx);
			int16_t tmp = (int16_t)MMU.load_int16(RS1+idx * 2);
			WRITE_VRD_H(tmp, idx);
		}
		break;
	default:
		break;
	}
}
