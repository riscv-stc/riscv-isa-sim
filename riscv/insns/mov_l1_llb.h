/*
uint8_t e_size = 2;

if (MTE_DATA_TYPE == 0x0 || MTE_DATA_TYPE == 0x101)
    e_size = 2;
else if (MTE_DATA_TYPE == 0x202)
    e_size = 4;
else
    e_size = 1;
*/
MTE_DTYPE_DECODING_TO_TYPE({
  uint8_t esize = 2;

  esize = sizeof(dtype_lut);
  check_traps_mov_l1_llb(dtype_lut);

  uint8_t* src = (uint8_t*)p->get_sim()->addr_to_mem(zext_xlen(RS1));
  uint8_t* dst = (uint8_t*)MMU.get_phy_addr(RD);

  //src shape
  uint16_t col = MTE_SHAPE_COLUMN;
  uint16_t row = MTE_SHAPE_ROW;
  uint32_t copy_stride_rs1 = (MTE_STRIDE_RS1 ? MTE_STRIDE_RS1 : col) * esize;
  uint32_t copy_stride_rd = (MTE_STRIDE_RD ? MTE_STRIDE_RD : col) * esize;

  uint64_t len = row * copy_stride_rs1;
  check_trap_mmu_pmp_ok(zext_xlen(RS1), len, LOAD, PRV_U);

  if ((MTE_STRIDE_RD == 0) && (MTE_STRIDE_RS1 == 0)) {
      memcpy(dst, src, col * row * esize);
  }
  else {
    for (int i = 0; i < row; i++) {
      memcpy(dst + i * copy_stride_rd, src + i * copy_stride_rs1, col * esize);
    }
  }

  WRITE_MEM_STC(RD, (uint8_t*)dst, CMT_LOG_MTE);  

})

