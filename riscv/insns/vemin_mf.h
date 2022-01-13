struct ShapeStride sst;
class CustomInsns CusIns;
sst_fill(sst, 1, 1);
unsigned long rs1 = MMU.get_phy_addr(RS1);
unsigned long rd  = MMU.get_phy_addr(RD);

float32_t vf32 = f32(FRS2);

switch (VME_DTYPE) {
  case 0x0:         // f16
    check_traps_vexxx_mf(uint16_t);
    CusIns.vemin_mf((half*)rs1, vf32, (half*)rd, &sst);
    WRITE_MEM_STC(RD, (half*)rd, CMT_LOG_VME_COM);
    break;
  case 0x10101:     // bf16
    check_traps_vexxx_mf(uint16_t);
    CusIns.vemin_mf((Bfloat16*)rs1, vf32, (Bfloat16*)rd, &sst);
    WRITE_MEM_STC(RD, (Bfloat16*)rd, CMT_LOG_VME_COM);
    break;
  case 0x020202:    // f32
    check_traps_vexxx_mf(uint32_t);
    CusIns.vemin_mf((float32_t*)rs1, vf32, (float32_t*)rd, &sst);
    WRITE_MEM_STC(RD, (float32_t*)rd, CMT_LOG_VME_COM); 
    break;  
  default:
    trap_ncp_cust_invalid_param(); 
}

// VME_DTYPE_DECODING_TO_TYPE({
//     check_traps_vexxx_mf(dtype_lut);
//     dtype_vd vr2(f32(FRS2));
//     vemin_mf<dtype_vd>((dtype_vd*)rs1, (dtype_vd*)rd, vr2, &sst, relu);
//     WRITE_MEM_STC(RD, (dtype_vd*)rd, CMT_LOG_VME_COM);
// })