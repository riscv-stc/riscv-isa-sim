require_extension('V');
check_ncp_vill_invalid()
// FIXME: should implement ncp access trap
check_vstart{
	Vfsqrt<half, unsigned short> vfsqrt;
	vfsqrt.vfsqrt_v((half*)&VRS2.vh[VSTART], (half*)&VRD.vh[VSTART], VM, VR0.vh, VL);
}
