struct ShapeStride sst;
class CustomInsns CusIns;
sst_fill(sst, 1, 1);
unsigned long rs1 = MMU.get_phy_addr(RS1);
unsigned long rs2 = MMU.get_phy_addr(RS2);
unsigned long rd  = MMU.get_phy_addr(RD);

switch (VME_DTYPE) {
  case 0x0:         // f16
    check_traps_vexxx_mv(uint16_t);
    CusIns.vemax_mv((half*)rs1, (half*)rs2, (half*)rd, &sst, DIM_DM);
    WRITE_MEM_STC(RD, (half*)rd, CMT_LOG_VME_COM);
    break;
  case 0x10101:     // bf16
    check_traps_vexxx_mv(uint16_t);
    CusIns.vemax_mv((Bfloat16*)rs1, (Bfloat16*)rs2, (Bfloat16*)rd, &sst, DIM_DM);
    WRITE_MEM_STC(RD, (Bfloat16*)rd, CMT_LOG_VME_COM);
    break;
  case 0x020202:    // f32
    check_traps_vexxx_mv(uint32_t);
    CusIns.vemax_mv((float32_t*)rs1, (float32_t*)rs2, (float32_t*)rd, &sst, DIM_DM);
    WRITE_MEM_STC(RD, (float32_t*)rd, CMT_LOG_VME_COM); 
    break; 
  default:
    trap_ncp_cust_invalid_param();  
}

// VME_DTYPE_DECODING_TO_TYPE({
//     check_traps_vexxx_mv(dtype_lut);
//     vemax_mv<dtype_vd>((dtype_vd*)rs1, (dtype_vd*)rd, (dtype_vd*)rs2, &sst, DIM_DM, relu);
//     WRITE_MEM_STC(RD, (dtype_vd*)rd, CMT_LOG_VME_COM);
// })