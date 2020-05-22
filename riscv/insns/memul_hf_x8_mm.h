require_extension('V');
check_traps_memul_mm(int8, int16, 0);

p->run_async([p, insn, pc]() {
  class CustomInsns CusIns;
  struct ShapeStride sst;
  bc_sst_fill(sst, 1, 2);

  unsigned long rs1 = MMU.get_phy_addr(RS1);
  unsigned long rs2 = MMU.get_phy_addr(RS2);
  unsigned long rd = MMU.get_phy_addr(RD);

  half cof;
  cof.x = f32_to_f16(f32(M_DEQUANT_COEFF)).v;

  CusIns.memul_hf_x8_mm((int8_t *)rs1, (int8_t *)rs2, (half *)rd, &sst, cof);
});

wfi();