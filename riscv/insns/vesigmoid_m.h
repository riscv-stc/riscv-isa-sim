struct ShapeStride sst;
sst_fill(sst, 1, 1);

unsigned long rs1 = MMU.get_phy_addr(RS1);
unsigned long rd = MMU.get_phy_addr(RD);

VME_DTYPE_DECODING_TO_TYPE({
    check_traps_vexxx_m_element_wise(dtype_vd);
    vesigmoid_m<dtype_vd>((dtype_vd*)rs1, (dtype_vd*)rd, &sst);
    WRITE_MEM_STC(RD, (dtype_vd*)rd, CMT_LOG_VME_COM);
})

