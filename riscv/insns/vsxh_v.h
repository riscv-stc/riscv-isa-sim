require_extension('V');
NCP_AUNIT();
check_vstart{
	switch(SEW){
	case 16:
		vector_for_each(idx){
			check_v0hmask(idx);
			MMU.store_uint16(RS1+VRS2.vh[idx], VRS3.vh[idx]);
		}
		break;
	default:
		break;
	}
}
