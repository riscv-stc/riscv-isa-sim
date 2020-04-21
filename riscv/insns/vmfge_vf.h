require_extension('V');
// FIXME: should implement ncp access trap
check_vstart{
	half vr1;
	vr1.x = f32_to_f16(f32(FRS1)).v;
	Vcompare<half, unsigned short, unsigned short> vcompare;
	vcompare.vge_vf((half*)&VRS2.vh[VSTART], vr1,&VRD.vh[VSTART], VM, VR0.vh, VL);
}
