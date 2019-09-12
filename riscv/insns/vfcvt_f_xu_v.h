require_extension('V');
NCP_AUNIT();
check_vstart{
	Vfcvt<unsigned short> vfcvt;
	vfcvt.vfcvt_f_xu_v(&VRS2.vh[VSTART], (half*)&VRD.vh[VSTART], VM, VR0.vh, VL);
}
