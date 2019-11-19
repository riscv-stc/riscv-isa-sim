auto proxy = p->get_proxy();
if (proxy == nullptr) return -1;

//TCP_AUNIT();
auto dst = RD - 0xf8000000;
auto src = *((unsigned int *)MMU.get_phy_addr(RS1));

unsigned short col = DMA_SHAPE_COLUMN;
unsigned short row = DMA_SHAPE_ROW;
unsigned int stride = STRIDE_DDR;

for (int times = 0; times < 5; times++) {
  if (likely(proxy->dmaXfer(src, dst, Transport::AbstractProxy::DDR2LLB,
    col, row, stride)))
    break;
}
