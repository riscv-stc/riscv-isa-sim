require_extension('V');


reg_t t_rs1 = RS1;
reg_t t_rd = RD;
//p->run_async([p, t_rs1, t_rs2, t_rd, pc]() {
  class CustomInsns CusIns;
  struct ShapeStride sst;


  unsigned long rs1 = MMU.npc_addr_to_mem(t_rs1);
  unsigned long rd = MMU.npc_addr_to_mem(t_rd);

  switch (p->get_csr(CSR_MME_DATA_TYPE)){
  case 0x0: // f16*f16 = f16
    check_traps_mexxx_m(int16_t, int16_t);
    bc_sst_fill(sst, 2, 2);
    CusIns.meacc_m((half*)rs1, (half*)rd, &sst);
    WRITE_MEM_STC(RD, (half*)rd, CMT_LOG_MME_REDUCE);
    break;
  case 0x010101: // bf16 * bf16 = bf16
    check_traps_mexxx_m(int16_t, int16_t);
    bc_sst_fill(sst, 2, 2);
    CusIns.meacc_m((Bfloat16*)rs1, (Bfloat16*)rd, &sst);
    WRITE_MEM_STC(RD, (Bfloat16*)rd, CMT_LOG_MME_REDUCE);
    break; 
  case 0x020202: // f32 * f32 = f32
    check_traps_mexxx_m(int16_t, int16_t);
    bc_sst_fill(sst, 4, 4);
    CusIns.meacc_m((float32_t*)rs1, (float32_t*)rd, &sst);
    WRITE_MEM_STC(RD, (float32_t*)rd, CMT_LOG_MME_REDUCE);
    break;
  default:
    check_cust_invalid_vme_or_reduce_data_type(p->get_csr(CSR_MME_DATA_TYPE));
    check_cust_invalid_npu_data_type(p->get_csr(CSR_MME_DATA_TYPE));
    break; 
  }
//});

//wfi();
