require_extension('V');
NCP_AUNIT();
check_vstart{
	Vext<unsigned short> vext;
	unsigned short rd;
	vext.vext_x_v(VRS2.vh, &rd, (unsigned short)RS1, VLEN/SEW);
	WRITE_RD(rd);
}
