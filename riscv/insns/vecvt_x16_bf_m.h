check_traps_vexxx_m_convert(int16_t, int16_t);

struct ShapeStride sst;
sst_fill(sst, 1, 1);
unsigned long rs1 = MMU.get_phy_addr(RS1);
unsigned long rd = MMU.get_phy_addr(RD);

require(STATE.frm < 0x5);
vecvt_x16_bf_m((Bfloat16*)rs1, (int16_t*)rd, &sst, STATE.frm);
WRITE_MEM_STC(RD, (int16_t*)rd, CMT_LOG_VME_COM);
