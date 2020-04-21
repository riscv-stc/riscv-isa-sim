require_extension('V');
// FIXME: should implement ncp access trap
check_vstart{
	WRITE_VRD_H(f32_to_f16(f32(FRS1)).v, 0);
}