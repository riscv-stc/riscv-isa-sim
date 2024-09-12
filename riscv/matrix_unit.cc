#include "config.h"
#include "matrix_unit.h"
#include "processor.h"
#include "arith.h"

void matrixUnit_t::reset(){
  free(tr_file);
  tr_file = malloc(mlenb * NMTRPR);
  acc_file = malloc(mlenb * NMTRPR * MAMUL_MAX);
  tr_renamefile = malloc(mlenb * NMTRPR);
  acc_renamefile = malloc(mlenb * NMACCPR * MAMUL_MAX);
  memset(tr_file, 0, mlenb * NMTRPR);
  memset(acc_file, 0, mlenb * NMACCPR * MAMUL_MAX);
  memset(tr_renamefile, 0, mlenb * NMACCPR);
  memset(acc_renamefile, 0, mlenb * NMACCPR * MAMUL_MAX);
  auto& csrmap = p->get_state()->csrmap;
  // mstrix 0.5 del
  // csrmap[CSR_MXSAT] = mxsat = std::make_shared<vxsat_csr_t>(p, CSR_MXSAT);
  csrmap[CSR_MTILEM] = tile_m = std::make_shared<matrix_csr_t>(p, CSR_MTILEM, 0, 0);
  csrmap[CSR_MTILEK] = tile_k = std::make_shared<matrix_csr_t>(p, CSR_MTILEK, /*mask*/ 0, 0);
  csrmap[CSR_MTILEN] = tile_n = std::make_shared<matrix_csr_t>(p, CSR_MTILEN, /*mask*/ 0, 0);
  csrmap[CSR_MLENB] = std::make_shared<matrix_csr_t>(p, CSR_MLENB, /*mask*/ 0, /*init*/ mlenb);
  csrmap[CSR_MRLENB] = std::make_shared<matrix_csr_t>(p, CSR_MRLENB, /*mask*/ 0, mrlenb);
  csrmap[CSR_MTYPE] = mtype = std::make_shared<matrix_csr_t>(p, CSR_MTYPE, /*mask*/ 0);
  csrmap[CSR_MSTART] = mstart = std::make_shared<matrix_csr_t>(p, CSR_MSTART, /*mask*/ 0);
  csrmap[CSR_MCSR] = mstart = std::make_shared<matrix_csr_t>(p, CSR_MCSR, /*mask*/ 0);
  // csrmap[CSR_MXRM] = mxrm = std::make_shared<matrix_csr_t>(p, CSR_MXRM, /*mask*/ 0x3ul); // mtrix 0.5 del
  csrmap[CSR_MOUTSH] = moutshape = std::make_shared<matrix_csr_t>(p, CSR_MOUTSH, 0);
  csrmap[CSR_MINSH] = minshape = std::make_shared<matrix_csr_t>(p, CSR_MINSH, 0);
  csrmap[CSR_MSTDI] = mstdi = std::make_shared<matrix_csr_t>(p, CSR_MSTDI, 0);
  csrmap[CSR_MPAD] = mpad = std::make_shared<matrix_csr_t>(p, CSR_MPAD, 0);
  csrmap[CSR_MINSK] = minsk = std::make_shared<matrix_csr_t>(p, CSR_MINSK, 0);
  csrmap[CSR_MOUTSK] = moutsk = std::make_shared<matrix_csr_t>(p, CSR_MOUTSK, 0);
  csrmap[CSR_MPADVAL] = mamul = std::make_shared<matrix_csr_t>(p, CSR_MPADVAL, 0);
  csrmap[CSR_MAMUL] = mpadval = std::make_shared<matrix_csr_t>(p, CSR_MAMUL, 1); // default value is 1
  csrmap[CSR_MTSP] = mtsp = std::make_shared<matrix_csr_t>(p, CSR_MTSP, 0);
  csrmap[CSR_MDSP] = mdsp = std::make_shared<matrix_csr_t>(p, CSR_MDSP, 0);
  mtype->write_raw(0);

  set_mtype(0, -1);
}

reg_t matrixUnit_t::set_mtype(int rd, reg_t newType) {
  bool mfp64_ext = false;
  if (mtype->read() != newType){
    mint4 = extract64(newType, 3, 1);
    mint8 = extract64(newType, 4, 1);
    mint16 = extract64(newType, 5, 1);
    mint32 = extract64(newType, 6, 1);
    mint64 = extract64(newType, 7, 1);
    mfp8 = extract64(newType, 8, 2);
    mfp32 = extract64(newType, 12, 2);
    mfp16 = extract64(newType, 10, 2);
    mfp64 = extract64(newType, 14, 1);
    if (mfp64){
      if(!p->get_isa().extension_enabled('D'))
      {
        mfp64_ext = true;
      }
    }
    mba = extract64(newType, 15, 1);
    // int4 in msew with 0x111
    msew = 1 << ((extract64(newType, 0, 3) + 3) & 0x7);

    mill = (newType >> 16) != 0 || mfp64_ext || msew > RLEN;

    if (mill){
      mlmax = 0;
      mtype->write_raw(UINT64_MAX << (p->get_xlen() -1));
    }
    else{
      mtype->write_raw(newType);
    }
  }
  mstart->write_raw(0);
  return mtype->read();
}

reg_t matrixUnit_t::set_mtypei(int rd, reg_t newType){
  reg_t type = (((mtype->read() & 0x3FF ) & ~((1UL << 10) - 1)) | newType) ;
  return set_mtype(rd, type);
}

