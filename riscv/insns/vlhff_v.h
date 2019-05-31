require_extension('V');
check_vstart{
	switch(SEW){
	case 8:
		abort();
		break;	
	case 16:
		vector_for_each(idx){
			check_v0hmask(idx);
			int16_t tmp = (int16_t)MMU.load_int16(RS1+idx*SEW/8);
			if(tmp == 0) break;
			WRITE_VRD_H(tmp, idx);
			p->set_csr(CSR_VL, idx);
		}
		break;
	default:
		break;
	}
}