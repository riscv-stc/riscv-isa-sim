require_extension('V');
check_vstart{
	Vfclass<unsigned int, unsigned short> vfclass;
	vfclass.vfclass_v((half*)&VRS2.vh[VSTART], (unsigned int*)&VRD.vh[VSTART], VM, VR0.vh, VL);
}