reg_t matrixUnit_t::set_mtypehi(int rd, reg_t newType){
  reg_t type = ((mtype->read() & 0x3FF) | newType | ((mtype->read() >> 20) << 20));
  return set_mtype(rd, type);
}

reg_t matrixUnit_t::set_msew(int rd, reg_t newType){
  reg_t type = ((mtype->read() & ~((1UL << 2) - 1)) | newType);
  return set_mtype(rd, type);
}

reg_t matrixUnit_t::set_mint(int rd, reg_t newType, reg_t bit){
  reg_t type = 0;
  if (newType){
    type = mtype->read() | (1UL << bit);
  }
  else {
    type = mtype->read() & ~(1UL << bit);
  }
  
  return set_mtype(rd, type);
}

reg_t matrixUnit_t::set_fp(int rd, reg_t newType, reg_t bit){
  reg_t type = mtype->read();
  if ( bit == 14 )
    return set_mint(rd, newType, bit);
  reg_t mask = 0x3UL << bit;
  // clear old value;
  type &= ~mask;
  // set new value;
  type |= (type << bit) & mask;
  return set_mtype(rd, type);
}

reg_t matrixUnit_t::set_ba(int rd, reg_t newType){
  return set_mint(rd, newType, 15); // bit15 used mba
}

reg_t matrixUnit_t::set_ml(int rd, int rs1, reg_t newMlen, char dim) {
  
  // if (mlmax == 0)
  //     return 0;

  reg_t MMAX = mrows;
  reg_t NMAX = mcols / msew;
  reg_t KMAX = std::min(MMAX, NMAX);

  if (dim == 'm' || dim == 'M') {
    
    if (rs1 != 0) {
      tile_m->write_raw(newMlen > MMAX? MMAX : newMlen);
    } else if (rs1 == 0 && rd != 0) {
      tile_m->write_raw(MMAX);
    }
    return tile_m->read();
  } else if (dim == 'k' || dim == 'K') {
    if (rs1 != 0) {
      tile_k->write_raw(newMlen > KMAX? KMAX : newMlen);
    } else if (rs1 == 0 && rd != 0) {
      tile_k->write_raw(KMAX);
    }
    return tile_k->read();
  } else if (dim == 'n' || dim == 'N') {
    if (rs1 != 0) {
      tile_n->write_raw(newMlen > NMAX? NMAX : newMlen);
    } else if (rs1 == 0 && rd != 0) {
      tile_n->write_raw(NMAX);
    }
    return tile_n->read();
  } else if(dim == 't' || dim == 'T'){
    if (rs1 != 0){
      tile_m->write_raw((newMlen & 0xFF) > MMAX ? MMAX : (newMlen & 0xFF));
      tile_n->write_raw(((newMlen >> 8) & 0xFF) > NMAX ? NMAX : ((newMlen >> 8) & 0xFF));
      tile_k->write_raw((newMlen >> 16) > KMAX ? KMAX : (newMlen >> 16));
    }else if( rs1 == 0 &&rd != 0){
      tile_k->write_raw(KMAX);
      tile_n->write_raw(NMAX);
      tile_m->write_raw(MMAX);
    }

    return (0xFF & tile_m->read()) | ((0xFF & tile_n->read()) << 8) | ((0xFF & tile_k->read()) << 16);
  } else {
    return 0;
  }

}

reg_t matrixUnit_t::set_moutsh(int rd, int rs1, int rs2){
  outshape[1] = extract64(rs1, 16,  16);
  outshape[0] = extract64(rs1, 0,  16);
  mstr_w = extract64(rs2, 0,  8);
  mstr_h = extract64(rs2, 8,  8);
  mdil_w = extract64(rs2, 16, 8);
  mdil_h = extract64(rs2, 24, 8);
  mstdi->write_raw(rs2 & 0xFFFFFFFF);
  moutshape->write_raw(rs1 & 0xFFFFFFFF);

  return moutshape->read();
}

reg_t matrixUnit_t::set_insh(int rd, int rs1, int rs2) {
  inshape[1] = extract64(rs1, 16, 16);
  inshape[0] = extract64(rs1, 0, 16);
  mpad_right  = extract64(rs2, 0,  8);
  mpad_left   = extract64(rs2, 8,  8);
  mpad_bottom = extract64(rs2, 16, 8);
  mpad_top    = extract64(rs2, 24, 8);
  minshape->write_raw(rs1 & 0xFFFFFFFF);
  return minshape->read();
}

reg_t matrixUnit_t::set_msk(int rd, int rs1, int rs2) {
  mskin[0] = int16_t(extract64(rs1, 0,  16));
  mskin[1] = int16_t(extract64(rs1, 16, 16));

  mskout[0] = extract64(rs2, 0,  16);
  mskout[1] = extract64(rs2, 16, 16);
  minsk->write_raw(rs1 & 0xFFFFFFFF);
  moutsk->write_raw(rs2 & 0xFFFFFFFF);
  return minsk->read();
}

reg_t matrixUnit_t::set_pad(int rd, int rs1) {
  mpadval->write_raw(rs1 & 0xFFFFFFFF);
  return mpadval->read();
}

reg_t matrixUnit_t::set_tsp(int rd, int rs1) {
  mtsp->write_raw(rs1 & 0xF);
  return mtsp->read();
}
reg_t matrixUnit_t::set_dsp(int rd, int rs1) {
  mdsp->write_raw(rs1 & 0xF);
  return mdsp->read();
}

