
p->run_async([p, insn, pc]() {
  p->sync();
}, false);

wfi();
