require_extension('V');
if(VSTART >= VL){
	VSTART = 0;
}
else{
	half vr1;
	vr1.x = (unsigned short)FRS1.v[0];
	Vadd<half, unsigned short> vadd;
	vadd.vadd_vf((half*)&VRS2.vh[VSTART], vr1, (half*)&VRD.vh[VSTART], VM, VR0.vh, VL);
}
