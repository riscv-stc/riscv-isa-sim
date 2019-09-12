auto trans = Transport::Interface::getInstance();
if (trans == nullptr) return -1;

TCP_AUNIT();
auto ddr = *((unsigned long long *)MMU.get_phy_addr(RD));
auto llb = RS1;
auto dataSize = RS2;

trans->dmaXfer(ddr, llb, Transport::Interface::LLB2DDR, dataSize);
