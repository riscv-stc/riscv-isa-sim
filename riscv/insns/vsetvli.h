require_extension('V');
NCP_AUNIT();

if ((0 == insn.rs1()) && (0 == insn.rd())) {
    p->set_csr(CSR_VTYPE, (VSEW_I<<VTYPE_SEW_SHIFT) \
					   |(VEDIV_I<<VTYPE_EDIV_SHIFT) \
					   |(VLMUL_I<<VTYPE_LMUL_SHIFT));
} else if ((0 == insn.rs1()) && (0 != insn.rd())) {
    p->set_csr(CSR_VL, VLMAX);
    WRITE_RD(p->get_csr(VLMAX));
} else {
    p->set_csr(CSR_VTYPE, (VSEW_I<<VTYPE_SEW_SHIFT) \
    					   |(VEDIV_I<<VTYPE_EDIV_SHIFT) \
    					   |(VLMUL_I<<VTYPE_LMUL_SHIFT));

    //spec 0.18 19/10/10, vsetvl{i}指令当rs1指定为x0寄存器时，
    //vl不再被设置成VLMAX, 而是保持原值保持不变
    if (insn.rs1()) {
        if(RS1 <= VLMAX){
        	p->set_csr(CSR_VL, RS1);
        }
        else if((RS1 > VLMAX) && (RS1) <= 2*VLMAX){ //TODO
    	    p->set_csr(CSR_VL, RS1/2);
        }
        else if(RS1 > 2*VLMAX){
    	    p->set_csr(CSR_VL, VLMAX);
        }
    }

    WRITE_RD(p->get_csr(CSR_VL));
}
