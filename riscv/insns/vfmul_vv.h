require_extension('V');
check_vstart{
	Vmul<half, unsigned short> vmul;
	vmul.vmul_vv((half*)&VRS2.vh[VSTART], (half*)&VRS1.vh[VSTART], 
				 (half*)&VRD.vh[VSTART], VM, VR0.vh, VL);
}