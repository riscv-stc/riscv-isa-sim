require_extension('V');

class CustomInsns CusIns;
struct ShapeStride sst;
sst_fill(sst);
unsigned long rs1 = MMU.get_phy_addr(RS1);
unsigned long rs2 = MMU.get_phy_addr(RS2);
unsigned long rd = MMU.get_phy_addr(RD);
if (!TS)
    CusIns.vemmul_mm((half*)rs1, (half*)rs2, (half*)rd, &sst);
else
    CusIns.vemmul_mm((half*)rs1, (half*)rs2, (half*)rd, &sst, TS);
