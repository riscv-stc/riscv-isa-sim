
p->run_async([p, insn, pc]() {
  p->sync_start();
  while (!p->sync_done()) {
    usleep(100);
  }
});

wfi();