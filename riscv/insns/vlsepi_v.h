require_extension('V');
// FIXME: should implement ncp access trap
check_vstart{
	switch(SEW){
	case 16:
		vector_for_each(idx){
			check_v0hmask(idx);
			WRITE_VRD_H((int16_t)MMU.load_int16(RS1+idx*RS2), idx);
		}
		break;
	default:
		break;
	}

	WRITE_RS1(RS1 + VL * RS2);
}
