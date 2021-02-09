require_extension('V');
//check_ncp_vill_invalid()
 if (p->get_csr(CSR_MTE_DATA_TYPE) == 0)
     check_traps_mexxx_m(int16, int16);

reg_t t_rs1 = RS1;
reg_t t_rd = RD;
//p->run_async([p, t_rs1, t_rs2, t_rd, pc]() {
  class CustomInsns CusIns;
  struct ShapeStride sst;
  bc_sst_fill(sst, 2, 2);

  unsigned long rs1 = MMU.get_phy_addr(t_rs1);
  unsigned long rd = MMU.get_phy_addr(t_rd);

  if (p->get_csr(CSR_MTE_DATA_TYPE) == 0)
    CusIns.memax_m((half*)rs1, (half*)rd, &sst);
//});

//wfi();
