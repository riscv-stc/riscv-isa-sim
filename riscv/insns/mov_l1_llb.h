using namespace Transport;
auto proxy = p->get_proxy();
if (proxy == nullptr) return -1;

TCP_AUNIT();
//class CustomInsns CusIns;
//struct ShapeStride sst;
//sst_fill(sst);
unsigned long src = RS1;
unsigned long dst = RD;
src -= 0xf8000000;

//src shape
unsigned short col = MTE_SHAPE_COLUMN;
unsigned short row = MTE_SHAPE_ROW; 
unsigned short stride = STRIDE_LLB ? STRIDE_LLB : 0;

for (int times = 0; times < 5; times++) {
  if (likely(proxy->tcpXfer(0, p->get_id(), dst, 0, col * row * 2, src,
               Transport::AbstractProxy::LLB2CORE, col * 2, 0, stride)))
    break;
}