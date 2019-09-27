auto trans = p->get_transport();
if (trans == nullptr) return -1;

trans->dmaXferPoll();
