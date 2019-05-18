require_extension('V');
if(VSTART >= VL){
	VSTART = 0;
}
else{
	Vma<unsigned short, unsigned short> vma;
	vma.vmax_vv(&VRS2.vh[VSTART], &VRS1.vh[VSTART], VRD.vh, VM, VR0.vh, VL);
}
