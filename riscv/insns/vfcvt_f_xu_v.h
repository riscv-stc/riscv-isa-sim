require_extension('V');
// FIXME: should implement ncp access trap
check_vstart{
	Vfcvt<unsigned short> vfcvt;
	vfcvt.vfcvt_f_xu_v(&VRS2.vh[VSTART], (half*)&VRD.vh[VSTART], VM, VR0.vh, VL);
}
