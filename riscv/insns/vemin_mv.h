require_extension('V');
// FIXME: should implement ncp access trap

class CustomInsns CusIns;
struct ShapeStride sst;
sst_fill(sst);
unsigned long rs1 = MMU.get_phy_addr(RS1);
unsigned long rs2 = MMU.get_phy_addr(RS2);
unsigned long rd = MMU.get_phy_addr(RD);

CusIns.vemin_mv((half*)rs1, (half*)rd, (half*)rs2, &sst, DIM_DM);
