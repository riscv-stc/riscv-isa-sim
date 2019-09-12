require_extension('V');
NCP_AUNIT();
check_vstart{
	switch(SEW){
	case 16:
		vector_for_each(idx){
			check_v0hmask(idx);
			WRITE_VRD_H((uint16_t)MMU.load_uint8(RS1+VRS2.vh[idx]), idx);
		}
		break;
	default:
		break;
	}
}
