require_extension('V');
check_traps_vexxx_m_element_wise;

class CustomInsns CusIns;
struct ShapeStride sst;

sst_fill(sst, 2, 2);
unsigned long rs1 = MMU.get_phy_addr(RS1);
unsigned long rd = MMU.get_phy_addr(RD);

CusIns.vesqrt_m((half*)rs1, (half*)rd, &sst);
