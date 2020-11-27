//check_traps_mov_llb_l1;
//check_ncp_vill_invalid()

uint8_t* src = (uint8_t*)MMU.get_phy_addr(RS1);
uint8_t* dst = (uint8_t*)p->get_sim()->addr_to_mem(zext_xlen(RD));

//src shape
unsigned short col = MTE_SHAPE_COLUMN;
unsigned short row = MTE_SHAPE_ROW; 
unsigned short stride = STRIDE_LLB ? STRIDE_LLB : 0;

if (stride == 0) {
  memcpy(dst, src, col * row * 2);
} else {
  for (int i = 0; i < row; i++) {
    memcpy(dst + i * stride, src + i * col * 2, col * 2);
  }
}
