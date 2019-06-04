auto trans = Transport::Interface::getInstance();
if (trans == nullptr) return -1;//FIXME?

trans->sync();
