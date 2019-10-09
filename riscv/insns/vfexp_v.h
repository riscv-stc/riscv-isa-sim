require_extension('V');
NCP_AUNIT();
check_vstart{
	Vfexp<half, unsigned short> vfexp;
	vfexp.vfexp_v((half*)&VRS2.vh[VSTART], (half*)&VRD.vh[VSTART], VM, VR0.vh, VL);
}
