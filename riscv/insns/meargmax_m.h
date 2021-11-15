require_extension('V');

reg_t t_rs1 = RS1;
reg_t t_rd = RD;
//p->run_async([p, t_rs1, t_rs2, t_rd, pc]() {
  struct ShapeStride sst;

  unsigned long rs1 = MMU.get_phy_addr(t_rs1);
  unsigned long rd = MMU.get_phy_addr(t_rd);

  switch (p->get_csr(CSR_MME_DATA_TYPE)){
  case 0x06: // f16 = uint16_t
    check_traps_meargxxx_m(int16_t, int16_t);
    bc_sst_fill(sst, 2, 2);
    veargmax_m<half>((half*)rs1, (uint16_t*)rd, &sst, 1);
    WRITE_MEM_STC(RD, (uint16_t*)rd, CMT_LOG_MME_REDUCE);
    break;
  case 0x010106: // bf16 = uint16_t
    check_traps_meargxxx_m(int16_t, int16_t);
    bc_sst_fill(sst, 2, 2);
    veargmax_m<Bfloat16>((Bfloat16*)rs1, (uint16_t*)rd, &sst, 1);
    WRITE_MEM_STC(RD, (uint16_t*)rd, CMT_LOG_MME_REDUCE);
    break;
  case 0x020206: // f32 = uint16_t
    check_traps_meargxxx_m(int32_t, int16_t);
    bc_sst_fill(sst, 4, 2);
    veargmax_m<Float32>((Float32*)rs1, (uint16_t*)rd, &sst, 1);
    WRITE_MEM_STC(RD, (uint16_t*)rd, CMT_LOG_MME_REDUCE);
    break;
  }
//});

//wfi();
