require_extension('V');
check_vstart{
	half vr1;
	vr1.x = f32_to_f16(f32(FRS1)).v;
	Vcompare<half, unsigned short, unsigned short> vcompare;
	vcompare.vmford_vf((half*)&VRS2.vh[VSTART], vr1, (unsigned short*)&VRD.vh[VSTART], VM, VR0.vh, VL);
}
