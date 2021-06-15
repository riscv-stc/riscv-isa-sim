check_traps_vexxx_mf_x8out;

struct ShapeStride sst;

sst_fill(sst, 1, 1);
sst.stride_rd = STRIDE_RD;

unsigned long rs1 = MMU.get_phy_addr(RS1);
unsigned long rd = MMU.get_phy_addr(RD);
half vr2;
vr2.x = f32_to_f16(f32(FRS2)).v;
veemul_xu8_hf_mf<unsigned char, half>((half *)rs1, (unsigned char *)rd, vr2, &sst, STATE.frm);
