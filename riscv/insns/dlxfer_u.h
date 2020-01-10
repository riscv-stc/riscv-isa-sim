auto proxy = p->get_proxy();
if (proxy == nullptr) return -1;

//TCP_AUNIT();
unsigned int dst;
GET_LLB_OFF(RD, dst);
auto src = *((unsigned int *)MMU.get_phy_addr(RS1));

unsigned int col = DMA_SHAPE_COLUMN;
unsigned int row = DMA_SHAPE_ROW;
unsigned int stride = STRIDE_DDR;

for (int times = 0; times < 5; times++) {
  if (likely(proxy->dmaXfer(dst, src, Transport::AbstractProxy::DDR2LLB,
    col, row, stride)))
    break;
}
