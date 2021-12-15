struct ShapeStride sst;
sst_fill(sst, 1, 1);

unsigned long rd = MMU.get_phy_addr(RD);

VME_DTYPE_DECODING_TO_TYPE({
    check_traps_verand_m(dtype_vd);

    dtype_vd **rand_value = p->rand<dtype_vd>(sst.shape1_row, sst.shape1_column );
    verand_m<dtype_vd>((dtype_vd*)rd, &sst, rand_value, false);
    WRITE_MEM_STC(RD, (dtype_vd*)rd, CMT_LOG_VME_COM);
})
