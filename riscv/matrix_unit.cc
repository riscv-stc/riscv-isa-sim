#include "config.h"
#include "matrix_unit.h"
#include "processor.h"
#include "arith.h"

void matrixUnit_t::reset(){
  free(tr_file);
  tr_file = malloc(mlenb * 8);
  memset(tr_file, 0, mlenb * 8);
  
  auto& csrmap = p->get_state()->csrmap;
  csrmap[CSR_MXSAT] = mxsat = std::make_shared<vxsat_csr_t>(p, CSR_MXSAT);
  csrmap[CSR_MTILEM] = std::make_shared<matrix_csr_t>(p, CSR_MTILEM, 0);
  csrmap[CSR_MTILEK] = std::make_shared<matrix_csr_t>(p, CSR_MTILEK, /*mask*/ 0);
  csrmap[CSR_MTILEN] = std::make_shared<matrix_csr_t>(p, CSR_MTILEN, /*mask*/ 0);
  csrmap[CSR_MLENB] = std::make_shared<matrix_csr_t>(p, CSR_MLENB, /*mask*/ mlenb);
  csrmap[CSR_MRLENB] = std::make_shared<matrix_csr_t>(p, CSR_MRLENB, /*mask*/ mlenb);
  csrmap[CSR_MTYPE] = mtype = std::make_shared<matrix_csr_t>(p, CSR_MTYPE, /*mask*/ 0);
  csrmap[CSR_MSTART] = mstart = std::make_shared<matrix_csr_t>(p, CSR_MSTART, /*mask*/ 0);
  csrmap[CSR_MCSR] = mstart = std::make_shared<matrix_csr_t>(p, CSR_MCSR, /*mask*/ 0);
  csrmap[CSR_MXRM] = mxrm = std::make_shared<matrix_csr_t>(p, CSR_MXRM, /*mask*/ 0x3ul);
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

    mill = (newType >> 11) != 0 || mfp64_ext;
    

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
      tile_m = newMlen > MMAX? MMAX : newMlen;
    } else if (rs1 == 0 && rd != 0) {
      tile_m = MMAX;
    }
    return tile_m;
  } else if (dim == 'k' || dim == 'K') {
    if (rs1 != 0) {
      tile_k = newMlen > KMAX? KMAX : newMlen;
    } else if (rs1 == 0 && rd != 0) {
      tile_k = KMAX;
    }
    return tile_k;
  } else if (dim == 'n' || dim == 'N') {
    if (rs1 != 0) {
      tile_n = newMlen > NMAX? NMAX : newMlen;
    } else if (rs1 == 0 && rd != 0) {
      tile_n = NMAX;
    }
    return tile_n;
  } else if(dim == 't' || dim == 'T'){
    if (rs1 != 0){
      tile_m = (newMlen & 0xFF) > MMAX ? MMAX : (newMlen & 0xFF);
      tile_n = ((newMlen >> 8) & 0xFF) > NMAX ? NMAX : ((newMlen >> 8) & 0xFF);
      tile_k = (newMlen >> 16) > KMAX ? KMAX : (newMlen >> 16);
    }else if( rs1 == 0 &&rd != 0){
      tile_k = KMAX;
      tile_n = NMAX;
      tile_m = MMAX;
    }
    return (0xFF & tile_m) | (8 << (0xFF & tile_n)) | (16 << (0xFF & tile_k));
  } else {
    return 0;
  }

}

reg_t matrixUnit_t::set_moutsh(int rd, int rs1, int rs2){
  outshape[0] = extract64(rs1, 0,  16);
  outshape[1] = extract64(rs1, 16, 16);

  mstr_w = extract64(rs2, 0,  8);
  mstr_h = extract64(rs2, 8,  8);
  mdil_w = extract64(rs2, 16, 8);
  mdil_h = extract64(rs2, 24, 8);

  return outshape[0] | (outshape[1] << 16);
}

reg_t matrixUnit_t::set_insh(int rd, int rs1, int rs2) {
  inshape[0] = extract64(rs1, 0,  16);
  inshape[1] = extract64(rs1, 16, 16);

  mpad_right  = extract64(rs2, 0,  8);
  mpad_left   = extract64(rs2, 8,  8);
  mpad_bottom = extract64(rs2, 16, 8);
  mpad_top    = extract64(rs2, 24, 8);

  return inshape[0] | (inshape[1] << 16);
}

reg_t matrixUnit_t::set_msk(int rd, int rs1, int rs2) {
  mskin[0] = short(extract64(rs1, 0,  16));
  mskin[1] = short(extract64(rs1, 16, 16));

  mskout[0] = extract64(rs2, 0,  16);
  mskout[1] = extract64(rs2, 16, 16);

  return mskin[0] | (mskin[1] << 16);
}

reg_t matrixUnit_t::set_pad(int rd, int rs1) {
  mpadv[0] = short(extract64(rs1, 0,  16));
  mpadv[1] = short(extract64(rs1, 16, 16));

  return mpadv[0] | (mpadv[1] << 16);
}

