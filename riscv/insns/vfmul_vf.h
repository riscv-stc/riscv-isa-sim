require_extension('V');
NCP_AUNIT();
check_vstart{
	half vr1;
	vr1.x = f32_to_f16(f32(FRS1)).v;
	Vmul<half, unsigned short> vmul;
	vmul.vmul_vf((half*)&VRS2.vh[VSTART], vr1, (half*)&VRD.vh[VSTART], VM, VR0.vh, VL);
}
