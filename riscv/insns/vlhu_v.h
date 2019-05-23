require_extension('V');
if(VSTART >= VL){
	VSTART = 0;
}
else{
	unsigned int idx = VSTART;
	switch(SEW){
	case 16:
		for(;idx < VL; idx++){
			if(!VM & !(READ_VREG(0).vh[idx]))
				continue;
			WRITE_VRD_H(MMU.load_uint16(RS1+idx*(SEW/8)), idx);
		}
		break;
	default:
		break;
	}
}
