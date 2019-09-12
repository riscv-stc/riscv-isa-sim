require_extension('V');
NCP_AUNIT();
check_vstart{
	Vmerge<half, half, unsigned short> vmerge;
	vmerge.vmerge_vvm((half*)&VRS2.vh[VSTART], (half*)&VRS1.vh[VSTART], (half*)&VRD.vh[VSTART], VM, VR0.vh, VL);
}
