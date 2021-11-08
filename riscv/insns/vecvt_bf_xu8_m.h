check_traps_vexxx_m_convert(int16_t, int8_t);

struct ShapeStride sst;
sst_fill(sst, 1, 1);
unsigned long rs1 = MMU.get_phy_addr(RS1);
unsigned long rd = MMU.get_phy_addr(RD);

vecvt_bf_xu8_m((uint8_t*)rs1, (Bfloat16*)rd, &sst);
WRITE_MEM_STC(RD, (Bfloat16*)rd, CMT_LOG_VME_DATA16);
