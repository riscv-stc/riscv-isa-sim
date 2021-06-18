
reg_t rs2 = RS2;

p->run_async([p, insn, pc, rs2]() {
  p->sync(rs2);
});

wfi();
