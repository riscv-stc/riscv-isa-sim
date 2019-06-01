require_extension('V');
check_vstart{
	vector_for_each(idx){
		switch(SEW){
			case 8:
			break;
			case 16:
				vdh_clear_exceed(idx){
					WRITE_VRD_H(~(VRS1.vh[idx] ^ VRS2.vh[idx]), idx);
				}
			break;
		}

	}
}
