require_extension('V');
if(VSTART >= VL){
	VSTART = 0;
}
else{
	Vma<unsigned short, unsigned short> vma;
	vma.vmacc_vf(&VRS2.vh[VSTART], (unsigned short)FRS1.v, VRD.vh, VM, VR0.vh, VL);
}
