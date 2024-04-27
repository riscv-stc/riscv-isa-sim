#ifndef _RISCV_MATRIX_UNIT_H
#define _RISCV_MATRIX_UNIT_H

#include "vector_unit.h"

enum MRM{
  MRNU = 0,
  MRNE,
  MRDN,
  MROD,
  MINVALID_RM
};

class matrixUnit_t {
    public:
      processor_t* p;
      void *tr_file;

      reg_t MLEN, mlenb, RLEN;
      reg_t msew, mba;
      reg_t mbf16, mtf32, mfp64, mfp8, mint4;
      reg_t mrows, mcols;
      reg_t mrlenb;
      reg_t mlmul;
      reg_t mlmax;
      csr_t_p mxsat;
      matrix_csr_t_p mtype, mstart, mcsr, mxrm, tile_m, tile_k, tile_n;
      // im2col register
      reg_t moutsh;
      reg_t minsh;
      reg_t mpad, mstdi;
      reg_t msk;
      reg_t outshape[2];
      reg_t inshape[2];
      reg_t mpad_top, mpad_bottom, mpad_left, mpad_right;
      reg_t mdil_h, mdil_w, mstr_h, mstr_w;
      sreg_t mskin[2];
      reg_t mskout[2];
      reg_t mpadv[2];
      bool mill;
      /* matrix element for varies eew
        td: tile reg num
        tt: read row 6 or col 7
        slice: slice
        n: index in slice
        is_write: write or read
      */
      template<class T>
        T& tr_elt(reg_t td, reg_t tt, reg_t slice, reg_t n, reg_t rows, reg_t elts_per_slice, bool is_write = false) {
          assert(msew != 0);
          assert((mcols >> 3)/sizeof(T) > 0);
#ifdef RISCV_ENABLE_COMMITLOG
          if (is_write)
            p->get_state()->log_reg_write[((td) << 4) | 3] = {0, 0};
#endif
          T *regStart = ((T*)tr_file) + td * elts_per_slice * rows;
          if (tt & 1) { // col
            reg_t new_slice = slice > (elts_per_slice-1)? (slice % elts_per_slice): slice;
            return regStart[elts_per_slice * n + new_slice];
          } else { //row
            reg_t new_slice = slice > (rows-1)? (slice % rows): slice;
            return regStart[elts_per_slice * new_slice + n];
          }
        }

        char* board_elt(reg_t td, reg_t rows, reg_t elts_per_slice, bool is_write = false) {
          assert(msew != 0);

#ifdef RISCV_ENABLE_COMMITLOG
          if (is_write)
            p->get_state()->log_reg_write[((td) << 4) | 4] = {0, 0};
#endif
          char *regStart = ((char*)tr_file) + td * elts_per_slice * rows;
          return regStart;
        }

    public:

      void reset();

      matrixUnit_t() {
        tr_file = 0;
      }
      ~matrixUnit_t(){
        free(tr_file);
        tr_file = 0;
      }

      reg_t set_mtype(int rd, reg_t newType);
      reg_t set_ml(int rd, int rs1, reg_t newMlen, char dim);
      reg_t set_moutsh(int rd, int rs1, int rs2);
      reg_t set_insh(int rd, int rs1, int rs2);
      reg_t set_msk(int rd, int rs1, int rs2);
      reg_t set_pad(int rd, int rs1);
      reg_t get_mlen() {return MLEN;}
      MRM get_mround_mode() {
        return (MRM)(mxrm->read());
    }
  };
#endif // _RISCV_MATRIX_UNIT_H