using namespace Transport;
auto proxy = p->get_proxy();
if (proxy == nullptr) return -1;

TCP_AUNIT();
//class CustomInsns CusIns;
//struct ShapeStride sst;
//sst_fill(sst);
uint8_t* src = (uint8_t*)MMU.get_phy_addr(RS1);
uint32_t dst = RD;
GET_LLB_OFF(dst, dst);

//src shape
unsigned short col = MTE_SHAPE_COLUMN;
unsigned short row = MTE_SHAPE_ROW; 
unsigned short stride = STRIDE_LLB ? STRIDE_LLB : 0;

for (int times = 0; times < 5; times++) {
  if (likely(proxy->llbStore(dst, col * row * 2, src, col * 2, stride, 0)))
    break;
}