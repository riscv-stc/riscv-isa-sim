#include "config.h"
#include "matrix_unit.h"
#include "processor.h"
#include "arith.h"

void matrixUnit_t::reset(){
  free(tr_file);
  tr_file = malloc(mlenb * NMPR);
  tr_renamefile = malloc(mlenb * NMPR);
  memset(tr_file, 0, mlenb * NMPR);
  memset(tr_renamefile, 0, mlenb * NMPR);
  
  auto& csrmap = p->get_state()->csrmap;
  csrmap[CSR_MXSAT] = mxsat = std::make_shared<vxsat_csr_t>(p, CSR_MXSAT);
  csrmap[CSR_MTILEM] = tile_m = std::make_shared<matrix_csr_t>(p, CSR_MTILEM, 0);
  csrmap[CSR_MTILEK] = tile_k = std::make_shared<matrix_csr_t>(p, CSR_MTILEK, /*mask*/ 0);
  csrmap[CSR_MTILEN] = tile_n = std::make_shared<matrix_csr_t>(p, CSR_MTILEN, /*mask*/ 0);
  csrmap[CSR_MLENB] = std::make_shared<matrix_csr_t>(p, CSR_MLENB, /*mask*/ mlenb);
  csrmap[CSR_MRLENB] = std::make_shared<matrix_csr_t>(p, CSR_MRLENB, /*mask*/ mrlenb);
  csrmap[CSR_MTYPE] = mtype = std::make_shared<matrix_csr_t>(p, CSR_MTYPE, /*mask*/ 0);
  csrmap[CSR_MSTART] = mstart = std::make_shared<matrix_csr_t>(p, CSR_MSTART, /*mask*/ 0);
  csrmap[CSR_MCSR] = mstart = std::make_shared<matrix_csr_t>(p, CSR_MCSR, /*mask*/ 0);
  csrmap[CSR_MXRM] = mxrm = std::make_shared<matrix_csr_t>(p, CSR_MXRM, /*mask*/ 0x3ul);
  csrmap[CSR_MOUTSH] = moutshape = std::make_shared<matrix_csr_t>(p, CSR_MOUTSH, 0);
  csrmap[CSR_MINSH] = minshape = std::make_shared<matrix_csr_t>(p, CSR_MINSH, 0);
  csrmap[CSR_MSTDI] = mstdi = std::make_shared<matrix_csr_t>(p, CSR_MSTDI, 0);
  csrmap[CSR_MPAD] = mpad = std::make_shared<matrix_csr_t>(p, CSR_MPAD, 0);
  csrmap[CSR_MINSK] = minsk = std::make_shared<matrix_csr_t>(p, CSR_MINSK, 0);
  csrmap[CSR_MOUTSK] = moutsk = std::make_shared<matrix_csr_t>(p, CSR_MOUTSK, 0);
  csrmap[CSR_MPADVAL] = mpadval = std::make_shared<matrix_csr_t>(p, CSR_MPADVAL, 0);
  mtype->write_raw(0);

  set_mtype(0, -1);
}

reg_t matrixUnit_t::set_mtype(int rd, reg_t newType) {
  bool mfp64_ext = false;
  if (mtype->read() != newType){
    mint4 = extract64(newType, 10, 1);
    mfp8 = extract64(newType, 9, 1);
    mtf32 = extract64(newType, 8, 1);
    mbf16 = extract64(newType, 7, 1);
    mfp64 = extract64(newType, 6, 1);
    if (mfp64){
      if(!p->get_isa().extension_enabled('D'))
      {
        mfp64_ext = true;
      }
    }
    mba = extract64(newType, 5, 1);

    msew = 1 << (extract64(newType, 2, 3) + 3);
    mlmul = 1 << extract64(newType, 0, 2);
    mlmax = (MLEN/msew) * mlmul;

    mill = (newType >> 11) != 0 || mfp64_ext || msew > RLEN;

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


reg_t matrixUnit_t::set_ml(int rd, int rs1, reg_t newMlen, char dim) {
  
  if (mlmax == 0)
      return 0;

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

