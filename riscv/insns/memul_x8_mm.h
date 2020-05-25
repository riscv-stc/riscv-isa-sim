require_extension('V');
check_ncp_vill_invalid()
check_traps_memul_mm(int8, int32, 0);

p->run_async([p, insn, pc]() {
  class CustomInsns CusIns;
  struct ShapeStride sst;
  bc_sst_fill(sst, 1, 4);

  unsigned long rs1 = MMU.get_phy_addr(RS1);
  unsigned long rs2 = MMU.get_phy_addr(RS2);
  unsigned long rd = MMU.get_phy_addr(RD);

  CusIns.memul_x8_mm((int8_t *)rs1, (int8_t *)rs2, (int32_t *)rd, &sst);
});

wfi();
