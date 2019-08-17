auto trans = Transport::Interface::getInstance();
if (trans == nullptr) return -1;//FIXME?

trans->sync();

//after sync done then dump memory. 
simif_t* sim = p->get_sim();
char fname[40] = {0};

sprintf(fname, "dump/output_mem2-%d@%d.dat", p->get_syncs(), p->get_id());
p->set_syncs(p->get_syncs() + 1);
sim->dump_heap(fname, 0x70000);

