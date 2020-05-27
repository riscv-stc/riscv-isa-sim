check_traps_pld;

p->run_async([p, insn, pc, xlen]() {
  uint8_t* src = p->get_sim()->addr_to_mem(zext_xlen(RS1));
  uint8_t* dst = (uint8_t*)MMU.get_phy_addr(RD);
  unsigned int core_map = MTE_CORE_MAP;

  // do sync for pld
  p->pld(core_map);

  #if 0
  //2020-01-04: the bit of current core is no need to set but the data will send
  core_map |= 0x1 << p->get_csr(CSR_TID);
  #endif

  //src shape
  unsigned short col = MTE_SHAPE_COLUMN;
  unsigned short row = MTE_SHAPE_ROW; 
  unsigned short stride = STRIDE_LLB ? STRIDE_LLB : 0;

  unsigned int core_id = p->get_csr(CSR_TID);

  if (core_map & (1 << core_id)) {
      if (stride == 0) {
          memcpy(dst, src, col * row * 2);
      } else {
          for (int i = 0; i < row; i++) {
              memcpy(dst + i * col * 2, src + i * stride, col * 2);
          }
      }
  }
});

wfi();
