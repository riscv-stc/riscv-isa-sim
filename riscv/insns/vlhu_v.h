require_extension('V');
check_vstart{
	switch(SEW){
	case 8:
		abort();
		break;
	case 16:
		vector_for_each(idx){
			check_v0hmask(idx);
			WRITE_VRD_H(MMU.load_uint16(RS1+idx * 2), idx);
		}
		break;
	default:
		break;
	}
}
