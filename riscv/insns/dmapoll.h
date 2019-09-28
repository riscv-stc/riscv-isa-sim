auto proxy = p->get_proxy();
if (proxy == nullptr) return -1;

proxy->dmaXferPoll();
