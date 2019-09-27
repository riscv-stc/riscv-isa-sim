auto trans = p->get_transport();
if (trans == nullptr) return -1;

//TCP_AUNIT();
auto dst = *((unsigned long long *)MMU.get_phy_addr(RD));
auto src = RS1;
auto len = RS2;

trans->dmaXfer(dst, src, len, Transport::Interface::LLB2DDR, 0);
