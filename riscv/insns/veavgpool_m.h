require_extension('V');

struct VmeShapeStride vss;
vme_ss_fill(vss);

unsigned long rs1 = MMU.get_phy_addr(RS1);
unsigned long rd = MMU.get_phy_addr(RD);

veavgpool_m((half*)rs1, (half*)rd, &vss);
