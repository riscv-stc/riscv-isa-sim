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
      void *acc_file;
      void *tr_renamefile;
      void *acc_renamefile;
      char tr_renamefile2[2048];
      reg_t MLEN, mlenb, RLEN, mamul;
      reg_t msew, mba;
      reg_t mfp16, mfp32, mfp64, mfp8, mint4, mint8, mint16, mint32, mint64;
      reg_t mrows, mcols;
      reg_t mrlenb;
      reg_t mlmul;
      reg_t mlmax;
      // csr_t_p mxsat;
      matrix_csr_t_p mtype, mstart, mcsr, tile_m, tile_k, tile_n, mtsp, mdsp;
      // im2col register

      matrix_csr_t_p moutshape, minshape, mstdi, mpad, minsk, moutsk, mpadval;
      reg_t outshape[2];
      reg_t inshape[2];
      reg_t mpad_top, mpad_bottom, mpad_left, mpad_right;
      reg_t mdil_h, mdil_w, mstr_h, mstr_w;
      sreg_t mskin[2];
      reg_t mskout[2];
      bool mill;
      /* matrix element for varies eew
        td: tile reg num
        tt: read row 6 or col 7
        slice: slice
        n: index in slice
        is_write: write or read
      */
      template<class T>
        T& tr_elt(reg_t td, reg_t tt, reg_t slice, reg_t n, reg_t rows, reg_t elts_per_slice, bool reg_rename = false, bool is_write = false) {
          assert(msew != 0);
          assert((mcols >> 3)/sizeof(T) > 0);
#ifdef RISCV_ENABLE_COMMITLOG
          if (is_write)
            p->get_state()->log_reg_write[((td) << 4) | 3] = {0, 0};
#endif
          T *regStart = reg_rename ? ((T*)tr_renamefile) + td * elts_per_slice * rows:((T*)tr_file) + td * elts_per_slice * rows ;
          if (tt & 1) { // col
            reg_t new_slice = slice > (elts_per_slice-1)? (slice % elts_per_slice): slice;
            return regStart[elts_per_slice * n + new_slice];
          } else { //row
            reg_t new_slice = slice > (rows-1)? (slice % rows): slice;
            return regStart[elts_per_slice * new_slice + n];
          }
        }

        char* board_elt(reg_t td, reg_t rows, reg_t elts_per_slice, bool is_acc = false, bool is_write = false) {
          assert(msew != 0);

#ifdef RISCV_ENABLE_COMMITLOG
          if (is_write)
            p->get_state()->log_reg_write[((td) << 4) | 4] = {0, 0};
#endif
          char *regStart = NULL;
          if (is_acc)
            regStart = ((char*)acc_file) + td * elts_per_slice * mamul * rows * msew / 8;
          else
            regStart = ((char*)tr_file) + td * elts_per_slice * rows * msew / 8;
          return regStart;
        }

        void reg_rename_write_back_elt(reg_t td, reg_t rows, reg_t elts_per_slice, reg_t lmul, reg_t reg_sum, bool is_acc = false) {
          assert(msew != 0);
          reg_t reg_byte_len = elts_per_slice * rows;

          char *regReNameStart = NULL;
          char *regStart = NULL;
          if (!is_acc){
            regReNameStart = ((char*)tr_renamefile) + td * reg_byte_len;
            regStart = ((char*)tr_file) + td * reg_byte_len;
          }
          else{
            regReNameStart = ((char*)acc_renamefile) + td * reg_byte_len;
            regStart = ((char*)acc_file) + td * reg_byte_len;
          }

          memcpy(regStart, regReNameStart, reg_byte_len * reg_sum * lmul);
        }

        void reg_copy_whole(reg_t td, reg_t ts1, bool is_acc = false) {
          assert(msew != 0);
          reg_t reg_byte_len = 0;

          char *regSrcStart = NULL;
          char *regDstStart = NULL;
          if (!is_acc){
            reg_byte_len = mrows * mcols / 8;
            regSrcStart = ((char*)tr_file) + ts1 * reg_byte_len;
            regDstStart = ((char*)tr_file) + td * reg_byte_len;
          }
          else{
            reg_byte_len = mrows * mcols / 8 * mamul;
            regSrcStart = ((char*)acc_file) + ts1 * reg_byte_len;
            regDstStart = ((char*)acc_file) + td * reg_byte_len;
          }

          memcpy(regDstStart, regSrcStart, reg_byte_len);
        }

        template<class T>
        T& acc_elt(reg_t td, reg_t tt, reg_t slice, reg_t n, reg_t rows, reg_t elts_per_slice, bool reg_rename = false, bool is_write = false) {
          assert(msew != 0);
          assert((mcols >> 3)/sizeof(T) > 0);

#ifdef RISCV_ENABLE_COMMITLOG
          if (is_write)
            p->get_state()->log_reg_write[((td) << 4) | 4] = {0, 0};
#endif
          T *regStart = reg_rename ? ((T*)acc_renamefile) +  td * rows * elts_per_slice : ((T *)acc_file) + td * rows * elts_per_slice;
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
        tr_renamefile = 0;
      }
      ~matrixUnit_t(){
        free(tr_file);
        free(tr_renamefile);
        tr_file = 0;
        tr_renamefile = 0;
      }

      reg_t set_mtype(int rd, reg_t newType);
      reg_t set_mtypei(int rd, reg_t newType);
      reg_t set_mtypehi(int rd, reg_t newType);
      reg_t set_msew(int rd, reg_t newType);
      reg_t set_mint(int rd, reg_t newType, reg_t bit);
      reg_t set_fp(int rd, reg_t newType, reg_t bit);
      reg_t set_ba(int rd, reg_t newType);
      reg_t set_ml(int rd, int rs1, reg_t newMlen, char dim);
      reg_t set_moutsh(int rd, int rs1, int rs2);
      reg_t set_insh(int rd, int rs1, int rs2);
      reg_t set_msk(int rd, int rs1, int rs2);
      reg_t set_pad(int rd, int rs1);
      reg_t get_mlen() {return MLEN;}
      reg_t set_tsp(int rs1);
      reg_t set_dsp(int rs1);
      
      // matrix 0.5 del mxrm
      // MRM get_mround_mode() {
      //   return (MRM)(mxrm->read());
      // }
  };

