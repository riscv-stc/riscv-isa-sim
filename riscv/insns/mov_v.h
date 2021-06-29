require_extension('V');

class CustomInsns CusIns;
struct ShapeStride sst;
sst_fill(sst, 1, 1);
unsigned long rs1 = MMU.get_phy_addr(RS1);
unsigned long rd = MMU.get_phy_addr(RD);

VME_DTYPE_DECODING_TO_TYPE({
    check_traps_mov_v(sizeof(dtype_lut));
    mov_v<dtype_vd>((dtype_vd*)rs1, (dtype_vd*)rd, &sst, DIM_DM);
})
