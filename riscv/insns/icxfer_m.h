auto trans = Transport::Interface::getInstance();
if (trans == nullptr) return -1;

auto dataAddr = MMU.get_phy_addr(RS1);  // FIXME RS1 mask with 0x7ffff
auto dataSize = RS2 & 0x7ffff;
auto chipId = RS2 & 0xf0000000;
chipId >>= 28;
auto coreId = RD & 0x1ff80000;
coreId >>= 19;
auto lut = RD & 0xe0000000;
lut >>= 29;
trans->send(chipId, coreId, dataAddr, dataSize, insn.i_imm(), lut);
