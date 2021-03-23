require_extension('V');
check_ncp_vill_invalid()
check_traps_vexxx_m_convert(int16_t, int32_t);

struct ShapeStride sst;
sst_fill(sst, 1, 1);
unsigned long rs1 = MMU.get_phy_addr(RS1);
unsigned long rd = MMU.get_phy_addr(RD);

vecvt_bf_f32_m((Float32*)rs1, (Bfloat16*)rd, &sst);
