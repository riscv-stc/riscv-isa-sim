
p->run_async([p, insn, pc]() {
  p->sync();
});

wfi();
