require_extension('V');
check_vstart{
	switch(SEW){
	case 8:
		vector_for_each(idx){
			check_v0bmask(idx);
			WRITE_VRD_B(MMU.load_uint8(RS1+idx*SEW/8), idx);
		}
	case 16:
		vector_for_each(idx){
			check_v0hmask(idx);
			WRITE_VRD_H(MMU.load_uint16(RS1+idx*SEW/8), idx);
		}
		break;
	default:
		break;
	}
}