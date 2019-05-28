require_extension('V');
check_vstart{
	Vext<half> vext;
	vext.vext_x_v((half*)&VRS2.vh[VSTART], (half*)&VRD.vh[VSTART], (unsigned short)RS1, VL);
}
