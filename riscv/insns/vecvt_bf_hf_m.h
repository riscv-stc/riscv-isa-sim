require_extension('V');
check_ncp_vill_invalid()
check_traps_vexxx_m_convert(int16, int16);

struct ShapeStride sst;
sst_fill(sst, 2, 2);
unsigned long rs1 = MMU.get_phy_addr(RS1);
unsigned long rd = MMU.get_phy_addr(RD);

vecvt_bf_hf_m((half*)rs1, (Bfloat16*)rd, &sst);
