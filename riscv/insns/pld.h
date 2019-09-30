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

//src shape
unsigned short col = MTE_SHAPE_COLUMN;
unsigned short row = MTE_SHAPE_ROW; 
unsigned short stride = STRIDE_LLB;

//llb --> l1; llb has stride, l1 no stride.
for (int core_id = 0; core_id < 32; core_id++)
{
    if (core_map & (0x1 << core_id)) {
        for (int i = 0; i < row; i++)
	    proxy->tcpXfer(0, core_id, dst + i * col * 2, src + i * stride, \
			    col * 2, 0, Transport::AbstractProxy::LLB2CORE);
    }
}

