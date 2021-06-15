struct ShapeStride sst;
sst_fill(sst, 1, 1);
unsigned long rs1 = MMU.get_phy_addr(RS1);

VME_DTYPE_DECODING_TO_TYPE({
    if (DIM == 0) {
        check_traps_vexxx_m_reduce_all(dtype_lut);
        dtype_vd res;
        vemax_m<dtype_vd>((dtype_vd*)rs1, &res, &sst, relu);
        float32_t f32res;
        f32res.v = Float32(res).x;
        WRITE_FRD(f32res);
    } else {
        check_traps_vexxx_m_reduce_vector(dtype_lut);
        unsigned long rd = MMU.get_phy_addr(RD);
        vemax_m<dtype_vd>((dtype_vd*)rs1, (dtype_vd*)rd, &sst, DIM_DM, relu);
    }
})