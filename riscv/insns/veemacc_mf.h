require_extension('V');
check_ncp_vill_invalid()
check_traps_vexxx_mf_reduce_vector;

struct ShapeStride sst;
sst_fill(sst, 2, 2);
unsigned long rs1 = MMU.get_phy_addr(RS1);
unsigned long rd = MMU.get_phy_addr(RD);
half vr2;
vr2.x = f32_to_f16(f32(FRS2)).v;

veemacc_mf<half, Float32>((half*)rs1, (half*)rd, vr2, &sst, DIM_DM);
