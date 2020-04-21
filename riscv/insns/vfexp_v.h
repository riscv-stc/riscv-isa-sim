require_extension('V');
// FIXME: should implement ncp access trap
check_vstart{
	Vfexp<half, unsigned short> vfexp;
	vfexp.vfexp_v((half*)&VRS2.vh[VSTART], (half*)&VRD.vh[VSTART], VM, VR0.vh, VL);
}
