using namespace Transport;
auto trans = Transport::Interface::getInstance();
if (trans == nullptr) return -1;

TCP_AUNIT();
auto src = MMU.get_phy_addr(RS1);
auto dst = RD;
auto dataSize = RS2;
trans->tcpXfer(0, 0, dst, src, dataSize, 0, Transport::Interface::CORE2LLB);
