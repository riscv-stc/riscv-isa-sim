require_extension('V');
check_ncp_vill_invalid()
check_traps_vexxx_mf_reduce_vector;

struct ShapeStride sst;
sst_fill(sst, 2, 2);
unsigned long rs1 = MMU.get_phy_addr(RS1);
unsigned long rd = MMU.get_phy_addr(RD);

VME_DTYPE_DECODING_TO_TYPE_ACC({
    dtype_vd vr2(f32(FRS2));
    veemacc_mf<dtype_vd, dtype_in>((dtype_vd*)rs1, (dtype_vd*)rd, vr2, &sst, DIM_DM, relu);
})