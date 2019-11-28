auto proxy = p->get_proxy();
if (proxy == nullptr) return -1;

//TCP_AUNIT();
auto dst = *((unsigned int *)MMU.get_phy_addr(RD));
auto src = RS1 - 0xf8000000;
unsigned int col = DMA_SHAPE_COLUMN;
unsigned int row = DMA_SHAPE_ROW;
unsigned int stride = STRIDE_DDR;

for (int times = 0; times < 5; times++) {
  if (likely(proxy->dmaXfer(dst, src, Transport::AbstractProxy::LLB2DDR,
      col, row, stride )))
    break;
}
