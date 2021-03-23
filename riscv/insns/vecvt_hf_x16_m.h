require_extension('V');
check_ncp_vill_invalid()
check_traps_vexxx_m_convert(int16_t, int16_t);

class CustomInsns CusIns;
struct ShapeStride sst;
sst_fill(sst, 1, 1);
unsigned long rs1 = MMU.get_phy_addr(RS1);
unsigned long rd = MMU.get_phy_addr(RD);

CusIns.vecvt_hf_x16_m((int16_t*)rs1, (half*)rd, &sst);