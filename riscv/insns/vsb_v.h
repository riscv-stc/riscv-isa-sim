require_extension('V');
check_vstart{
	switch(SEW){
	case 16:
		vector_for_each(idx){;
			check_v0hmask(idx)
			MMU.store_uint8(RS1+idx*SEW/8, VRS3.vh[idx]);
		}
		break;
	default:
		break;
	}
}
