require_extension('V');
check_vstart{
	half vr1;
	vr1.x = f32_to_f16(f32(FRS1)).v;
	Vmerge<half, half, unsigned short> vmerge;
	vmerge.vmerge_vf((half*)&VRS2.vh[VSTART], vr1, (half*)&VRD.vh[VSTART], VM, VR0.vh, VL);
}