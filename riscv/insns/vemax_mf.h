struct ShapeStride sst;
class CustomInsns CusIns;
sst_fill(sst, 1, 1);
unsigned long rs1 = MMU.npc_addr_to_mem(RS1);
unsigned long rd  = MMU.npc_addr_to_mem(RD);

float32_t vf32 = f32(FRS2);

switch (VME_DTYPE) {
  case 0x0:         // f16
    check_traps_vexxx_mf(uint16_t);
    CusIns.vemax_mf((half*)rs1, vf32, (half*)rd, &sst);
    WRITE_MEM_STC(RD, (half*)rd, CMT_LOG_VME_COM);
    break;
  case 0x01:     // bf16
    check_traps_vexxx_mf(uint16_t);
    CusIns.vemax_mf((Bfloat16*)rs1, vf32, (Bfloat16*)rd, &sst);
    WRITE_MEM_STC(RD, (Bfloat16*)rd, CMT_LOG_VME_COM);
    break;
  case 0x02:    // f32
    check_traps_vexxx_mf(uint32_t);
    CusIns.vemax_mf((float32_t*)rs1, vf32, (float32_t*)rd, &sst);
    WRITE_MEM_STC(RD, (float32_t*)rd, CMT_LOG_VME_COM); 
    break;  
  default:
    break;
}

// VME_DTYPE_DECODING_TO_TYPE({
//     check_traps_vexxx_mf(dtype_lut);
//     dtype_vd vr2(f32(FRS2));
//     vemax_mf<dtype_vd>((dtype_vd*)rs1, (dtype_vd*)rd, vr2, &sst, relu);
//     WRITE_MEM_STC(RD, (dtype_vd*)rd, CMT_LOG_VME_COM);
// })