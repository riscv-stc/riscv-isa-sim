require_extension('V');
NCP_AUNIT();
check_vstart{
	Vfred<half, unsigned short> vfred;
	vfred.vfredmax_vs((half*)&VRS2.vh[VSTART], (half*)&VRS1.vh[VSTART], (half*)&VRD.vh[VSTART], VM, VR0.vh, VL);
}
