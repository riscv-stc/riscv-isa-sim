check_traps_vexxx_mv_x32;

class CustomInsns CusIns;
struct ShapeStride sst;
sst_fill(sst, 4, 2);
unsigned long rs1 = MMU.get_phy_addr(RS1);
unsigned long rs2 = MMU.get_phy_addr(RS2);
unsigned long rd = MMU.get_phy_addr(RD);

CusIns.veemul_x32_mv((int32_t *)rs1, (half*)rs2, (half*)rd, &sst);
WRITE_MEM_STC(RD, (half*)rd, CMT_LOG_VME_COM);
