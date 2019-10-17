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
unsigned short stride = STRIDE_LLB ? STRIDE_LLB : col * 2;

//l1 --> llb, l1 no stride, llb has stride.
for (int i = 0; i < row; i++)
  proxy->tcpXfer(0, p->get_id(), dst + stride * i, src + i * col * 2, col * 2, 0, Transport::AbstractProxy::CORE2LLB);
