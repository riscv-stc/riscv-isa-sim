using namespace Transport;
auto proxy = p->get_proxy();
if (proxy == nullptr) return -1;

TCP_AUNIT();
//class CustomInsns CusIns;
//struct ShapeStride sst;
//sst_fill(sst);
unsigned long src = RS1;
unsigned long dst = RD;
unsigned int core_map = MTE_CORE_MAP;
//2020-01-04: the bit of current core is no need to set but the data will send
core_map |= 0x1 << p->get_csr(CSR_TID);

GET_LLB_OFF(src, src);
//src shape
unsigned short col = MTE_SHAPE_COLUMN;
unsigned short row = MTE_SHAPE_ROW; 
//tcpXfer will deal with stride while stride is 0
unsigned short stride = STRIDE_LLB;

//llb --> l1; llb has stride, l1 no stride.
for (int core_id = 0; core_id < 32; core_id++)
{
    if (core_map & (0x1 << core_id)) {
        for (int times = 0; times < 5; times++) {
            if (likely(proxy->tcpXfer(0, core_id, dst, 0, col * row * 2, src,
                   Transport::AbstractProxy::LLB2CORE, col * 2, 0, stride)))
            break;
        }
    }
}
