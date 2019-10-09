require_extension('V');
NCP_AUNIT();
check_vstart{
	Vdiv<half, unsigned short> vdiv;
	vdiv.vdiv_vv((half*)&VRS2.vh[VSTART], (half*)&VRS1.vh[VSTART], 
				 (half*)&VRD.vh[VSTART], VM, VR0.vh, VL-VSTART);
}
