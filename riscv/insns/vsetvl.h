require_extension('V');
p->set_csr(CSR_VTYPE, RS2);
if(RS1 <= VLMAX){
	p->set_csr(CSR_VL, RS2);
}
else if((RS1 > VLMAX) && (RS1) <= 2*VLMAX){ //TODO
	p->set_csr(CSR_VL, RS1/2);
}
else if(RS1 > 2*VLMAX){
	p->set_csr(CSR_VL, VLMAX);
}
WRITE_RD(p->get_csr(CSR_VL));
