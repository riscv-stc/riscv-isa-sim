require_extension('V');
NCP_AUNIT();
check_vstart{
	vector_for_each(idx){
		switch(SEW){
			case 8:
			break;
			case 16:
				vdh_clear_exceed(idx){
					WRITE_VRD_H((VRS2.vh[idx] & 0x1) || (!(VRS1.vh[idx] & 0x1)), idx);
				}
			break;
		}

	}
}
