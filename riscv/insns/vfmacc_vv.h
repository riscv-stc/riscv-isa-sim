require_extension('V');
check_ncp_vill_invalid()
// FIXME: should implement ncp access trap
check_vstart{
	Vma<half, unsigned short> vma;
	vma.vmacc_vv((half*)&VRS2.vh[VSTART], (half*)&VRS1.vh[VSTART], 
				 (half*)&VRD.vh[VSTART], VM, VR0.vh, VL);
}
