struct ShapeStride sst;
class CustomInsns CusIns;
sst_fill(sst, 1, 1);
unsigned long rs1 = MMU.npc_addr_to_mem(RS1);
unsigned long rd  = MMU.npc_addr_to_mem(RD);

switch (VME_DTYPE) {
  case 0x0:         // f16
    if (DIM == 0) {
        check_traps_vexxx_m_reduce_all(uint16_t);
        half res;
        CusIns.vemin_m((half*)rs1, &res, &sst);
        float32_t f32res;
        f32res.v = Float32(res).x;
        WRITE_FRD_STC(f32res);
    } else {
        check_traps_vexxx_m_reduce_vector(uint16_t);
        CusIns.vemin_m((half*)rs1, (half*)rd, &sst, DIM_DM);
        if(DIM_DM){
            WRITE_MEM_STC(RD, (half*)rd, CMT_LOG_VME_COM_W);
        } else {
            WRITE_MEM_STC(RD, (half*)rd, CMT_LOG_VME_COM_H);
        } 
    }
    break;
  case 0x10101:     // bf16
    if (DIM == 0) {
        check_traps_vexxx_m_reduce_all(uint16_t);
        Bfloat16 res;
        CusIns.vemin_m((Bfloat16*)rs1, &res, &sst);
        float32_t f32res;
        f32res.v = Float32(res).x;
        WRITE_FRD_STC(f32res);
    } else {
        check_traps_vexxx_m_reduce_vector(uint16_t);
        CusIns.vemin_m((Bfloat16*)rs1, (Bfloat16*)rd, &sst, DIM_DM);
        if(DIM_DM){
            WRITE_MEM_STC(RD, (Bfloat16*)rd, CMT_LOG_VME_COM_W);
        } else {
            WRITE_MEM_STC(RD, (Bfloat16*)rd, CMT_LOG_VME_COM_H);
        } 
    }
    break;
  case 0x020202:    // f32
    if (DIM == 0) {
        check_traps_vexxx_m_reduce_all(uint32_t);
        float32_t res;
        CusIns.vemin_m((float32_t*)rs1, &res, &sst);
        WRITE_FRD_STC(res);
    } else {
        check_traps_vexxx_m_reduce_vector(uint32_t);
        CusIns.vemin_m((float32_t*)rs1, (float32_t*)rd, &sst, DIM_DM);
        if(DIM_DM){
            WRITE_MEM_STC(RD, (float32_t*)rd, CMT_LOG_VME_COM_W);
        } else {
            WRITE_MEM_STC(RD, (float32_t*)rd, CMT_LOG_VME_COM_H);
        } 
    }
    break; 
  default:
    trap_ncp_cust_invalid_param(); 
}

// VME_DTYPE_DECODING_TO_TYPE({
//     if (DIM == 0) {
//         check_traps_vexxx_m_reduce_all(dtype_lut);
//         dtype_vd res;
//         vemin_m<dtype_vd>((dtype_vd*)rs1, &res, &sst, relu);
//         float32_t f32res;
//         f32res.v = Float32(res).x;
//         WRITE_FRD_STC(f32res);  //WRITE_FRD(f32res);
//     } else{
//         check_traps_vexxx_m_reduce_vector(dtype_lut);
//         unsigned long rd = MMU.npc_addr_to_mem(RD);
//         vemin_m<dtype_vd>((dtype_vd*)rs1, (dtype_vd*)rd, &sst, DIM_DM, relu);
//         if(DIM_DM){
//             WRITE_MEM_STC(RD, (dtype_vd*)rd, CMT_LOG_VME_COM_W);
//         } else {
//             WRITE_MEM_STC(RD, (dtype_vd*)rd, CMT_LOG_VME_COM_H);
//         } 
//     }
// })