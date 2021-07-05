
p->run_async([p, insn, pc]() {
  p->sync(0);
}, false);

wfi();
