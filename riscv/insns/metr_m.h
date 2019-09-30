require_extension('V');
NCP_AUNIT();

class CustomInsns CusIns;
struct ShapeStride sst;

bc_sst_fill(sst);
unsigned long rs1 = MMU.get_phy_addr(RS1);
unsigned long rd = MMU.get_phy_addr(RD);

CusIns.metr_m((half*)rs1, (half*)rd, &sst);
