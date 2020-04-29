require_extension('V');
check_traps_vexxx_mf;

class CustomInsns CusIns;
struct ShapeStride sst;
sst_fill(sst, 2, 2);
unsigned long rs1 = MMU.get_phy_addr(RS1);
//unsigned long rs2 = MMU.get_phy_addr(RS2);
unsigned long rd = MMU.get_phy_addr(RD);
half vr2;
vr2.x = f32_to_f16(f32(FRS2)).v;

CusIns.velkrelu_mf((half*)rs1, vr2, (half*)rd, &sst);
