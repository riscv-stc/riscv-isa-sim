require_extension('V');

struct VmeShapeStride vss;
vme_ss_fill(vss, 2);

unsigned long rs1 = MMU.get_phy_addr(RS1);
unsigned long rd = MMU.get_phy_addr(RD);

vemaxpool_m((half*)rs1, (half*)rd, &vss);
