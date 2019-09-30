auto proxy = p->get_proxy();
if (proxy == nullptr) return -1;

//TCP_AUNIT();
auto dst = RD;
auto src = *((unsigned long long *)MMU.get_phy_addr(RS1));
auto len = RS2;

proxy->dmaXfer(src, dst, len, Transport::AbstractProxy::DDR2LLB);