#include <iostream>
#include <cstdint>
#include <type_traits>

template <typename T>
struct bit4_t {
    static_assert(std::is_integral<T>::value, "T must be an integral type");
    T value : 4;  // 使用位域表示 4 位整数
    bool saturation_enabled;  // 指示是否启用饱和精度

    bit4_t(bool saturation = false) : value(0), saturation_enabled(saturation) {}

    bit4_t(T v, bool saturation = false) : saturation_enabled(saturation) {
        value = v & 0xF;       // 截断到 4 位
        if (std::is_signed<T>::value && (value & 0x8)) {  // 如果符号位为 1，扩展符号（仅对有符号类型）
            value |= 0xF0;
        }
    }

    bit4_t& operator=(T v) {
        value = v & 0xF;
        if (std::is_signed<T>::value && (value & 0x8)) {
            value |= 0xF0;
        }
        return *this;
    }

    // 转换为 T
    T toValue() const {
        return value;
    }

    // 用于处理饱和精度的辅助函数
    T saturate(T result) const {
        if (saturation_enabled) {
            if (std::is_signed<T>::value) {
                if (result > 7) return 7;     // int4_t 最大值
                if (result < -8) return -8;   // int4_t 最小值
            } else {
                if (result > 15) return 15;   // uint4_t 最大值
                if (result < 0) return 0;     // uint4_t 最小值
            }
        }
        return result;
    }

    // 运算符重载：加法
    bit4_t operator+(const bit4_t& other) const {
        T result = toValue() + other.toValue();
        return bit4_t(saturate(result), saturation_enabled);
    }

