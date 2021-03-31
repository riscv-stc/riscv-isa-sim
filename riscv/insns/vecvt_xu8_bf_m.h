require_extension('V');
check_ncp_vill_invalid()
check_traps_vexxx_m_convert(int8_t, int16_t);

struct ShapeStride sst;
sst_fill(sst, 1, 1);
unsigned long rs1 = MMU.get_phy_addr(RS1);
unsigned long rd = MMU.get_phy_addr(RD);

require(STATE.frm < 0x5);
vecvt_xu8_bf_m((Bfloat16*)rs1, (uint8_t*)rd, &sst, STATE.frm);
