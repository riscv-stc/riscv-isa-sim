require_extension('V');
check_traps_velut_m;

class CustomInsns CusIns;
struct ShapeStride sst;
sst_fill(sst, 2, 2);
unsigned long rs1 = MMU.get_phy_addr(RS1);
unsigned long rs2 = MMU.get_phy_addr(RS2);
unsigned long rd = MMU.get_phy_addr(RD);

CusIns.velut_m((uint16_t*)rs1, (uint16_t*)rs2, (half*)rd, &sst);
