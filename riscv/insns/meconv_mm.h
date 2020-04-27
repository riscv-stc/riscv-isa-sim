require_extension('V');
check_traps_meconv_mm(int16, int16);

class CustomInsns CusIns;
struct ConvShapeStride sst;
conv_fill(sst);

unsigned long rs1 = MMU.get_phy_addr(RS1);
unsigned long rs2 = MMU.get_phy_addr(RS2);
unsigned long rd = MMU.get_phy_addr(RD);

CusIns.meconv_mm((half*)rs1, (half*)rd, (half*)rs2, &sst);

