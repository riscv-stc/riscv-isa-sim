require_extension('V');

class CustomInsns CusIns;
struct ShapeStride sst;
sst_fill(sst);
unsigned long rs1 = MMU.get_phy_addr(RS1);
unsigned long rs2 = MMU.get_phy_addr(RS2);
unsigned long rd = MMU.get_phy_addr(RD);
float32_t frs2 = f32(FRS2);

CusIns.velkrelu_mf((half*)rs1, (half)frs2.v, (half*)rd, &sst);
