require_extension('V');
if(VSTART >= VL){
	VSTART = 0;
}
else{
	Vext<unsigned short> vext;
	vext.vext_x_v(&VRS2.vh[VSTART], (unsigned short*)&RD, (unsigned short)RS1, VL);
}
