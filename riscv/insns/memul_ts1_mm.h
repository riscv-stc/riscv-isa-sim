require_extension('V');

reg_t t_rs1 = RS1;
reg_t t_rs2 = RS2;
reg_t t_rd = RD;
//p->run_async([p, t_rs1, t_rs2, t_rd, pc]() {
  class CustomInsns CusIns;
  struct ShapeStride sst;

  unsigned long rs1 = MMU.get_phy_addr(t_rs1);
  unsigned long rs2 = MMU.get_phy_addr(t_rs2);
  unsigned long rd = MMU.get_phy_addr(t_rd);

  switch (p->get_csr(CSR_MME_DATA_TYPE)){
  case 0x0: // f16*f16 = f16
    check_traps_memul_ts_mm(int16_t, int16_t, int16_t); 
    memul_sst_fill(sst, 2, 2, 2); 
    CusIns.memul_ts_mm((half*)rs1, (half*)rs2, (half*)rd, &sst);
    WRITE_MEM_STC(RD, (half*)rd, CMT_LOG_MME_MEMUL_TS);
    break;
  case 0x03090b: // (float16->int8,mf) * int8 = f16
    check_traps_memul_ts_mm(int16_t, int8_t, int16_t); 
    memul_sst_fill(sst, 2, 1, 2);
    CusIns.memul_ts_mm((half*)rs1, (int8_t*)rs2, (half*)rd, &sst);
    WRITE_MEM_STC(RD, (half*)rd, CMT_LOG_MME_MEMUL_TS);
    break;
  case 0x03030b: // int8 * int8 = f16(int32->f16,mf)
    check_traps_memul_ts_mm(int8_t, int8_t, int16_t); 
    memul_sst_fill(sst, 1, 1, 2);
    CusIns.memul_ts_mm((int8_t*)rs1, (int8_t*)rs2, (half*)rd, &sst);
    WRITE_MEM_STC(RD, (half*)rd, CMT_LOG_MME_MEMUL_TS);
    break;
  case 0x020202: // f32 * f32 = f32
    check_traps_memul_ts_mm(int32_t, int32_t, int32_t); 
    memul_sst_fill(sst, 4, 4, 4);
    CusIns.memul_ts_mm((float32_t*)rs1, (float32_t*)rs2, (float32_t*)rd, &sst);
    WRITE_MEM_STC(RD, (float32_t*)rd, CMT_LOG_MME_MEMUL_TS);
    break;
  case 0x010101: // bf16 * bf16 = bf16
    memul_sst_fill(sst, 2, 2, 2); //note
    break;
  }
//});

//wfi();
