auto trans = Transport::Interface::getInstance();
if (trans == nullptr) return -1;

auto src = MMU.get_phy_addr(RS1);
auto dst = RD;

auto dataSize = RS2;
auto tmode =  TMODE & 0x1;
auto tag = 0; /*tag for RDMA*/
if (tmode == 0) /*Msg*/
    tag = (TPARA0 >> TPARA0_TAG_SHIFT) & TPARA0_TAG_MASK;
auto chipId = (TPARA0 >> TPARA0_CHIP_SHIFT) & TPARA0_CHIP_MASK;
auto coreId =(TPARA0 >> TPARA0_CORE_SHIFT) & TPARA0_CORE_MASK;
trans->tcpXfer(chipId, coreId, dst, src, dataSize + 1, 0, Transport::Interface::CORE2CORE, tmode, tag);
