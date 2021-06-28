int csr = validate_csr(insn.csr(), true);
reg_t old = p->get_csr(csr);
p->set_csr(csr, insn.rs1());
WRITE_RD(sext_xlen(old));
serialize();
if (csr >= 0x400 && csr <= 0x426)
   p->check_intersection(0, 0, 0);