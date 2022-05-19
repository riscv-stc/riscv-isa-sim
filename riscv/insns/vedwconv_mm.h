struct VmeShapeStride vss;
vme_ss_fill(vss, 2);

unsigned long rs1 = MMU.npc_addr_to_mem(RS1);
unsigned long rs2 = MMU.npc_addr_to_mem(RS2);
unsigned long rd = MMU.npc_addr_to_mem(RD);

VME_DTYPE_DECODING_TO_TYPE({
    check_traps_veconv_m(dtype_vd);
    vedwconv_mm<dtype_vd, dtype_in>((dtype_vd*)rs1, (dtype_vd*)rs2, (dtype_vd*)rd, &vss);
    WRITE_MEM_STC(RD, (dtype_vd*)rd, CMT_LOG_VME_CONV);
})

