require_extension('V');
check_ncp_vill_invalid()
check_traps_vexxx_mm;

struct ShapeStride sst;
sst_fill(sst, 2, 2);
unsigned long rs1 = MMU.get_phy_addr(RS1);
unsigned long rs2 = MMU.get_phy_addr(RS2);
unsigned long rd = MMU.get_phy_addr(RD);

DTYPE_DECODING_TO_TYPE({
    veadd_mm<dtype_vd>((dtype_vd*)rs1, (dtype_vd*)rd, (dtype_vd*)rs2, &sst);
})
