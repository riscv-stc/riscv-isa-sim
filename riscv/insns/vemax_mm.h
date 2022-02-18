struct ShapeStride sst;
class CustomInsns CusIns;
sst_fill(sst, 1, 1);
unsigned long rs1 = MMU.npc_addr_to_mem(RS1);
unsigned long rs2 = MMU.npc_addr_to_mem(RS2);
unsigned long rd  = MMU.npc_addr_to_mem(RD);

switch (VME_DTYPE) {
  case 0x0:         // f16
    check_traps_vexxx_mm(uint16_t);
    CusIns.vemax_mm((half*)rs1, (half*)rs2, (half*)rd, &sst);
    WRITE_MEM_STC(RD, (half*)rd, CMT_LOG_VME_COM);
    break;
  case 0x10101:     // bf16
    check_traps_vexxx_mm(uint16_t);
    CusIns.vemax_mm((Bfloat16*)rs1, (Bfloat16*)rs2, (Bfloat16*)rd, &sst);
    WRITE_MEM_STC(RD, (Bfloat16*)rd, CMT_LOG_VME_COM);
    break;
  case 0x020202:    // f32
    check_traps_vexxx_mm(uint32_t);
    CusIns.vemax_mm((float32_t*)rs1, (float32_t*)rs2, (float32_t*)rd, &sst);
    WRITE_MEM_STC(RD, (float32_t*)rd, CMT_LOG_VME_COM); 
    break;  
  default:
    trap_ncp_cust_invalid_param(); 
}

// VME_DTYPE_DECODING_TO_TYPE({
//     check_traps_vexxx_mm(dtype_lut);
//     vemax_mm<dtype_vd>((dtype_vd*)rs1, (dtype_vd*)rd, (dtype_vd*)rs2, &sst, relu);
//     WRITE_MEM_STC(RD, (dtype_vd*)rd, CMT_LOG_VME_COM);
// })