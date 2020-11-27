//check_traps_mov_l1_llb;
//check_ncp_vill_invalid()

uint8_t* src = (uint8_t*)p->get_sim()->addr_to_mem(zext_xlen(RS1));
uint8_t* dst = (uint8_t*)MMU.get_phy_addr(RD);

//src shape
unsigned short col = MTE_SHAPE_COLUMN;
unsigned short row = MTE_SHAPE_ROW; 
unsigned short stride = STRIDE_LLB ? STRIDE_LLB : 0;

if (stride == 0) {
  memcpy(dst, src, col * row * 2);
} else {
  for (int i = 0; i < row; i++) {
    memcpy(dst + i * col * 2, src + i * stride, col * 2);
  }
}