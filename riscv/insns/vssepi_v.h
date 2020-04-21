require_extension('V');
// FIXME: should implement ncp access trap
check_vstart{
	switch(SEW){
	case 16:
		vector_for_each(idx){
			check_v0hmask(idx);
			MMU.store_uint16(RS1+idx*RS2, VRS3.vh[idx]);
		}
		break;
	default:
		break;
	}

	WRITE_RS1(RS1 + VL * RS2);
}

