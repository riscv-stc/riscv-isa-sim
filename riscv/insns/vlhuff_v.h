require_extension('V');
// FIXME: should implement ncp access trap
check_vstart{
	switch(SEW){
	case 8:
		abort();
		break;	
	case 16:
		vector_for_each(idx){
			check_v0hmask(idx);
			uint16_t tmp = (uint16_t)MMU.load_uint16(RS1+idx * 2);
			if(tmp == 0) break;
			WRITE_VRD_H(tmp, idx);
			p->set_csr(CSR_VL, idx);
		}
		break;
	default:
		break;
	}
}
