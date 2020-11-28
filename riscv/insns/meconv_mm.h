require_extension('V');
//check_ncp_vill_invalid()
// if (get_csr(CSR_MTE_DATA_TYPE) = 0))
  // check_traps_meconv_mm(int16, int16);

reg_t t_rs1 = RS1;
reg_t t_rs2 = RS2;
reg_t t_rd = RD;

//p->run_async([p, &insn, pc]() {
  class CustomInsns CusIns;
  struct ConvShapeStride sst;
  conv_fill(sst);

  unsigned long rs1 = MMU.get_phy_addr(t_rs1);
  unsigned long rs2 = MMU.get_phy_addr(t_rs2);
  unsigned long rd = MMU.get_phy_addr(t_rd);

  // if (get_csr(CSR_MTE_DATA_TYPE) = 0))
    CusIns.meconv_mm((half*)rs1, (half*)rd, (half*)rs2, &sst);
//});

//wfi();