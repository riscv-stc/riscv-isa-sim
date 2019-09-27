auto trans = p->get_transport();
if (trans == nullptr) return -1;

TCP_AUNIT();
auto src = MMU.get_phy_addr(RS1);
auto dst = RD & 0x7ffff;
// FIXME: workaround because of current message size
auto dataSize = RS2 & 0x7ffff;
auto tag = (RS2 & 0xff80000) >> 19;
auto chipId = (RS2 & 0xf0000000) >> 28;
auto coreId = (RD & 0x1ff80000) >> 19;
auto lut = (RD & 0xe0000000) >> 29;
trans->tcpXfer(chipId, coreId, dst, src, dataSize + 1, 0, Transport::Interface::CORE2CORE, TMODE & 0x1, tag, lut);
