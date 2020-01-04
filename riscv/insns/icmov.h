auto proxy = p->get_proxy();
if (proxy == nullptr) return -1;

TCP_AUNIT();
auto src = MMU.get_phy_addr(RS1);
auto dst = RD;

auto dataSize = RS2;
auto chipId = DST_CHIP_ID;
auto coreId = DST_CORE_ID;

for (int times = 0; times < 5; times++) {
  //2020-01-04: datasize no need to add one
  if (likely(proxy->tcpXfer(chipId, coreId, dst, src, dataSize, 0, Transport::AbstractProxy::CORE2CORE)))
    break;
}

