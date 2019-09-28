auto proxy = p->get_proxy();
if (proxy == nullptr) return -1;

TCP_AUNIT();
auto src = MMU.get_phy_addr(RS1);
auto dst = RD & 0x7ffff;
// FIXME: workaround because of current message size
auto dataSize = RS2 & 0x7ffff;
auto chipId = (RS2 & 0xf0000000) >> 28;
auto coreId = (RD & 0x1ff80000) >> 19;
proxy->tcpXfer(chipId, coreId, dst, src, dataSize + 1, 0, Transport::AbstractProxy::CORE2CORE);
