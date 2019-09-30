require_extension('V');
NCP_AUNIT();

class CustomInsns CusIns;
struct ShapeStride sst;
bc_sst_fill(sst);

unsigned long rs1 = MMU.get_phy_addr(RS1);
unsigned long rs2 = MMU.get_phy_addr(RS2);
unsigned long rd = MMU.get_phy_addr(RD);

CusIns.memul_x8_mm((int8_t *)rs1, (int8_t *)rs2, (int32_t *)rd, &sst);
