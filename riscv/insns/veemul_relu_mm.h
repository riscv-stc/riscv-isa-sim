struct ShapeStride sst;
sst_fill(sst, 1, 1);
unsigned long rs1 = MMU.get_phy_addr(RS1);
unsigned long rs2 = MMU.get_phy_addr(RS2);
unsigned long rd = MMU.get_phy_addr(RD);

VME_DTYPE_DECODING_TO_TYPE({
    check_traps_vexxx_mm(dtype_lut);
    veemul_mm<dtype_vd>((dtype_vd*)rs1, (dtype_vd*)rs2, (dtype_vd*)rd, &sst, true);
    WRITE_MEM_STC(RD, (dtype_vd*)rd, CMT_LOG_VME_COM); 
})
