#ifndef _RISCV_MATRIX_UNIT_H
#define _RISCV_MATRIX_UNIT_H

#include "vector_unit.h"

class matrixUnit_t {
    public:
      processor_t* p;
      void *tr_file;
      void *acc_file;

      reg_t tile_m, tile_k, tile_n;
      reg_t MLEN, mlenb;
      reg_t mill, maccq;
      reg_t msew;
      reg_t mbf16, mtf32;
      reg_t mrows, mcols;
      
      matrix_csr_t_p mtype;
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

      /* matrix element for varies eew
        td: tile reg num
        tt: read row 6 or col 7
        slice: slice
        n: index in slice
        is_write: write or read
      */
      template<class T>
        T& tr_elt(reg_t td, reg_t tt, reg_t slice, reg_t n, bool is_write = false) {
          assert(msew != 0);
          assert((mcols >> 3)/sizeof(T) > 0);
          reg_t elts_per_slice = (mcols>> 3) / (sizeof(T));
#ifdef RISCV_ENABLE_COMMITLOG
          if (is_write)
            p->get_state()->log_reg_write[((td) << 4) | 3] = {0, 0};
#endif
          T *regStart = ((T*)tr_file) + td * elts_per_slice * mrows;
          if (tt & 1) { // col
            reg_t new_slice = slice > (elts_per_slice-1)? (slice % elts_per_slice): slice;
            return regStart[elts_per_slice * n + new_slice];
          } else { //row
            reg_t new_slice = slice > (mrows-1)? (slice % mrows): slice;
            return regStart[elts_per_slice * new_slice + n];
          }
        }

      template<class T>
        T& acc_elt(reg_t td, reg_t tt, reg_t slice, reg_t n, bool is_write = false) {
          assert(msew != 0);
          assert((mcols >> 3)/sizeof(T) > 0);
          reg_t elts_per_slice;
          if (maccq) {
            elts_per_slice = (mcols * 4 >> 3) / (sizeof(T));
          } else {
            elts_per_slice = (mcols * 2 >> 3) / (sizeof(T));
          }
#ifdef RISCV_ENABLE_COMMITLOG
          if (is_write)
            p->get_state()->log_reg_write[((td) << 4) | 4] = {0, 0};
#endif
          T *regStart = (T *)((char*)acc_file + td * mlenb * 4);
          if (tt & 1) { // col
            // reg_t new_slice = slice > (elts_per_slice-1)? (slice % elts_per_slice): slice;
            return *(regStart + elts_per_slice * n + slice);
          } else { //row
            // reg_t new_slice = slice > (mrows-1)? (slice % mrows): slice;
            return *(regStart + elts_per_slice * slice + n);
          }
        }

    public:

      void reset();

      matrixUnit_t() {
        tr_file = 0;
        acc_file = 0;
      }
      ~matrixUnit_t(){
        free(tr_file);
        free(acc_file);
        tr_file = 0;
        acc_file = 0;
      }

      reg_t set_mtype(int rd, reg_t newType);
      reg_t set_ml(int rd, int rs1, reg_t newMlen, char dim);
      reg_t set_moutsh(int rd, int rs1, int rs2);
      reg_t set_insh(int rd, int rs1, int rs2);
      reg_t set_msk(int rd, int rs1, int rs2);

  };
#endif // _RISCV_MATRIX_UNIT_H