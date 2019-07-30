auto trans = Transport::Interface::getInstance();
if (trans == nullptr) return -1;

auto src = MMU.get_phy_addr(RS1);
auto dst = RD & 0x7ffff;

// FIXME: workaround because of current message size
auto dataSize = RS2 & 0x7ffff;
auto tag = (RS2 & 0xff80000) >> 19;
auto chipId = (RS2 & 0xf0000000) >> 28;
auto coreId = (RD & 0x1ff80000) >> 19;
trans->send(chipId, coreId, dst, src, dataSize, insn.rm(), tag);
