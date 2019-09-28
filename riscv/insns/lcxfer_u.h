auto proxy = p->get_proxy();
if (proxy == nullptr) return -1;

TCP_AUNIT();
auto src = RS1;
auto dst = RD;
auto dataSize = RS2;

proxy->tcpXfer(0, 0, dst, nullptr, dataSize, src, Transport::AbstractProxy::LLB2CORE);
