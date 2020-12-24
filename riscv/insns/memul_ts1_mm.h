require_extension('V');
//check_ncp_vill_invalid()
if (p->get_csr(CSR_MTE_DATA_TYPE) == 0)
      check_traps_memul_ts_mm(int16, int16);

reg_t t_rs1 = RS1;
reg_t t_rs2 = RS2;
reg_t t_rd = RD;
//p->run_async([p, t_rs1, t_rs2, t_rd, pc]() {
  class CustomInsns CusIns;
  struct ShapeStride sst;
  bc_sst_fill(sst, 2, 2);

  unsigned long rs1 = MMU.get_phy_addr(t_rs1);
  unsigned long rs2 = MMU.get_phy_addr(t_rs2);
  unsigned long rd = MMU.get_phy_addr(t_rd);

  if (p->get_csr(CSR_MTE_DATA_TYPE) == 0)
    CusIns.memul_ts_mm((half*)rs1, (half*)rs2, (half*)rd, &sst, TS);
//});

//wfi();
