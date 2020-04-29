require_extension('V');
check_traps_vexxx_m_convert(int16, int8);

class CustomInsns CusIns;
struct ShapeStride sst;
sst_fill(sst, 1, 2);
unsigned long rs1 = MMU.get_phy_addr(RS1);
unsigned long rd = MMU.get_phy_addr(RD);

sst.stride_rs1 = STRIDE_RS1;
CusIns.vecvt_hf_x8_m((int8_t*)rs1, (half*)rd, &sst);
