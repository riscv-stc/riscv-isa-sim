require_extension('V');
check_traps_meconv_mm(int8, int32);

class CustomInsns CusIns;
struct ConvShapeStride sst;
conv_fill(sst);

unsigned long rs1 = MMU.get_phy_addr(RS1);
unsigned long rs2 = MMU.get_phy_addr(RS2);
unsigned long rd = MMU.get_phy_addr(RD);

CusIns.meconv_x8_mm((int8_t*)rs1, (int32_t*)rd, (int8_t*)rs2, &sst);

