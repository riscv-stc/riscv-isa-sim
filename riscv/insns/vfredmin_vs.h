require_extension('V');
check_ncp_vill_invalid()
// FIXME: should implement ncp access trap
check_vstart{
	Vfred<half, unsigned short> vfred;
	vfred.vfredmin_vs((half*)&VRS2.vh[VSTART], (half*)&VRS1.vh[VSTART], (half*)&VRD.vh[VSTART], VM, VR0.vh, VL);
}
