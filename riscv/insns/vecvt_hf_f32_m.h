require_extension('V');
check_ncp_vill_invalid()
check_traps_vexxx_m_convert(int16, int32);

struct ShapeStride sst;
sst_fill(sst, 4, 2);
unsigned long rs1 = MMU.get_phy_addr(RS1);
unsigned long rd = MMU.get_phy_addr(RD);

vecvt_hf_f32_m((Float32*)rs1, (half*)rd, &sst);
