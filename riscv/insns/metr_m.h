require_extension('V');
check_ncp_vill_invalid()
check_traps_metr_m;

p->run_async([p, insn, pc]() {
  class CustomInsns CusIns;
  struct ShapeStride sst;

  bc_sst_fill(sst, 2, 2);
  unsigned long rs1 = MMU.get_phy_addr(RS1);
  unsigned long rd = MMU.get_phy_addr(RD);

  CusIns.metr_m((half*)rs1, (half*)rd, &sst);
});

wfi();