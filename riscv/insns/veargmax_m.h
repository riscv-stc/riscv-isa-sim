struct ShapeStride sst;
class CustomInsns CusIns;
sst_fill(sst, 1, 1);
unsigned long rs1 = MMU.get_phy_addr(RS1);
unsigned long rd  = MMU.get_phy_addr(RD);

switch (VME_DTYPE) {
  case 0x0:         // f16
    if (DIM == 0) {
        check_traps_vexxx_m_reduce_all(uint16_t);
        uint32_t res;
        CusIns.veargmax_m((half*)rs1, &res, &sst);
        WRITE_RD_STC(res);
    } else {
        check_traps_vexxx_m_reduce_vector(uint16_t);
        CusIns.veargmax_m((half*)rs1, (uint16_t*)rd, &sst, DIM_DM);
        if(DIM_DM){
            WRITE_MEM_STC(RD, (uint16_t*)rd, CMT_LOG_VME_COM_W);
        } else {
            WRITE_MEM_STC(RD, (uint16_t*)rd, CMT_LOG_VME_COM_H);
        } 
    }
    break;
  case 0x10101:     // bf16
    if (DIM == 0) {
        check_traps_vexxx_m_reduce_all(uint16_t);
        uint32_t res;
        CusIns.veargmax_m((Bfloat16*)rs1, &res, &sst);
        WRITE_RD_STC(res);
    } else {
        check_traps_vexxx_m_reduce_vector(uint16_t);
        CusIns.veargmax_m((Bfloat16*)rs1, (uint16_t*)rd, &sst, DIM_DM);
        if(DIM_DM){
            WRITE_MEM_STC(RD, (uint16_t*)rd, CMT_LOG_VME_COM_W);
        } else {
            WRITE_MEM_STC(RD, (uint16_t*)rd, CMT_LOG_VME_COM_H);
        } 
    }
    break;
  case 0x020202:    // f32
    if (DIM == 0) {
        check_traps_vexxx_m_reduce_all(uint32_t);
        uint32_t res;
        CusIns.veargmax_m((float32_t*)rs1, &res, &sst);
        WRITE_RD_STC(res);
    } else {
        check_traps_vexxx_m_reduce_vector(uint32_t);
        CusIns.veargmax_m((float32_t*)rs1, (uint16_t*)rd, &sst, DIM_DM);
        if(DIM_DM){
            WRITE_MEM_STC(RD, (uint16_t*)rd, CMT_LOG_VME_COM_W);
        } else {
            WRITE_MEM_STC(RD, (uint16_t*)rd, CMT_LOG_VME_COM_H);
        } 
    }
    break; 
  default:
    trap_ncp_cust_invalid_param();      
}

// VME_DTYPE_DECODING_TO_TYPE({
//     if (DIM == 0) {
//         check_traps_vexxx_m_reduce_all(dtype_lut);
//         uint32_t res;
//         veargmax_m<dtype_vd>((dtype_vd*)rs1, &res, &sst);
//         WRITE_RD_STC(res);      //WRITE_RD(res);
//     } else {
//         check_traps_vexxx_m_reduce_vector(dtype_lut);
//         unsigned long rd = MMU.get_phy_addr(RD);
//         veargmax_m<dtype_vd>((dtype_vd*)rs1, (uint16_t*)rd, &sst, DIM_DM);
//         if(DIM_DM){
//             WRITE_MEM_STC(RD, (uint16_t*)rd, CMT_LOG_VME_COM_W);
//         } else {
//             WRITE_MEM_STC(RD, (uint16_t*)rd, CMT_LOG_VME_COM_H);
//         } 
//     }
// })