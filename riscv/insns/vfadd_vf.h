require_extension('V');
vr1.x = (unsigned short)FRS1.v;
if(VSTART >= VL){
	VSTART = 0;
}
else{
	half vr1;
	float32_t fr1 = f32(FRS1);
	Vadd<half, unsigned short> vadd;
	vadd.vadd_vf((half)&VRS2.vh[VSTART], vr1, VRD.vh, VM, VR0.vh, VL);
}
