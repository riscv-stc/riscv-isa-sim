using namespace Transport;
auto proxy = p->get_proxy();
if (proxy == nullptr) return -1;

TCP_AUNIT();
auto src = MMU.get_phy_addr(RS1);
auto dst = RD;
auto dataSize = RS2;
proxy->tcpXfer(0, 0, dst, src, dataSize, 0, Transport::AbstractProxy::CORE2LLB);
