require_extension('V');
check_vstart{
	switch(SEW){
	case 8:
		vector_for_each(idx){
			check_v0bmask(idx);
			WRITE_VRD_B(MMU.load_int8(RS1+idx), idx);
		}
	case 16:
		vector_for_each(idx){
			check_v0hmask(idx);
			WRITE_VRD_H((int16_t)MMU.load_int8(RS1+idx), idx);
		}
		break;
	default:
		break;
	}
}
