require_extension('V');

struct VmeShapeStride vss;
vme_ss_fill(vss, 2);

unsigned long rs1 = MMU.get_phy_addr(RS1);
unsigned long rs2 = MMU.get_phy_addr(RS2);
unsigned long rd = MMU.get_phy_addr(RD);

vedwconv_mm<half, Float32>((half*)rs1, (half*)rs2, (half*)rd, &vss);
