if (!p->get_sim()->getMultiCoreThreadFlag())
  p->run_async([p, insn, pc]() {
    p->sync();
  }, false);
else{
  p->set_run_async_state_start(false);
  p->sync();
  p->set_run_async_state_finish(false);
}
wfi();
