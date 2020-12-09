/*
 * Copyright (c) 2020 StreamComputing Corp.
 * All rights reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Authors: Hao.Chen
 */


#ifndef EIGEN_BFLOAT16_STC_H
#define EIGEN_BFLOAT16_STC_H

#include "../../../../../../softfloat/softfloat.h"

#if __cplusplus > 199711L
#define EIGEN_EXPLICIT_CAST(tgt_type) explicit operator tgt_type()
#else
#define EIGEN_EXPLICIT_CAST(tgt_type) operator tgt_type()
#endif

namespace Eigen {
struct Bfloat16;

namespace bfloat16_impl {

struct __bfloat16_raw {
  EIGEN_DEVICE_FUNC __bfloat16_raw() : x(0) {}
  explicit EIGEN_DEVICE_FUNC __bfloat16_raw(uint16_t raw) : x(raw) {}
  uint16_t x;
};

EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC __bfloat16_raw raw_uint16_to_bfloat16(uint16_t x);
EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC __bfloat16_raw float_to_bfloat16_rtne(float ff);
EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC float bfloat16_to_float(__bfloat16_raw h);

struct bfloat16_base : public __bfloat16_raw {
  EIGEN_DEVICE_FUNC bfloat16_base() {}
  EIGEN_DEVICE_FUNC bfloat16_base(const bfloat16_base& h) : __bfloat16_raw(h) {}
  EIGEN_DEVICE_FUNC bfloat16_base(const __bfloat16_raw& h) : __bfloat16_raw(h) {}
};

} // namespace bfloat16_impl

// Class definition.
struct Bfloat16 : public bfloat16_impl::bfloat16_base {
  typedef bfloat16_impl::__bfloat16_raw __bfloat16_raw;

  EIGEN_DEVICE_FUNC Bfloat16() {}

  EIGEN_DEVICE_FUNC Bfloat16(const __bfloat16_raw& h) : bfloat16_impl::bfloat16_base(h) {}
  EIGEN_DEVICE_FUNC Bfloat16(const Bfloat16& h) : bfloat16_impl::bfloat16_base(h) {}

  explicit EIGEN_DEVICE_FUNC Bfloat16(bool b)
      : bfloat16_impl::bfloat16_base(bfloat16_impl::raw_uint16_to_bfloat16(b ? 0x3f80 : 0)) {}
  template<class T>
  explicit EIGEN_DEVICE_FUNC Bfloat16(const T& val)
      : bfloat16_impl::bfloat16_base(bfloat16_impl::float_to_bfloat16_rtne(static_cast<float>(val))) {}
  explicit EIGEN_DEVICE_FUNC Bfloat16(float f)
      : bfloat16_impl::bfloat16_base(bfloat16_impl::float_to_bfloat16_rtne(f)) {}
  explicit EIGEN_DEVICE_FUNC Bfloat16(float32_t f) {
      x = f32_to_bf16(f).v;
  }

  EIGEN_DEVICE_FUNC EIGEN_EXPLICIT_CAST(bool) const {
    // +0.0 and -0.0 become false, everything else becomes true.
    return (x & 0x7fff) != 0;
  }
  EIGEN_DEVICE_FUNC EIGEN_EXPLICIT_CAST(signed char) const {
    return static_cast<signed char>(bfloat16_impl::bfloat16_to_float(*this));
  }
  EIGEN_DEVICE_FUNC EIGEN_EXPLICIT_CAST(unsigned char) const {
    return static_cast<unsigned char>(bfloat16_impl::bfloat16_to_float(*this));
  }
  EIGEN_DEVICE_FUNC EIGEN_EXPLICIT_CAST(short) const {
    return static_cast<short>(bfloat16_impl::bfloat16_to_float(*this));
  }
  EIGEN_DEVICE_FUNC EIGEN_EXPLICIT_CAST(unsigned short) const {
    return static_cast<unsigned short>(bfloat16_impl::bfloat16_to_float(*this));
  }
  EIGEN_DEVICE_FUNC EIGEN_EXPLICIT_CAST(int) const {
    return static_cast<int>(bfloat16_impl::bfloat16_to_float(*this));
  }
  EIGEN_DEVICE_FUNC EIGEN_EXPLICIT_CAST(unsigned int) const {
    return static_cast<unsigned int>(bfloat16_impl::bfloat16_to_float(*this));
  }
  EIGEN_DEVICE_FUNC EIGEN_EXPLICIT_CAST(long) const {
    return static_cast<long>(bfloat16_impl::bfloat16_to_float(*this));
  }
  EIGEN_DEVICE_FUNC EIGEN_EXPLICIT_CAST(unsigned long) const {
    return static_cast<unsigned long>(bfloat16_impl::bfloat16_to_float(*this));
  }
  EIGEN_DEVICE_FUNC EIGEN_EXPLICIT_CAST(long long) const {
    return static_cast<long long>(bfloat16_impl::bfloat16_to_float(*this));
  }
  EIGEN_DEVICE_FUNC EIGEN_EXPLICIT_CAST(unsigned long long) const {
    return static_cast<unsigned long long>(bfloat16_to_float(*this));
  }
  EIGEN_DEVICE_FUNC EIGEN_EXPLICIT_CAST(float) const {
    return bfloat16_impl::bfloat16_to_float(*this);
  }
  EIGEN_DEVICE_FUNC EIGEN_EXPLICIT_CAST(double) const {
    return static_cast<double>(bfloat16_impl::bfloat16_to_float(*this));
  }

