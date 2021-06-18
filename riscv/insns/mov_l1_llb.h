uint8_t e_size = 2;

if (MTE_DATA_TYPE_RS1 == 0 || MTE_DATA_TYPE_RS1 == 1 ||
    MTE_DATA_TYPE_RS1 == 5 || MTE_DATA_TYPE_RS1 == 6)
    e_size = 2;
else if (MTE_DATA_TYPE_RS1 == 2 || MTE_DATA_TYPE_RS1 == 7 ||
  MTE_DATA_TYPE_RS1 == 8)
    e_size = 4;
else
    e_size = 1;

check_traps_mov_l1_llb(e_size);

uint8_t* src = (uint8_t*)p->get_sim()->addr_to_mem(zext_xlen(RS1));
uint8_t* dst = (uint8_t*)MMU.get_phy_addr(RD);

//src shape
unsigned short col = MTE_SHAPE_COLUMN;
unsigned short row = MTE_SHAPE_ROW; 
unsigned short copy_stride_rs1 = MTE_STRIDE_RS1 ? MTE_STRIDE_RS1 : (col * e_size);
unsigned short copy_stride_rd = MTE_STRIDE_RD ? MTE_STRIDE_RD : (col * e_size);

if ((MTE_STRIDE_RD == 0) && (MTE_STRIDE_RS1 == 0)) {
    memcpy(dst, src, col * row * e_size);
}
else {
  for (int i = 0; i < row; i++) {
    memcpy(dst + i * copy_stride_rd, src + i * copy_stride_rs1, col * e_size);
  }
}
