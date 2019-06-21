require_extension('V');
check_vstart{
	Vcompare<half, unsigned short, unsigned short> vcompare;
	vcompare.vmford_vv((half*)&VRS2.vh[VSTART], (half*)&VRS1.vh[VSTART], (unsigned short*)&VRD.vh[VSTART], VM, VR0.vh, VL);
}
