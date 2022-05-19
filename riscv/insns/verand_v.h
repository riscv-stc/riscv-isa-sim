struct ShapeStride sst;
sst_fill(sst, 1, 1);

unsigned long rs1 = MMU.npc_addr_to_mem(RS1);
unsigned long rd = MMU.npc_addr_to_mem(RD);

VME_DTYPE_DECODING_TO_TYPE({
    check_traps_verand_v(dtype_vd);

    uint32_t **prng_state = verand_v<dtype_vd>((dtype_vd*)rs1 );
    p->update_prng_state(prng_state);

    dtype_vd **rand_value = p->rand<dtype_vd>(sst.shape1_row, sst.shape1_column );
    verand_m<dtype_vd>((dtype_vd*)rd, &sst, rand_value, true);   
    WRITE_MEM_STC(RD, (dtype_vd*)rd, CMT_LOG_VME_COM);
})
