require_extension('V');
check_traps_memul_mm(int16, int16, TS);

p->run_async([p, insn, pc](){
  class CustomInsns CusIns;
  struct ShapeStride sst;
  bc_sst_fill(sst, 2, 2);

  unsigned long rs1 = MMU.get_phy_addr(RS1);
  unsigned long rs2 = MMU.get_phy_addr(RS2);
  unsigned long rd = MMU.get_phy_addr(RD);

  if (!TS)
      CusIns.memul_mm((half*)rs1, (half*)rs2, (half*)rd, &sst);
  else
      CusIns.memul_mm((half*)rs1, (half*)rs2, (half*)rd, &sst, TS);
});

wfi();