  EIGEN_DEVICE_FUNC Bfloat16& operator=(const Bfloat16& other) {
    x = other.x;
    return *this;
  }
};

} // end namespace Eigen

namespace Eigen {

namespace bfloat16_impl {

EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC bool (isinf)(const Bfloat16& a) {
  return (a.x & 0x7fff) == 0x7f80;
}
EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC bool (ispinf)(const Bfloat16& a) {
  return (a.x & 0xffff) == 0x7f80;
}
EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC bool (isninf)(const Bfloat16& a) {
  return (a.x & 0xffff) == 0xff80;
}
EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC bool (isPositive)(const Bfloat16& a) {
  return (a.x & 0x8000) == 0x0;
}
EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC bool (iszero)(const Bfloat16& a) {
  return (a.x & 0x7fff) == 0x0;
}
EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC bool (isnan)(const Bfloat16& a) {
  return (a.x & 0x7fff) > 0x7f80;
}

EIGEN_DEVICE_FUNC static EIGEN_STRONG_INLINE Bfloat16 highest() {
  return bfloat16_impl::raw_uint16_to_bfloat16(0x7f7f);
}
EIGEN_DEVICE_FUNC static EIGEN_STRONG_INLINE Bfloat16 lowest() {
  return bfloat16_impl::raw_uint16_to_bfloat16(0xff7f);
}
EIGEN_DEVICE_FUNC static EIGEN_STRONG_INLINE Bfloat16 pinf() {
  return bfloat16_impl::raw_uint16_to_bfloat16(0x7f80);
}
EIGEN_DEVICE_FUNC static EIGEN_STRONG_INLINE Bfloat16 ninf() {
  return bfloat16_impl::raw_uint16_to_bfloat16(0xff80);
}
EIGEN_DEVICE_FUNC static EIGEN_STRONG_INLINE Bfloat16 quiet_NaN() {
  return bfloat16_impl::raw_uint16_to_bfloat16(0x7f81);
}
EIGEN_DEVICE_FUNC static EIGEN_STRONG_INLINE Bfloat16 NaN() {
  return bfloat16_impl::raw_uint16_to_bfloat16(0x7fc0);
}

EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC bool (isfinite)(const Bfloat16& a) {
  return !(isinf EIGEN_NOT_A_MACRO (a)) && !(isnan EIGEN_NOT_A_MACRO (a));
}

EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC Bfloat16 abs(const Bfloat16& a) {
  Bfloat16 result;
  result.x = a.x & 0x7FFF;
  return result;
}

EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC bfloat16_t bfloat16_to_bfloat16_t(__bfloat16_raw x) {
  bfloat16_t bf16;
  bf16.v = x.x;
  return bf16;
}

EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC __bfloat16_raw bfloat16_t_to_bfloat16(bfloat16_t bf16) {
  __bfloat16_raw h;
  h.x = bf16.v;
  return h;
}

EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC Bfloat16 operator + (const Bfloat16& a, const Bfloat16& b) {
  return bfloat16_t_to_bfloat16(bf16_add(bfloat16_to_bfloat16_t(a), bfloat16_to_bfloat16_t(b)));
}
EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC Bfloat16 operator * (const Bfloat16& a, const Bfloat16& b) {
  return bfloat16_t_to_bfloat16(bf16_mul(bfloat16_to_bfloat16_t(a), bfloat16_to_bfloat16_t(b)));
}

EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC Bfloat16 operator * (const Bfloat16&  a, int b) {
  float32_t f32 = i32_to_f32(b);
  bfloat16_t bf16 = f32_to_bf16(f32);

  return bfloat16_t_to_bfloat16(bf16_mul(bfloat16_to_bfloat16_t(a), bf16));
}

EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC Bfloat16 operator - (const Bfloat16& a, const Bfloat16& b) {
  return bfloat16_t_to_bfloat16(bf16_sub(bfloat16_to_bfloat16_t(a), bfloat16_to_bfloat16_t(b)));
}

EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC Bfloat16 operator / (const Bfloat16& a, const Bfloat16& b) {
  return Bfloat16(float(a) / float(b));
}

EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC Bfloat16 operator - (const Bfloat16& a) {
  Bfloat16 result;
  result.x = a.x ^ 0x8000;
  return result;
}

EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC Bfloat16& operator += (Bfloat16& a, const Bfloat16& b) {
  a = a + b;
  return a;
}

EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC Bfloat16& operator *= (Bfloat16& a, const Bfloat16& b) {
  a = a * b;
  return a;
}

EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC Bfloat16& operator *= (Bfloat16& a, int b) {
  a = a * b;
  return a;
}

EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC Bfloat16& operator -= (Bfloat16& a, const Bfloat16& b) {
  a = a - b;
  return a;
}
EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC Bfloat16& operator /= (Bfloat16& a, const Bfloat16& b) {
  a = a / b;
  return a;
}
EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC bool operator == (const Bfloat16& a, const Bfloat16& b) {
  if( isnan(a)||isnan(b))
    return false;
  if((ispinf(a) && ispinf(b)) || (isninf(a) && isninf(b)))
    return true;
  if((isinf(a)||isinf(b)))
    return false;
  return (a.x == b.x);
}
EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC bool operator != (const Bfloat16& a, const Bfloat16& b) {
  if( isnan(a) ||  isnan(b))
    return true;
  return (a.x != b.x);
}
EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC bool operator < (const Bfloat16& a, const Bfloat16& b) {
  if( isnan(a) ||  isnan(b))
    return false;
  if((ispinf(a) && ispinf(b)) || (isninf(a) && isninf(b)))
    return false;
  if( ispinf(a) || isninf(b))
    return false;
  if( isninf(a) || ispinf(b))
    return true;
  return float(a) < float(b);
}
EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC bool operator <= (const Bfloat16& a, const Bfloat16& b) {
  return Eigen::bfloat16_impl::operator<(a,b) || Eigen::bfloat16_impl::operator==(a,b);
}
EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC bool operator > (const Bfloat16& a, const Bfloat16& b) {
  if( isnan(a) ||  isnan(b))
    return false;
  if((ispinf(a) && ispinf(b)) || (isninf(a) && isninf(b)))
    return false;
  if( ispinf(a) || isninf(b))
    return true;
  if( isninf(a) || ispinf(b))
    return false;
  return float(a) > float(b);
}
EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC bool operator >= (const Bfloat16& a, const Bfloat16& b) {
  return Eigen::bfloat16_impl::operator>(a,b) || Eigen::bfloat16_impl::operator==(a,b);
}

EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC Bfloat16 operator / (const Bfloat16& a, Index b) {
  return Bfloat16(static_cast<float>(a) / static_cast<float>(b));
}

EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC __bfloat16_raw raw_uint16_to_bfloat16(uint16_t x) {
  __bfloat16_raw h;
  h.x = x;
  return h;
}

union float32_bits {
  uint32_t u;
  float f;
};


EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC __bfloat16_raw float_to_bfloat16_rtne(float ff) {
  float32_bits f;
  float32_t f32;
  __bfloat16_raw bf16_bits;

  f.f = ff;
  f32.v = f.u;

  bfloat16_t bf16 = f32_to_bf16(f32);
  bf16_bits.x = bf16.v;

  return bf16_bits;
}

EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC float bfloat16_to_float(__bfloat16_raw h) {
  bfloat16_t bf16;
  bf16.v = h.x;
  float32_bits f32_bits;

  float32_t f32 = bf16_to_f32(bf16);
  f32_bits.u = f32.v;
  return f32_bits.f;
}

EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC Bfloat16 exp(const Bfloat16& a) {
   return Bfloat16(::expf(float(a)));
}
EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC Bfloat16 log(const Bfloat16& a) {
  return Bfloat16(::logf(float(a)));
}
EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC Bfloat16 log1p(const Bfloat16& a) {
  return Bfloat16(numext::log1p(float(a)));
}
EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC Bfloat16 log10(const Bfloat16& a) {
  return Bfloat16(::log10f(float(a)));
}
EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC Bfloat16 sqrt(const Bfloat16& a) {
    return Bfloat16(::sqrtf(float(a)));
}
EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC Bfloat16 pow(const Bfloat16& a, const Bfloat16& b) {
  return Bfloat16(::powf(float(a), float(b)));
}
EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC Bfloat16 sin(const Bfloat16& a) {
  return Bfloat16(::sinf(float(a)));
}
EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC Bfloat16 cos(const Bfloat16& a) {
  return Bfloat16(::cosf(float(a)));
}
EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC Bfloat16 tan(const Bfloat16& a) {
  return Bfloat16(::tanf(float(a)));
}
EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC Bfloat16 tanh(const Bfloat16& a) {
  return Bfloat16(::tanhf(float(a)));
}
EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC Bfloat16 floor(const Bfloat16& a) {
  return Bfloat16(::floorf(float(a)));
}
EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC Bfloat16 ceil(const Bfloat16& a) {
  return Bfloat16(::ceilf(float(a)));
}

EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC Bfloat16 (min)(const Bfloat16& a, const Bfloat16& b) {
  if ( isnan(a) ||  isnan(b))
      return Eigen::bfloat16_impl::NaN();
  if (((a.x == 0x8000) && (b.x == 0x0)) || ((b.x == 0x8000) && (a.x == 0x0)))
      return Eigen::bfloat16_impl::raw_uint16_to_bfloat16(0x8000);
  const float f1 = static_cast<float>(a);
  const float f2 = static_cast<float>(b);
  return f2 < f1 ? b : a;
}
EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC Bfloat16 (max)(const Bfloat16& a, const Bfloat16& b) {
  if ( isnan(a) ||  isnan(b)) 
      return Eigen::bfloat16_impl::NaN();
  if (((a.x==0x8000) && (b.x==0x0)) || ((b.x==0x8000) && (a.x==0x0)))
      return Eigen::bfloat16_impl::raw_uint16_to_bfloat16(0x0);
  const float f1 = static_cast<float>(a);
  const float f2 = static_cast<float>(b);
  return f1 < f2 ? b : a;
}

EIGEN_ALWAYS_INLINE std::ostream& operator << (std::ostream& os, const Bfloat16& v) {
  os << static_cast<float>(v) << "(0x" << std::hex << v.x << ")";
  return os;
}

} // end namespace bfloat16_impl

// import Eigen::bfloat16_impl::Bfloat16 into Eigen namespace
// using bfloat16_impl::Bfloat16;

namespace internal {

template<>
struct random_default_impl<Bfloat16, false, false>
{
  static inline Bfloat16 run(const Bfloat16& x, const Bfloat16& y)
  {
    return x + (y-x) * Bfloat16(float(std::rand()) / float(RAND_MAX));
  }
  static inline Bfloat16 run()
  {
    return run(Bfloat16(-1.f), Bfloat16(1.f));
  }
};

template<> struct is_arithmetic<Bfloat16> { enum { value = true }; };

} // end namespace internal

} // end namespace Eigen

namespace std {

#if __cplusplus > 199711L
template <>
struct hash<Eigen::Bfloat16> {
  EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE std::size_t operator()(const Eigen::Bfloat16& a) const {
    return static_cast<std::size_t>(a.x);
  }
};
#endif

} // end namespace std

#endif // EIGEN_HALF_CUDA_H
