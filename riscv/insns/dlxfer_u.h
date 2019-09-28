auto proxy = p->get_proxy();
if (proxy == nullptr) return -1;

//TCP_AUNIT();
auto addr = *((unsigned long long *)MMU.get_phy_addr(RS1));
auto data = MMU.get_phy_addr(addr);
auto dst = RD;
auto len = RS2;

proxy->dmaXfer(0, dst, len, Transport::AbstractProxy::DDR2LLB, data);
