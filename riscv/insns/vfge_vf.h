require_extension('V');
check_vstart{
	half vr1;
	vr1.x = (unsigned short)FRS1.v[0];
	Vcompare<half, unsigned short, unsigned short> vcompare;
	vcompare.vge_vf((half*)&VRS2.vh[VSTART], vr1, &VRD.vh[VSTART], VM, VR0.vh, VL);
}
