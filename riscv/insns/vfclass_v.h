require_extension('V');
check_ncp_vill_invalid()
// FIXME: should implement ncp access trap
check_vstart{
	Vfclass<unsigned short, unsigned short> vfclass;
	vfclass.vfclass_v((half*)&VRS2.vh[VSTART], (unsigned short*)&VRD.vh[VSTART], VM, VR0.vh, VL);
}
