require_extension('V');
NCP_AUNIT();
check_vstart{
	float16_t cvt;
	cvt.v = VRS2.vh[0];
	WRITE_FRD(f16_to_f32(cvt));
}
