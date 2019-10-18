auto proxy = p->get_proxy();
if (proxy == nullptr) return -1;

//TCP_AUNIT();
auto dst = RD - 0xf8000000;
auto src = *((unsigned int *)MMU.get_phy_addr(RS1));
auto len = RS2;

for (int times = 0; times < 5; times++) {
  if (likely(proxy->dmaXfer(src, dst, len, Transport::AbstractProxy::DDR2LLB)))
    break;
}
