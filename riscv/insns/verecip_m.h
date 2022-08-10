struct ShapeStride sst;
sst_fill(sst, 1, 1);

unsigned long rs1 = MMU.npc_addr_to_mem(RS1);
unsigned long rd = MMU.npc_addr_to_mem(RD);

VME_DTYPE_DECODING_TO_TYPE({
    check_traps_vexxx_m_element_wise(dtype_vd);
    softfloat_roundingMode = softfloat_round_near_maxMag;    
    verecip_m<dtype_vd>((dtype_vd*)rs1, (dtype_vd*)rd, &sst);
    WRITE_MEM_STC(RD, (dtype_vd*)rd, CMT_LOG_VME_COM);
})

