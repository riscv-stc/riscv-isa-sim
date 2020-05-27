require_extension('V');
check_ncp_vill_invalid()
// FIXME: should implement ncp access trap
check_vstart{
	Vext<unsigned short> vext;
	unsigned short rd;
	vext.vext_x_v(VRS2.vh, &rd, (unsigned short)RS1, VL);
	WRITE_RD(rd);
}
