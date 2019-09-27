auto trans = p->get_transport();
if (trans == nullptr) return -1;

TCP_AUNIT();
auto src = MMU.get_phy_addr(RS1);
auto dst = RD;

auto dataSize = RS2;
auto chipId = (TPARA0 >> TPARA0_CHIP_SHIFT) & TPARA0_CHIP_MASK;
auto coreId =(TPARA0 >> TPARA0_CORE_SHIFT) & TPARA0_CORE_MASK;
trans->tcpXfer(chipId, coreId, dst, src, dataSize + 1, 0, Transport::Interface::CORE2CORE);
