using namespace Transport;
auto proxy = p->get_proxy();
if (proxy == nullptr) return -1;

TCP_AUNIT();
//class CustomInsns CusIns;
//struct ShapeStride sst;
//sst_fill(sst);
unsigned long src = MMU.get_phy_addr(RS1);
unsigned long dst = RD;
dst -= 0xf8000000;

//src shape
unsigned short col = MTE_SHAPE_COLUMN;
unsigned short row = MTE_SHAPE_ROW; 
unsigned short stride = STRIDE_LLB ? STRIDE_LLB : 0;

for (int times = 0; times < 5; times++) {
  if (likely(proxy->tcpXfer(0, 0, dst, src, col * row * 2, 0,
               Transport::AbstractProxy::CORE2LLB, col * 2, stride, 0)))
    break;
}