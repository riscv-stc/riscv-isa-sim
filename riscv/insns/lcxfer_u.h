auto trans = Transport::Interface::getInstance();
if (trans == nullptr) return -1;

auto src = RS1;
auto dst = RD;
auto dataSize = RS2;

trans->tcpXfer(0, 0, dst, nullptr, dataSize, src, Transport::Interface::LLB2CORE);
