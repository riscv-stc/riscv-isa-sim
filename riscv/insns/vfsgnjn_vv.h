require_extension('V');
check_vstart{
	Vsgnj<half, unsigned short> vsgnj;
	vsgnj.vsgnjn_vv((half*)&VRS2.vh[VSTART], (half*)&VRS1.vh[VSTART], (half*)&VRD.vh[VSTART], VM, VR0.vh, VL);
}