    // 运算符重载：减法
    bit4_t operator-(const bit4_t& other) const {
        T result = toValue() - other.toValue();
        return bit4_t(saturate(result), saturation_enabled);
    }

    // 运算符重载：乘法
    bit4_t operator*(const bit4_t& other) const {
        T result = toValue() * other.toValue();
        return bit4_t(saturate(result), saturation_enabled);
    }

    // 运算符重载：除法
    bit4_t operator/(const bit4_t& other) const {
        if (other.toValue() == 0) {
            throw std::overflow_error("Division by zero");  // 处理除以零的情况
        }
        T result = toValue() / other.toValue();
        return bit4_t(saturate(result), saturation_enabled);
    }

    // 运算符重载：取模
    bit4_t operator%(const bit4_t& other) const {
        if (other.toValue() == 0) {
            throw std::overflow_error("Modulo by zero");  // 处理取模零的情况
        }
        T result = toValue() % other.toValue();
        return bit4_t(saturate(result), saturation_enabled);
    }

    // 自增运算符
    bit4_t& operator++() {
        *this = *this + bit4_t(1, saturation_enabled);
        return *this;
    }

    // 自减运算符
    bit4_t& operator--() {
        *this = *this - bit4_t(1, saturation_enabled);
        return *this;
    }

    // 取反运算符
    bit4_t operator-() const {
        return bit4_t(saturate(-toValue()), saturation_enabled);
    }

    // 重载输出运算符，便于打印 bit4_t 值
    friend std::ostream& operator<<(std::ostream& os, const bit4_t& b) {
        os << +b.toValue(); // +号用于确保打印的是整数而不是字符
        return os;
    }
};

// 处理两个 4 位数值
template <typename T>
struct bit4_pair_t {
    bit4_t<T> high;  // 高 4 位
    bit4_t<T> low;   // 低 4 位

    bit4_pair_t() : high(0, false), low(0, false) {}

    // 构造函数处理 int8_t/uint8_t 类型的拆分
    bit4_pair_t(T v, bool saturation = false) 
        : high((v >> 4) & 0xF, saturation), low(v & 0xF, saturation) {}

    // 支持赋值操作
    bit4_pair_t& operator=(T v) {
        high = (v >> 4) & 0xF;
        low = v & 0xF;
        return *this;
    }

    // 重载运算符：加法
    bit4_pair_t operator+(const bit4_pair_t& other) const {
        return bit4_pair_t((high + other.high).toValue() << 4 | (low + other.low).toValue(),
                           high.saturation_enabled);
    }

    // 重载运算符：减法
    bit4_pair_t operator-(const bit4_pair_t& other) const {
        return bit4_pair_t((high - other.high).toValue() << 4 | (low - other.low).toValue(),
                           high.saturation_enabled);
    }

    // 重载运算符：乘法
    bit4_pair_t operator*(const bit4_pair_t& other) const {
        return bit4_pair_t((high * other.high).toValue() << 4 | (low * other.low).toValue(),
                           high.saturation_enabled);
    }

    // 重载运算符：除法
    bit4_pair_t operator/(const bit4_pair_t& other) const {
        return bit4_pair_t((high / other.high).toValue() << 4 | (low / other.low).toValue(),
                           high.saturation_enabled);
    }

    // 重载运算符：取模
    bit4_pair_t operator%(const bit4_pair_t& other) const {
        return bit4_pair_t((high % other.high).toValue() << 4 | (low % other.low).toValue(),
                           high.saturation_enabled);
    }

    // 类型转换操作符：转换为 int8_t 或 uint8_t
    operator T() const {
        return (high.toValue() << 4) | (low.toValue());
    }

    // 比较运算符重载
    bool operator==(const bit4_pair_t& other) const {
        return (high.toValue() == other.high.toValue()) && (low.toValue() == other.low.toValue());
    }

