check_traps_vexxx_m_convert(int32_t, int16_t);

struct ShapeStride sst;
sst_fill(sst, 1, 1);
unsigned long rs1 = MMU.get_phy_addr(RS1);
unsigned long rd = MMU.get_phy_addr(RD);

vecvt_f32_bf_m((Bfloat16*)rs1, (Float32*)rd, &sst);
WRITE_MEM_STC(RD, (Float32*)rd, CMT_LOG_VME_DATA32);