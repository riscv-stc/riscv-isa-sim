uint8_t e_size = 2;

if (MTE_DATA_TYPE_RS1 == 0 || MTE_DATA_TYPE_RS1 == 1 ||
    MTE_DATA_TYPE_RS1 == 5 || MTE_DATA_TYPE_RS1 == 6)
    e_size = 2;
else if (MTE_DATA_TYPE_RS1 == 2)
    e_size = 4;
else
    e_size = 1;

check_traps_pld(e_size);
//check_ncp_vill_invalid()

reg_t addr = zext_xlen(RS1);
reg_t dst_addr = RD;
reg_t rs2 = RS2;
p->run_async([p, insn, pc, xlen, addr, dst_addr, e_size, rs2]() {
  uint8_t* src = (uint8_t*)p->get_sim()->addr_to_mem(addr);
  uint8_t* dst = (uint8_t*)MMU.get_phy_addr(dst_addr);
  unsigned int core_map = (unsigned int)rs2;

  // do sync for pld
  p->pld(core_map);

  #if 0
  //2020-01-04: the bit of current core is no need to set but the data will send
  core_map |= 0x1 << p->get_csr(CSR_TID);
  #endif

  //src shape
  unsigned short col = MTE_SHAPE_COLUMN;
  unsigned short row = MTE_SHAPE_ROW; 
  unsigned short copy_stride_rs1 = 0;
  unsigned short copy_stride_rd = 0;
  unsigned int core_id = p->get_csr(CSR_TID);

  if (core_map != 0 && (core_map & (1 << core_id))) {
    copy_stride_rs1 = MTE_STRIDE_RS1 ? MTE_STRIDE_RS1 : (col * e_size);
    copy_stride_rd = MTE_STRIDE_RD ? MTE_STRIDE_RD : (col * e_size);

    if ((MTE_STRIDE_RD == 0) && (MTE_STRIDE_RS1 == 0)) {
      memcpy(dst, src, col * row * e_size);
    }
    else {
      for (int i = 0; i < row; i++) {
        memcpy(dst + i * copy_stride_rd, src + i * copy_stride_rs1, col * e_size);
      }
    }
  }
});

wfi();
