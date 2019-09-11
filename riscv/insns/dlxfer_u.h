auto trans = Transport::Interface::getInstance();
if (trans == nullptr) return -1;

auto ddr = *((unsigned long long *)MMU.get_phy_addr(RS1));
auto llb = RD;
auto dataSize = RS2;

trans->dmaXfer(ddr, llb, Transport::Interface::DDR2LLB, dataSize);
