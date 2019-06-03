require_extension('V');

check_vstart{
        switch(SEW) {
                case 8:
			vector_for_each_from_zero(idx) {
				if (VRS2.vb[idx]) {
				    WRITE_REG(insn.rd(), idx);
				    break;
				}
				if ((idx == (VLMAX -1)) && (!VRS2.vb[idx])) {
				    WRITE_REG(insn.rd(), -1);
				}
			}
                break;

                case 16:
			vector_for_each_from_zero(idx) {
				if (VRS2.vh[idx]) {
					WRITE_REG(insn.rd(), idx);
					break;
				}
				if ((idx == (VLMAX -1)) && (!VRS2.vh[idx])) {
				    WRITE_REG(insn.rd(), -1);
				}
			}
                break;

                default:
                break;
        }
}
