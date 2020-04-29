using namespace Transport;
auto proxy = p->get_proxy();
if (proxy == nullptr) return -1;

// FIXME: should implement tcp access trap
//class CustomInsns CusIns;
//struct ShapeStride sst;
//sst_fill(sst, 2, 2);
uint32_t src = RS1;
uint8_t* dst = (uint8_t*)MMU.get_phy_addr(RD);
GET_LLB_OFF(src, src);

//src shape
unsigned short col = MTE_SHAPE_COLUMN;
unsigned short row = MTE_SHAPE_ROW; 
unsigned short stride = STRIDE_LLB ? STRIDE_LLB : 0;

for (int times = 0; times < 5; times++) {
  if (likely(proxy->llbLoad(src, col * row * 2, dst, col * 2, 0, stride)))
    break;
}
