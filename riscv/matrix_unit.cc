#include "config.h"
#include "matrix_unit.h"
#include "processor.h"
#include "arith.h"

void matrixUnit_t::reset(){
  free(tr_file);
  free(acc_file);
  tr_file = malloc(mlenb * 8);
  memset(tr_file, 0, mlenb * 8);
  
  acc_file = malloc(mlenb * 4 * 16);
  memset(acc_file, 0, mlenb * 4 * 16);

  auto& csrmap = p->get_state()->csrmap;
  csrmap[CSR_TILE_M] = std::make_shared<matrix_csr_t>(p, CSR_TILE_M, 0);
  csrmap[CSR_TILE_K] = std::make_shared<matrix_csr_t>(p, CSR_TILE_K, /*mask*/ 0);
  csrmap[CSR_TILE_N] = std::make_shared<matrix_csr_t>(p, CSR_TILE_N, /*mask*/ 0);
  csrmap[CSR_MLENB] = std::make_shared<matrix_csr_t>(p, CSR_MLENB, /*mask*/ mlenb);
  csrmap[CSR_MTYPE] = mtype = std::make_shared<matrix_csr_t>(p, CSR_MTYPE, /*mask*/ 0);

  mtype->write_raw(0);
  set_mtype(0, -1);
}

reg_t matrixUnit_t::set_mtype(int rd, reg_t newType) {
  
  if (mtype->read() != newType){
    msew = 1 << (extract64(newType, 0, 3) + 3);
    maccq = extract64(newType, 3, 1);
    mbf16 = extract64(newType, 4, 1);
    mtf32 = extract64(newType, 5, 1);
    mtype->write_raw(newType);
  }
  return mtype->read();
}


reg_t matrixUnit_t::set_ml(int rd, int rs1, reg_t newMlen, char dim) {
  reg_t MMAX = mrows;
  reg_t NMAX = mcols / msew;
  reg_t KMAX = std::min(MMAX, NMAX);

  if (dim == 'm' || dim == 'M') {
    if (rs1 != 0 && rd != 0) {
      tile_m = newMlen > MMAX? MMAX : newMlen;
    } else if (rs1 == 0 && rd != 0) {
      tile_m = MMAX;
    }
    return tile_m;
  } else if (dim == 'k' || dim == 'K') {
    if (rs1 != 0 && rd != 0) {
      tile_k = newMlen > KMAX? KMAX : newMlen;
    } else if (rs1 == 0 && rd != 0) {
      tile_k = KMAX;
    }
    return tile_k;
  } else if (dim == 'n' || dim == 'N') {
    if (rs1 != 0 && rd != 0) {
      tile_n = newMlen > NMAX? NMAX : newMlen;
    } else if (rs1 == 0 && rd != 0) {
      tile_n = NMAX;
    }
    return tile_n;
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

