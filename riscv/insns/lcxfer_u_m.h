auto trans = Transport::Interface::getInstance();
if (trans == nullptr) return -1;

auto src = MMU.get_phy_addr(RS1);
auto dst = MMU.get_phy_addr(RD);
auto dataSize = RS2;

trans->tcpXfer(0, 0, dst, nullptr, dataSize, src, Transport::Interface::LLB2CORE);
