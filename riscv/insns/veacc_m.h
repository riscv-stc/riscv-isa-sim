require_extension('V');
check_ncp_vill_invalid()

struct ShapeStride sst;
sst_fill(sst, 2, 2);
unsigned long rs1 = MMU.get_phy_addr(RS1);

// a dtype bug here, to do ....
if(DIM == 0){
    check_traps_vexxx_m_reduce_all;
} else {
    check_traps_vexxx_m_reduce_vector;
}

VME_DTYPE_DECODING_TO_TYPE({
    if(DIM == 0) {
        dtype_vd result;
        veacc_m<dtype_vd, dtype_in>((dtype_vd*)rs1, &result, &sst, relu);
        Float32 f32res = Float32(result);
        float32_t f32reg;
        f32reg.v = f32res.x;
        WRITE_FRD(f32reg);
    } else {
        unsigned long rd = MMU.get_phy_addr(RD);
        veacc_m<dtype_vd, dtype_in>((dtype_vd*)rs1, (dtype_vd*)rd, &sst, DIM_DM, relu);
    }
})
