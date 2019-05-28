require_extension('V');
check_vstart{
	switch(SEW){
	case 8:
		vector_for_each(idx){
			check_v0bmask(idx);
			MMU.store_uint8(RS1+idx*RS2, VRS3.vb[idx]);
		}
		break;
	case 16:
		abort();
		break;
	default:
		break;
	}
}
