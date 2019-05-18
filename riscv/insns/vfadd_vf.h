require_extension('V');
if(VSTART >= VL){
	VSTART = 0;
}
else{
	Vadd<unsigned short, unsigned short> vadd;
	vadd.vadd_vf(&VRS2.vh[VSTART], (unsigned short)FRS1.v, VRD.vh, VM, VR0.vh, VL);
}
