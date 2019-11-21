using namespace Transport;
auto proxy = p->get_proxy();
if (proxy == nullptr) return -1;

TCP_AUNIT();
//class CustomInsns CusIns;
//struct ShapeStride sst;
//sst_fill(sst);
uint64_t src = RS1 & 0xffffffff;
uint8_t* dst = (uint8_t*)MMU.get_phy_addr(RD);
src -= 0xf8000000;

//src shape
unsigned short col = MTE_SHAPE_COLUMN;
unsigned short row = MTE_SHAPE_ROW; 
unsigned short stride = STRIDE_LLB ? STRIDE_LLB : 0;

for (int times = 0; times < 5; times++) {
  if (likely(proxy->llbLoad(src, col * row * 2, dst, col * 2, 0, stride)))
    break;
}