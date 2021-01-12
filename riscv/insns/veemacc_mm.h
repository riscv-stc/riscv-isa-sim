require_extension('V');
check_ncp_vill_invalid()

struct ShapeStride sst;
sst_fill(sst, 2, 2);
unsigned long rs1 = MMU.get_phy_addr(RS1);
unsigned long rs2 = MMU.get_phy_addr(RS2);

VME_DTYPE_DECODING_TO_TYPE_ACC({
    if(DIM == 0){
        check_traps_vexxx_mm_reduce_all;
        dtype_vd res;
        veemacc_mm<dtype_vd, dtype_in>((dtype_vd*)rs1, &res, (dtype_vd*)rs2, &sst, relu);
        float32_t f32t;
        f32t.v = Float32(res).x;
        WRITE_FRD(f32t);
    } else {
        check_traps_vexxx_mm_reduce_vector;
        unsigned long rd  = MMU.get_phy_addr(RD);
        veemacc_mm<dtype_vd, dtype_in>((dtype_vd*)rs1, (dtype_vd*)rd, (dtype_vd*)rs2, &sst, DIM_DM, relu);
    }
})