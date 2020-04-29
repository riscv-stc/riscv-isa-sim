require_extension('V');
check_traps_vexxx_m_convert(int8, int16);

class CustomInsns CusIns;
struct ShapeStride sst;
sst_fill(sst, 2, 1);
unsigned long rs1 = MMU.get_phy_addr(RS1);
unsigned long rd = MMU.get_phy_addr(RD);

//sst.stride_rs1 = STRIDE_RS1;
sst.stride_rd = STRIDE_RD;
CusIns.vecvt_x8_hf_m((half *)rs1, (int8_t *)rd, &sst);
