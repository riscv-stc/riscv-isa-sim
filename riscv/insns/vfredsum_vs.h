require_extension('V');
// FIXME: should implement ncp access trap
check_vstart{
	Vfred<half, unsigned short> vfred;
	vfred.vfredsum_vs((half*)&VRS2.vh[VSTART], (half*)&VRS1.vh[VSTART], (half*)&VRD.vh[VSTART], VM, VR0.vh, VL);
}
