require_extension('V');

class CustomInsns CusIns;
struct ShapeStride sst;
sst_fill(sst, 1, 1);
unsigned long rs1 = MMU.npc_addr_to_mem(RS1);
unsigned long rd = MMU.npc_addr_to_mem(RD);

VME_DTYPE_DECODING_TO_TYPE({
    check_traps_mov_v(sizeof(dtype_lut));
    mov_v<dtype_vd>((dtype_vd*)rs1, (dtype_vd*)rd, &sst, DIM_DM);
    WRITE_MEM_STC(RD, (dtype_vd*)rd, CMT_LOG_VME_COM); 
})
