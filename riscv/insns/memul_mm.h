require_extension('V');
//check_ncp_vill_invalid()
if (p->get_csr(CSR_MME_DATA_TYPE) == 0)
      check_traps_memul_mm(int16, int16);

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
    memul_sst_fill(sst, 2, 2, 2);
    CusIns.memul_mm((half*)rs1, (half*)rs2, (half*)rd, &sst);
    break;
  case 0x90300: // (float16->int8) * int8 = f16
    memul_sst_fill(sst, 2, 1, 2);
    CusIns.memul_mm((half*)rs1, (int8_t*)rs2, (half*)rd, &sst);
    break;
  }
//});

//wfi();