    bool operator!=(const bit4_pair_t& other) const {
        return !(*this == other);
    }

    bool operator<(const bit4_pair_t& other) const {
        return (high.toValue() < other.high.toValue()) ||
               (high.toValue() == other.high.toValue() && low.toValue() < other.low.toValue());
    }

    bool operator<=(const bit4_pair_t& other) const {
        return (*this < other) || (*this == other);
    }

    bool operator>(const bit4_pair_t& other) const {
        return !(*this <= other);
    }

    bool operator>=(const bit4_pair_t& other) const {
        return !(*this < other);
    }

    // 逻辑左移：根据other的low和high值分别移动this的low和high值
    bit4_pair_t operator<<(const bit4_pair_t& other) const {
        T new_high = (high.toValue() << other.high.toValue()) & 0xF;
        T new_low = (low.toValue() << other.low.toValue()) & 0xF;
        return bit4_pair_t((new_high << 4) | new_low, high.saturation_enabled);
    }

    // 逻辑右移：根据other的low和high值分别移动this的low和high值
    bit4_pair_t operator>>(const bit4_pair_t& other) const {
        T new_high = (high.toValue() >> other.high.toValue()) & 0xF;
        T new_low = (low.toValue() >> other.low.toValue()) & 0xF;
        return bit4_pair_t((new_high << 4) | new_low, high.saturation_enabled);
    }

    // 算术右移：根据other的low和high值分别移动this的low和high值
    bit4_pair_t arithmetic_right_shift(bit4_pair_t& other) const {
        T new_high = high.toValue();
        T new_low = low.toValue();

        if (std::is_signed<T>::value) {
            new_high = (new_high >> other.high.toValue()) | 
                       ((new_high & 0x8) ? ~(0xF >> other.high.toValue()) : 0);
            new_low = (new_low >> other.low.toValue()) | 
                      ((new_low & 0x8) ? ~(0xF >> other.low.toValue()) : 0);
        } else {
            new_high >>= other.high.toValue();
            new_low >>= other.low.toValue();
        }

        return bit4_pair_t((new_high << 4) | new_low, high.saturation_enabled);
    }

    bit4_pair_t arithmetic_right_shift(uint8_t shift_value) const {
        T new_high = high.toValue();
        T new_low = low.toValue();
        bit4_pair_t<uint8_t> other = shift_value;
        if (std::is_signed<T>::value) {
            new_high = (new_high >> other.high.toValue()) | 
                       ((new_high & 0x8) ? ~(0xF >> other.high.toValue()) : 0);
            new_low = (new_low >> other.low.toValue()) | 
                      ((new_low & 0x8) ? ~(0xF >> other.low.toValue()) : 0);
        } else {
            new_high >>= other.high.toValue();
            new_low >>= other.low.toValue();
        }

        return bit4_pair_t((new_high << 4) | new_low, high.saturation_enabled);
    }

    // 输出两个 4 位数值
    void print() const {
        std::cout << "High 4 bits: " << high << ", Low 4 bits: " << low << std::endl;
    }
};

union uint4_bit_pair {
    struct {
        uint8_t low  : 4;
        uint8_t high : 4;
    };
    uint8_t value;

    // Constructor to initialize directly from uint8_t
    uint4_bit_pair(uint8_t val) : value(val) {}
    
    uint4_bit_pair() : value(0) {}

    uint4_bit_pair& operator=(uint8_t val) {
        value = val;
        return *this;
    }

    operator uint8_t() const {
        return value;
    }
};

union int4_bit_pair {
    struct {
        int8_t low  : 4;
        int8_t high : 4;
    };
    int8_t value;

    // Constructor to initialize directly from uint8_t
    int4_bit_pair(int8_t val) : value(val) {}

    int4_bit_pair& operator=(int8_t val) {
        value = val;
        return *this;
    }

    operator int8_t() const {
        return value;
    }
};

#endif // _RISCV_MATRIX_UNIT_H