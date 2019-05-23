require_extension('V');

class CustomInsns CusIns;
struct ShapeStride sst;
sst_fill(sst);
unsigned long rs1 = MMU.get_phy_addr(RS1);
unsigned long rd = MMU.get_phy_addr(RD);
float32_t frs2 = f32(FRS2);
half vr2;
vr2.x = (unsigned short)frs2.v;

CusIns.veemacc_mf((half*)rs1, (half*)rd, vr2, &sst, DIM_DM);
