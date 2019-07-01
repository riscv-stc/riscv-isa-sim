auto trans = Transport::Interface::getInstance();
if (trans == nullptr) return -1;

auto dataAddr = MMU.get_phy_addr(RS1 & 0x7ffff);
auto dataSize = RS2 & 0xffff;
auto mark = (RS2 & 0x3ff0000) >> 16;
auto tag = (RS2 & 0xc000000) >> 26;
auto chipId = RS2 & 0xf0000000;
chipId >>= 28;
auto coreId = RD & 0x1ff80000;
coreId >>= 19;
trans->send(chipId, coreId, dataAddr, dataSize, insn.i_imm(), tag, mark);
