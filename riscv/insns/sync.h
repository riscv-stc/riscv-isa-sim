check_ncp_vill_invalid()

p->run_async([p, insn, pc]() {
  p->sync();
}, false);

p->clearRequest(64);

wfi();