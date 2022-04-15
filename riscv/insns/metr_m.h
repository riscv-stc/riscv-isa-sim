require_extension('V');

//p->run_async([p, insn, pc]() {
  class CustomInsns CusIns;
  struct ShapeStride sst;

  bc_sst_fill(sst, 2, 2);
  unsigned long rs1 = MMU.get_phy_addr(RS1);
  unsigned long rd = MMU.get_phy_addr(RD);

  switch (p->get_csr(CSR_MME_DATA_TYPE)){
  case 0x0: // f16*f16 = f16
    check_traps_metr_m(int16_t, int16_t);
    bc_sst_fill(sst, 2, 2);
    CusIns.metr_m((half*)rs1, (half*)rd, &sst);
    WRITE_MEM_STC(RD, (half*)rd, CMT_LOG_MME_METR);   
    break;
  case 0x10101: // bf16 -> f16
    check_traps_metr_m(int16_t, int16_t);
    bc_sst_fill(sst, 2, 2);
    CusIns.metr_m((Bfloat16*)rs1, (Bfloat16*)rd, &sst);
    WRITE_MEM_STC(RD, (Bfloat16*)rd, CMT_LOG_MME_METR);   
    break;
  case 0x030303: // int8 =  int8  //note: ver0.9 not support 0x30303 
    check_traps_metr_m(int8_t, int8_t);
    bc_sst_fill(sst, 1, 1);
    CusIns.metr_m((int8_t*)rs1, (int8_t*)rd, &sst);  
    break; 
  case 0x020202: // f32 * f32 = f32
    check_traps_metr_m(int32_t, int32_t);
    bc_sst_fill(sst, 4, 4);
    CusIns.metr_m((float32_t*)rs1, (float32_t*)rd, &sst);
    WRITE_MEM_STC(RD, (float32_t*)rd, CMT_LOG_MME_METR);   
    break;

  default:
    check_cust_invalid_mme_tran_data_type(p->get_csr(CSR_MME_DATA_TYPE));
    break;    
  }
//});

//wfi();
