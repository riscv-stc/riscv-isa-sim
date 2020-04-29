require_extension('V');
// FIXME: should implement ncp access trap

class CustomInsns CusIns;
struct ShapeStride sst;
sst_fill(sst, 2, 2);
unsigned long rd = MMU.get_phy_addr(RD);
half vr1;
vr1.x = f32_to_f16(f32(FRS1)).v;

CusIns.mov_f(vr1, (half*)rd, &sst);
