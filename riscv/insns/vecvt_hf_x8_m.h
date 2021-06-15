check_traps_vexxx_m_convert(int16_t, int8_t);

struct ShapeStride sst;
sst_fill(sst, 1, 1);
unsigned long rs1 = MMU.get_phy_addr(RS1);
unsigned long rd = MMU.get_phy_addr(RD);

vecvt_hf_x8_m((int8_t*)rs1, (half*)rd, &sst);
