require_extension('V');
// FIXME: should implement ncp access trap
check_vstart{
	Vcompare<half, unsigned short, unsigned short> vcompare;
	vcompare.vle_vv((half*)&VRS2.vh[VSTART], (half*)&VRS1.vh[VSTART],&VRD.vh[VSTART], VM, VR0.vh, VL);
}
