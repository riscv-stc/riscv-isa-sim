
printf("npc%d sync %lx\n", p->get_id(), pc);
p->run_async([p, insn, pc]() {
  p->sync();
}, false);

wfi();
