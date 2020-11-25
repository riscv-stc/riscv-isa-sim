require_extension('V');
check_ncp_vill_invalid()
check_traps_vexxx_mm;

class CustomInsns CusIns;
struct ShapeStride sst;
sst_fill(sst, 2, 2);

unsigned long rs1 = MMU.get_phy_addr(RS1);
unsigned long rs2 = MMU.get_phy_addr(RS2);
unsigned long rd = MMU.get_phy_addr(RD);

CusIns.vesub_mm((half*)rs1, (half*)rd, (half*)rs2, &sst);
