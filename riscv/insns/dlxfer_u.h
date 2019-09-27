auto trans = p->get_transport();
if (trans == nullptr) return -1;

//TCP_AUNIT();
auto addr = *((unsigned long long *)MMU.get_phy_addr(RS1));
auto data = MMU.get_phy_addr(addr);
auto dst = RD;
auto len = RS2;

trans->dmaXfer(0, dst, len, Transport::Interface::DDR2LLB, data);
