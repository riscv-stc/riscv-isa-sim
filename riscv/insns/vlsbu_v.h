require_extension('V');
check_vstart{
	switch(SEW){
	case 16:
		vector_for_each(idx){
			check_v0bmask(idx);
			WRITE_VRD_H(MMU.load_uint8(RS1+idx*RS2), idx);
		}
		break;
	default:
		break;
	}
}
