require_extension('V');
// FIXME: should implement ncp access trap
check_vstart{
	switch(SEW){
	case 8:
		vector_for_each(idx){
			check_v0bmask(idx);
			int8_t tmp = MMU.load_int8(RS1+idx*SEW/8);
			if(tmp == 0)
				break;
			WRITE_VRD_B(tmp, idx);
			p->set_csr(CSR_VL, idx);
		}
		break;	
	case 16:
		vector_for_each(idx){
			check_v0hmask(idx);
			uint16_t tmp = MMU.load_int16(RS1+idx*SEW/8);
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
