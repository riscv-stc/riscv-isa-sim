require_extension('V');
check_ncp_vill_invalid()
check_traps_velut_m;

struct ShapeStride sst;
sst_fill(sst, 2, 2);
unsigned long rs1 = MMU.get_phy_addr(RS1);
unsigned long rs2 = MMU.get_phy_addr(RS2);
unsigned long rd = MMU.get_phy_addr(RD);

VME_DTYPE_DECODING_TO_TYPE({
    velut_m<dtype_lut, dtype_vd>((dtype_lut*)rs1, rs2, (dtype_vd*)rd, &sst, relu);
})