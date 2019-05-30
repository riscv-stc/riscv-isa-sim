require_extension('V');
check_vstart{
	half vr1;
	vr1.x = (unsigned short)FRS1.v[0];
	Vsgnj<half, unsigned short> vsgnj;
	vsgnj.vsgnj_vf((half*)&VRS2.vh[VSTART], vr1, (half*)&VRD.vh[VSTART], VM, VR0.vh, VL);
}
