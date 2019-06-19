require_extension('V');
check_vstart{
	Vfclass<unsigned short, unsigned short> vfclass;
	vfclass.vfclass_v((half*)&VRS2.vh[VSTART], (unsigned short*)&VRD.vh[VSTART], VM, VR0.vh, VL);
}
