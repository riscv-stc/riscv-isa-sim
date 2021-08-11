struct ShapeStride sst;
sst_fill(sst, 1, 1);
unsigned long rs1 = MMU.get_phy_addr(RS1);

VME_DTYPE_DECODING_TO_TYPE({
    if (DIM == 0) {
        check_traps_vexxx_m_reduce_all(dtype_lut);
        uint32_t res;
        veargmin_m<dtype_vd>((dtype_vd*)rs1, &res, &sst);
        WRITE_RD(res);
    } else {
        check_traps_vexxx_m_reduce_vector(dtype_lut);
        unsigned long rd = MMU.get_phy_addr(RD);
        veargmin_m<dtype_vd>((dtype_vd*)rs1, (uint16_t*)rd, &sst, DIM_DM);
    }
})