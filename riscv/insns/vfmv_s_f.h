require_extension('V');
check_ncp_vill_invalid()
// FIXME: should implement ncp access trap
check_vstart{
	WRITE_VRD_H(f32_to_f16(f32(FRS1)).v, 0);
}