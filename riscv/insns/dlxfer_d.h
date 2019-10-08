auto proxy = p->get_proxy();
if (proxy == nullptr) return -1;

//TCP_AUNIT();
auto dst = *((unsigned int *)MMU.get_phy_addr(RD));
auto src = RS1;
auto len = RS2;

proxy->dmaXfer(dst, src, len, Transport::AbstractProxy::LLB2DDR);
