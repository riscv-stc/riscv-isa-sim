require_extension('V');
check_ncp_vill_invalid()
check_traps_vexxx_mf_x8out;

struct ShapeStride sst;

sst_fill(sst, 2, 1);
sst.stride_rd = STRIDE_RD;

unsigned long rs1 = MMU.get_phy_addr(RS1);
unsigned long rd = MMU.get_phy_addr(RD);

Bfloat16 vr2(f32(FRS2));
veemul_xx_xx_mf<unsigned char, Bfloat16>((Bfloat16 *)rs1, (unsigned char *)rd, vr2, &sst);
