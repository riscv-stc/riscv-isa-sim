require_extension('V');
NCP_AUNIT();
check_vstart{
	switch(SEW){
	case 16:
		vector_for_each(idx){
			check_v0hmask(idx);
			WRITE_VRD_H(MMU.load_int16(RS1+idx*VRS2.vh[idx]), idx);
		}
		break;
	default:
		break;
	}
}
