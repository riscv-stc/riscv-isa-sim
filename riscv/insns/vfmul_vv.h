require_extension('V');
// FIXME: should implement ncp access trap
check_vstart{
	Vmul<half, unsigned short> vmul;
	vmul.vmul_vv((half*)&VRS2.vh[VSTART], (half*)&VRS1.vh[VSTART], 
				 (half*)&VRD.vh[VSTART], VM, VR0.vh, VL);
}
