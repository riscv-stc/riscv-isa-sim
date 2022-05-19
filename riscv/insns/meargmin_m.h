require_extension('V');

reg_t t_rs1 = RS1;
reg_t t_rd = RD;
//p->run_async([p, t_rs1, t_rs2, t_rd, pc]() {
  struct ShapeStride sst;
  class CustomInsns CusIns;
  unsigned long rs1 = MMU.npc_addr_to_mem(t_rs1);
  unsigned long rd = MMU.npc_addr_to_mem(t_rd);

  switch (p->get_csr(CSR_MME_DATA_TYPE)){
  case 0x0: // f16 = uint16
    check_traps_meargxxx_m(int16_t, int16_t);
    bc_sst_fill(sst, 2, 2);
    CusIns.veargmin_m((half*)rs1, (uint16_t*)rd, &sst, 1);
    WRITE_MEM_STC(RD, (uint16_t*)rd, CMT_LOG_MME_DATA16);
    break;
  case 0x010101: // bf16 = uint16
    check_traps_meargxxx_m(int16_t, int16_t);
    bc_sst_fill(sst, 2, 2);
    CusIns.veargmin_m((Bfloat16*)rs1, (uint16_t*)rd, &sst, 1);
    WRITE_MEM_STC(RD, (uint16_t*)rd, CMT_LOG_MME_DATA16);
    break;
  case 0x020202: // f32 = uint16
    check_traps_meargxxx_m(int32_t, int16_t);
    bc_sst_fill(sst, 4, 2);
    CusIns.veargmin_m((float32_t*)rs1, (uint16_t*)rd, &sst, 1);
    WRITE_MEM_STC(RD, (uint16_t*)rd, CMT_LOG_MME_DATA16);
    break;
  default:
    check_cust_invalid_vme_or_reduce_data_type(p->get_csr(CSR_MME_DATA_TYPE));
    break; 
  };
//});

//wfi();
