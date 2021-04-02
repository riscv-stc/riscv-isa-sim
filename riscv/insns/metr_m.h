require_extension('V');
check_ncp_vill_invalid()

//p->run_async([p, insn, pc]() {
  class CustomInsns CusIns;
  struct ShapeStride sst;

  bc_sst_fill(sst, 2, 2);
  unsigned long rs1 = MMU.get_phy_addr(RS1);
  unsigned long rd = MMU.get_phy_addr(RD);

  switch (p->get_csr(CSR_MME_DATA_TYPE)){
  case 0x0: // f16*f16 = f16
    check_traps_metr_m(int16_t, int16_t);
    bc_sst_fill(sst, 2, 2);
    CusIns.metr_m((half*)rs1, (half*)rd, &sst);
    break;
  case 0x030303: // int8 =  int8
    check_traps_metr_m(int8_t, int8_t);
    bc_sst_fill(sst, 1, 1);
    CusIns.metr_m((int8_t*)rs1, (int8_t*)rd, &sst);
    break; 
  case 0x020202: // f32 * f32 = f32
    check_traps_metr_m(int32_t, int32_t);
    bc_sst_fill(sst, 4, 4);
    CusIns.metr_m((float32_t*)rs1, (float32_t*)rd, &sst);
    break;
  }
//});

//wfi();
