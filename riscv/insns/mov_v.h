require_extension('V');
check_ncp_vill_invalid()
check_traps_mov_v;

class CustomInsns CusIns;
struct ShapeStride sst;
sst_fill(sst, 2, 2);
unsigned long rs1 = MMU.get_phy_addr(RS1);
unsigned long rd = MMU.get_phy_addr(RD);

CusIns.mov_v((half*)rs1, (half*)rd, &sst, DIM_DM);
