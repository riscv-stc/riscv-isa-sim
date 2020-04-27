require_extension('V');
check_traps_vexxx_mf_x8out;

class CustomInsns CusIns;
struct ShapeStride sst;

sst_fill(sst);
sst.stride_rd = STRIDE_RD;

unsigned long rs1 = MMU.get_phy_addr(RS1);
unsigned long rd = MMU.get_phy_addr(RD);
half vr2;
vr2.x = f32_to_f16(f32(FRS2)).v;

CusIns.veemul_x8_hf_mf((half *)rs1, (int8_t *)rd, vr2, &sst);
