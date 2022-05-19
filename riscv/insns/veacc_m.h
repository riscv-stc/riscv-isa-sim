struct ShapeStride sst;
sst_fill(sst, 1, 1);
unsigned long rs1 = MMU.npc_addr_to_mem(RS1);

VME_DTYPE_DECODING_TO_TYPE({
    if(DIM == 0) {
        check_traps_vexxx_m_reduce_all(dtype_lut);
        dtype_in result;
        veacc_m<dtype_vd, dtype_in>((dtype_vd*)rs1, &result, &sst, relu);
        float32_t f32reg;
        f32reg.v = result.x;      
        WRITE_FRD_STC(f32reg);  //WRITE_FRD(f32reg);    
    } else {
        check_traps_vexxx_m_reduce_vector(dtype_lut);
        unsigned long rd = MMU.npc_addr_to_mem(RD);
        veacc_m<dtype_vd, dtype_in>((dtype_vd*)rs1, (dtype_vd*)rd, &sst, DIM_DM, relu);
        if(DIM_DM){
            WRITE_MEM_STC(RD, (dtype_vd*)rd, CMT_LOG_VME_COM_W);
        } else {
            WRITE_MEM_STC(RD, (dtype_vd*)rd, CMT_LOG_VME_COM_H);
        } 
    }
})
