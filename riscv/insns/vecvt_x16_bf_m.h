require_extension('V');
check_ncp_vill_invalid()
check_traps_vexxx_m_convert(int16_t, int16_t);

struct ShapeStride sst;
sst_fill(sst, 1, 1);
unsigned long rs1 = MMU.get_phy_addr(RS1);
unsigned long rd = MMU.get_phy_addr(RD);

vecvt_x16_bf_m((Bfloat16*)rs1, (int16_t*)rd, &sst);
