check_traps_velut_m;

struct ShapeStride sst;
sst_fill(sst, 1, 1);
unsigned long rs1 = MMU.npc_addr_to_mem(RS1);
unsigned long rs2 = MMU.npc_addr_to_mem(RS2);
unsigned long rd = MMU.npc_addr_to_mem(RD);

VME_DTYPE_DECODING_TO_TYPE({
    velut_m<uint16_t, dtype_vd>((uint16_t*)rs1, rs2, (dtype_vd*)rd, &sst, relu);
    WRITE_MEM_STC(RD, (dtype_vd*)rd, CMT_LOG_VME_COM);
})