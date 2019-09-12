require_extension('V');
NCP_AUNIT();
check_vstart{
	Vma<half, unsigned short> vma;
	vma.vmin_vv((half*)&VRS2.vh[VSTART], (half*)&VRS1.vh[VSTART], (half*)&VRD.vh[VSTART], VM, VR0.vh, VL);
}
