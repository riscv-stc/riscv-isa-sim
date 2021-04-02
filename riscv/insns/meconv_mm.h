require_extension('V');
//check_ncp_vill_invalid()

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

  switch (p->get_csr(CSR_MME_DATA_TYPE)){
  case 0x0: // f16*f16 = f16
    check_traps_meconv_mm(int16_t, int16_t, int16_t);
    CusIns.meconv_mm((half*)rs1, (half*)rd, (half*)rs2, &sst);
    break;
  case 0x03090b: // (float16->int8) * int8 = f16
    check_traps_meconv_mm(int16_t, int8_t, int16_t);
    CusIns.meconv_mm((half*)rs1, (half*)rd, (int8_t*)rs2, &sst);
    break;
  case 0x03030b: // int8 * int8 = f16(int32->f16, mf)
    check_traps_meconv_mm(int8_t, int8_t, int8_t);
    CusIns.meconv_mm((int8_t*)rs1, (half*)rd, (int8_t*)rs2, &sst);
    break;
  case 0x020202: // f32 * f32 = f32
    check_traps_meconv_mm(int32_t, int32_t, int32_t);
    CusIns.meconv_mm((float32_t*)rs1, (float32_t*)rd, (float32_t*)rs2, &sst);
    break;
  }
//});

//wfi();
