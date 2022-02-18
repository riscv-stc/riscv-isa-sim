struct ShapeStride sst;
sst_fill(sst, 1, 1);
unsigned long rs1 = MMU.npc_addr_to_mem(RS1);
unsigned long rs2 = MMU.npc_addr_to_mem(RS2);
unsigned long rd = MMU.npc_addr_to_mem(RD);

VME_DTYPE_DECODING_TO_TYPE({
    check_traps_vexxx_mv(dtype_lut);
    versub_mv<dtype_vd>((dtype_vd*)rs1, (dtype_vd*)rd, (dtype_vd*)rs2, &sst, DIM_DM);
    WRITE_MEM_STC(RD, (dtype_vd*)rd, CMT_LOG_VME_COM);
})
