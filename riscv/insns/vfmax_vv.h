require_extension('V');
check_vstart{
	Vma<half, unsigned short> vma;
	vma.vmax_vv((half*)&VRS2.vh[VSTART], (half*)&VRS1.vh[VSTART], (half*)&VRD.vh[VSTART], VM, VR0.vh, VL);
}
