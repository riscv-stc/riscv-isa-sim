struct ShapeStride sst;
sst_fill(sst, 1, 1);
unsigned long rs1 = MMU.get_phy_addr(RS1);
unsigned long rs2 = MMU.get_phy_addr(RS2);

VME_DTYPE_DECODING_TO_TYPE({
    if(DIM == 0){
        check_traps_vexxx_mm_reduce_all(dtype_lut);
        dtype_in res;
        veemacc_mm<dtype_vd, dtype_in>((dtype_vd*)rs1, (dtype_vd*)rs2, &res, &sst, relu);
        float32_t f32t;
        f32t.v = res.x;
        WRITE_FRD_STC(f32t);    //WRITE_FRD(f32t);
    } else {
        check_traps_vexxx_mm_reduce_vector(dtype_lut);
        unsigned long rd  = MMU.get_phy_addr(RD);
        veemacc_mm<dtype_vd, dtype_in>((dtype_vd*)rs1, (dtype_vd*)rs2, (dtype_vd*)rd, &sst, DIM_DM, relu);
        if(DIM_DM){
            WRITE_MEM_STC(RD, (dtype_vd*)rd, CMT_LOG_VME_COM_W);
        } else {
            WRITE_MEM_STC(RD, (dtype_vd*)rd, CMT_LOG_VME_COM_H);
        }       
    }
})