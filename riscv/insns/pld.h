using namespace Transport;
auto proxy = p->get_proxy();
if (proxy == nullptr) return -1;

// FIXME: should implement tcp access trap
//class CustomInsns CusIns;
//struct ShapeStride sst;
//sst_fill(sst);
unsigned long src = RS1;
uint8_t* dst = (uint8_t*)MMU.get_phy_addr(RD);
unsigned int core_map = MTE_CORE_MAP;
#if 0
//2020-01-04: the bit of current core is no need to set but the data will send
core_map |= 0x1 << p->get_csr(CSR_TID);
#endif
GET_LLB_OFF(src, src);
//src shape
unsigned short col = MTE_SHAPE_COLUMN;
unsigned short row = MTE_SHAPE_ROW; 
unsigned short stride = STRIDE_LLB ? STRIDE_LLB : 0;

unsigned int core_id = p->get_csr(CSR_TID);
if (core_map & (1 << core_id)) {
    for (int times = 0; times < 5; times++) {
    if (likely(proxy->llbLoad(src, col * row * 2, dst, col * 2, 0, stride, core_map)))
        break;
    }
}
