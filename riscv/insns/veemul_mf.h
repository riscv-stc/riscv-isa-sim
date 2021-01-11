require_extension('V');
check_ncp_vill_invalid()
check_traps_vexxx_mf;

struct ShapeStride sst;
sst_fill(sst, 2, 2);
unsigned long rs1 = MMU.get_phy_addr(RS1);
unsigned long rd = MMU.get_phy_addr(RD);

VME_DTYPE_DECODING_TO_TYPE({
    dtype_vd vr2(f32(FRS2));
    veemul_mf<dtype_vd>((dtype_vd*)rs1, (dtype_vd*)rd, vr2, &sst);
})