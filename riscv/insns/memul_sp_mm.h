require_extension('V');
//check_ncp_vill_invalid()
if (p->get_csr(CSR_MME_DATA_TYPE) == 0)
      check_traps_memul_sp_mm(int16, int16);

reg_t t_rs1 = RS1;
reg_t t_rs2 = RS2;
reg_t t_idx = MME_SPARSE_BASE;
reg_t t_rd = RD;
//p->run_async([p, t_rs1, t_rs2, t_rd, pc]() {
  class CustomInsns CusIns;
  struct ShapeStride sst;
  

  unsigned long rs1 = MMU.get_phy_addr(t_rs1);
  unsigned long rs2 = MMU.get_phy_addr(t_rs2);
  unsigned long idx = MMU.get_phy_addr(t_idx);
  unsigned long rd = MMU.get_phy_addr(t_rd);

  switch (p->get_csr(CSR_MME_DATA_TYPE)){
  case 0x0: // f16*f16 = f16
    memul_sst_fill(sst, 2, 2, 2); 
    CusIns.memul_sp_mm((half*)rs1, (half*)rs2, (uint8_t *)idx, (half*)rd, &sst);
    break;
  case 0x03090b: // (float16->int8,mf) * int8 = f16
    memul_sst_fill(sst, 2, 1, 2);
    break;
  case 0xa0300: // (float16->uint8,mf) * int8 = f16
    memul_sst_fill(sst, 2, 1, 2);
    break;
  case 0xb0300: // (float16->int8,mv) * int8 = f16
    memul_sst_fill(sst, 2, 1, 2);
    break;
  case 0xc0300: // (float16->uint8,mv) * int8 = f16
    memul_sst_fill(sst, 2, 1, 2);
    break;
  case 0xd0300: // (bfloat16->int8,mf) * int8 = bf16
    memul_sst_fill(sst, 2, 1, 2);
    break;
  case 0xe0300: // (bfloat16->uint8,mf) * int8 = bf16
    memul_sst_fill(sst, 2, 1, 2);
    break;
  case 0xf0300: // (bfloat16->int8,mv) * int8 = bf16
    memul_sst_fill(sst, 2, 1, 2);
    break;
  case 0x100300: // (bfloat16->uint8,mv) * int8 = bf16
    memul_sst_fill(sst, 2, 1, 2);
    break;
  case 0x2: // f16 * f32 = f32
    memul_sst_fill(sst, 2, 4, 4);
    break;
  case 0x10101: // bf16 * bf16 = bf16
    memul_sst_fill(sst, 2, 2, 2);
    break;
  case 0x10102: // bf16 * bf16 = f32
    memul_sst_fill(sst, 2, 2, 4);
    break;
  case 0x20202: // f32 * f32 = f32
    memul_sst_fill(sst, 4, 4, 4);
    break;
  }
//});

//wfi();
