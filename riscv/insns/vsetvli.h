require_extension('V');
// FIXME: should implement ncp access trap

reg_t rd = insn.rd();
reg_t rs1 = insn.rs1();
reg_t vtypei = insn.z_imm();

// check if vtype supported
if ((8<<((vtypei>>VTYPE_SEW_SHIFT) & VTYPE_VSEW) != 16) ||
    (1<<((vtypei>>VTYPE_LMUL_SHIFT) & VTYPE_VLMUL) != 1)) {
    // not support, set vill bit, and clear others
    p->set_vtype(1 << (p->get_xlen()-1));
    if (rd != 0) WRITE_RD(0);
} else {
    // we only support VSEW=16(SEW=1), VLMUL=1(LMUL=0)

    // set vtype
    p->set_vtype(vtypei);

    // set vl
    reg_t vl = p->get_csr(CSR_VL);
    if (VLMAX == 0) {
        vl = 0;
    } else if (rd == 0 && rs1 == 0) {
        vl = vl > VLMAX ? VLMAX : vl;
    } else if (rd != 0 && rs1 == 0) {
        vl = VLMAX;
    } else if (rs1 != 0) {
        if (RS1 <= VLMAX)
            vl = RS1;
        else if (RS1 <= VLMAX * 2)
            vl = (RS1 + 1)/ 2;
        else
            vl = VLMAX;
    }
    p->set_csr(CSR_VL, vl);

    // set rd
    if (rd != 0) WRITE_RD(vl);
}