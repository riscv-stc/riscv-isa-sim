auto trans = Transport::Interface::getInstance();
if (trans == nullptr) return -1;

auto src = MMU.get_phy_addr(RS1);
auto dst = MMU.get_phy_addr(RD);
auto dataSize = RS2;

trans->dmaXfer(dst, src, Transport::Interface::DDR2LLB, dataSize);
