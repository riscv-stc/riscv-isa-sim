require_extension('V');
// FIXME: should implement ncp access trap

class CustomInsns CusIns;
struct ShapeStride sst;
sst_fill(sst);
unsigned long rs1 = MMU.get_phy_addr(RS1);
unsigned long rd = MMU.get_phy_addr(RD);
half vr2;
vr2.x = f32_to_f16(f32(FRS2)).v;

CusIns.vemax_mf((half*)rs1, (half*)rd, vr2, &sst);
