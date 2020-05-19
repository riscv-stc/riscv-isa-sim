require_extension('V');
check_ncp_vill_invalid()
// FIXME: should implement ncp access trap
check_vstart{
	half vr1;
	vr1.x = f32_to_f16(f32(FRS1)).v;
	Vdiv<half, unsigned short> vdiv;
	vdiv.vdiv_vf((half*)&VRS2.vh[VSTART], vr1, (half*)&VRD.vh[VSTART],
				   VM, VR0.vh, VL-VSTART);
}
