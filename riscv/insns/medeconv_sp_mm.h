require_extension('V');
//check_ncp_vill_invalid()
if (p->get_csr(CSR_MME_DATA_TYPE) == 0)
  check_traps_medeconv_sp_mm(int16_t, int16_t);

reg_t t_rs1 = RS1;
reg_t t_rs2 = RS2;
reg_t t_idx = MME_SPARSE_BASE;
reg_t t_rd = RD;

//p->run_async([p, &insn, pc]() {
  class CustomInsns CusIns;
  struct ConvShapeStride sst;
  conv_fill(sst);

  unsigned long rs1 = MMU.get_phy_addr(t_rs1);
  unsigned long rs2 = MMU.get_phy_addr(t_rs2);
  unsigned long idx = MMU.get_phy_addr(t_idx);
  unsigned long rd = MMU.get_phy_addr(t_rd);

  switch (p->get_csr(CSR_MME_DATA_TYPE)){
  case 0x0: // f16*f16 = f16
    CusIns.medeconv_sp_mm((half*)rs1, (half*)rs2, (uint8_t *)idx, (half*)rd, &sst);
    break;
  }
//});

//wfi();