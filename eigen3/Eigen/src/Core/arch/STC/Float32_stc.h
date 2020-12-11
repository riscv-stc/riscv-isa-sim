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


#ifndef EIGEN_FLOAT32_STC_H
#define EIGEN_FLOAT32_STC_H

#include "../../../../../../softfloat/softfloat.h"

#if __cplusplus > 199711L
#define EIGEN_EXPLICIT_CAST(tgt_type) explicit operator tgt_type()
#else
#define EIGEN_EXPLICIT_CAST(tgt_type) operator tgt_type()
#endif

namespace Eigen {
struct Float32;
struct half;

namespace float32_impl {

struct __float32_raw {
  EIGEN_DEVICE_FUNC __float32_raw() : x(0) {}
  explicit EIGEN_DEVICE_FUNC __float32_raw(uint32_t raw) : x(raw) {}
  uint32_t x;
};

EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC __float32_raw raw_uint32_to_float32(uint32_t x);
EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC __float32_raw float_to_float32_rtne(float ff);
EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC float float32_to_float(__float32_raw h);

struct float32_base : public __float32_raw {
  EIGEN_DEVICE_FUNC float32_base() {}
  EIGEN_DEVICE_FUNC float32_base(const float32_base& h) : __float32_raw(h) {}
  EIGEN_DEVICE_FUNC float32_base(const __float32_raw& h) : __float32_raw(h) {}
};

} // namespace float32_impl

// Class definition.
struct Float32 : public float32_impl::float32_base {
  typedef float32_impl::__float32_raw __float32_raw;

  EIGEN_DEVICE_FUNC Float32() {}

  EIGEN_DEVICE_FUNC Float32(const __float32_raw& h) : float32_impl::float32_base(h) {}
  EIGEN_DEVICE_FUNC Float32(const Float32& h) : float32_impl::float32_base(h) {}

  explicit EIGEN_DEVICE_FUNC Float32(bool b)
      : float32_impl::float32_base(float32_impl::raw_uint32_to_float32(b ? 0x3f800000 : 0)) {}
  template<class T>
  explicit EIGEN_DEVICE_FUNC Float32(const T& val)
      : float32_impl::float32_base(float32_impl::float_to_float32_rtne(static_cast<float>(val))) {}
  explicit EIGEN_DEVICE_FUNC Float32(float f)
      : float32_impl::float32_base(float32_impl::float_to_float32_rtne(f)) {}
  explicit EIGEN_DEVICE_FUNC Float32(float32_t f) {
      x = f.v;
  }
  explicit EIGEN_DEVICE_FUNC Float32(const half& f);

  EIGEN_DEVICE_FUNC EIGEN_EXPLICIT_CAST(bool) const {
    // +0.0 and -0.0 become false, everything else becomes true.
    return (x & 0x7fffffff) != 0;
  }
  EIGEN_DEVICE_FUNC EIGEN_EXPLICIT_CAST(signed char) const {
    return static_cast<signed char>(float32_impl::float32_to_float(*this));
  }
  EIGEN_DEVICE_FUNC EIGEN_EXPLICIT_CAST(unsigned char) const {
    return static_cast<unsigned char>(float32_impl::float32_to_float(*this));
  }
  EIGEN_DEVICE_FUNC EIGEN_EXPLICIT_CAST(short) const {
    return static_cast<short>(float32_impl::float32_to_float(*this));
  }
  EIGEN_DEVICE_FUNC EIGEN_EXPLICIT_CAST(unsigned short) const {
    return static_cast<unsigned short>(float32_impl::float32_to_float(*this));
  }
  EIGEN_DEVICE_FUNC EIGEN_EXPLICIT_CAST(int) const {
    return static_cast<int>(float32_impl::float32_to_float(*this));
  }
  EIGEN_DEVICE_FUNC EIGEN_EXPLICIT_CAST(unsigned int) const {
    return static_cast<unsigned int>(float32_impl::float32_to_float(*this));
  }
  EIGEN_DEVICE_FUNC EIGEN_EXPLICIT_CAST(long) const {
    return static_cast<long>(float32_impl::float32_to_float(*this));
  }
  EIGEN_DEVICE_FUNC EIGEN_EXPLICIT_CAST(unsigned long) const {
    return static_cast<unsigned long>(float32_impl::float32_to_float(*this));
  }
  EIGEN_DEVICE_FUNC EIGEN_EXPLICIT_CAST(long long) const {
    return static_cast<long long>(float32_impl::float32_to_float(*this));
  }
  EIGEN_DEVICE_FUNC EIGEN_EXPLICIT_CAST(unsigned long long) const {
    return static_cast<unsigned long long>(float32_to_float(*this));
  }
  EIGEN_DEVICE_FUNC EIGEN_EXPLICIT_CAST(float) const {
    return float32_impl::float32_to_float(*this);
  }
  EIGEN_DEVICE_FUNC EIGEN_EXPLICIT_CAST(double) const {
    return static_cast<double>(float32_impl::float32_to_float(*this));
  }
  EIGEN_DEVICE_FUNC EIGEN_EXPLICIT_CAST(half) const;

  EIGEN_DEVICE_FUNC Float32& operator=(const Float32& other) {
    x = other.x;
    return *this;
  }
};

} // end namespace Eigen

namespace Eigen {

namespace float32_impl {

EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC bool (isinf)(const Float32& a) {
  return (a.x & 0x7fffffff) == 0x7f800000;
}
EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC bool (ispinf)(const Float32& a) {
  return (a.x & 0xffffffff) == 0x7f800000;
}
EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC bool (isninf)(const Float32& a) {
  return (a.x & 0xffffffff) == 0xff800000;
}
EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC bool (isPositive)(const Float32& a) {
  return (a.x & 0x80000000) == 0x0;
}
EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC bool (iszero)(const Float32& a) {
  return (a.x & 0x7fffffff) == 0x0;
}
EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC bool (isnan)(const Float32& a) {
  return (a.x & 0x7fffffff) > 0x7f800000;
}

EIGEN_DEVICE_FUNC static EIGEN_STRONG_INLINE Float32 highest() {
  return float32_impl::raw_uint32_to_float32(0x7f7f0000);
}
EIGEN_DEVICE_FUNC static EIGEN_STRONG_INLINE Float32 lowest() {
  return float32_impl::raw_uint32_to_float32(0xff7f0000);
}
EIGEN_DEVICE_FUNC static EIGEN_STRONG_INLINE Float32 pinf() {
  return float32_impl::raw_uint32_to_float32(0x7f800000);
}
EIGEN_DEVICE_FUNC static EIGEN_STRONG_INLINE Float32 ninf() {
  return float32_impl::raw_uint32_to_float32(0xff800000);
}
EIGEN_DEVICE_FUNC static EIGEN_STRONG_INLINE Float32 quiet_NaN() {
  return float32_impl::raw_uint32_to_float32(0x7f800001);
}
EIGEN_DEVICE_FUNC static EIGEN_STRONG_INLINE Float32 NaN() {
  return float32_impl::raw_uint32_to_float32(0x7fc00000);
}

EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC bool (isfinite)(const Float32& a) {
  return !(isinf EIGEN_NOT_A_MACRO (a)) && !(isnan EIGEN_NOT_A_MACRO (a));
}

EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC Float32 abs(const Float32& a) {
  Float32 result;
  result.x = a.x & 0x7FFFFFFF;
  return result;
}

EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC float32_t float32_to_float32_t(__float32_raw x) {
  float32_t bf32;
  bf32.v = x.x;
  return bf32;
}

EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC __float32_raw float32_t_to_float32(float32_t f32) {
  __float32_raw h;
  h.x = f32.v;
  return h;
}

EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC Float32 operator + (const Float32& a, const Float32& b) {
  return float32_t_to_float32(f32_add(float32_to_float32_t(a), float32_to_float32_t(b)));
}
EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC Float32 operator * (const Float32& a, const Float32& b) {
  return float32_t_to_float32(f32_mul(float32_to_float32_t(a), float32_to_float32_t(b)));
}

EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC Float32 operator * (const Float32&  a, int b) {
  float32_t f32 = i32_to_f32(b);

  return float32_t_to_float32(f32_mul(float32_to_float32_t(a), f32));
}

EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC Float32 operator - (const Float32& a, const Float32& b) {
  return float32_t_to_float32(f32_sub(float32_to_float32_t(a), float32_to_float32_t(b)));
}

EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC Float32 operator / (const Float32& a, const Float32& b) {
  return float32_t_to_float32(f32_div(float32_to_float32_t(a), float32_to_float32_t(b)));
}

EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC Float32 operator - (const Float32& a) {
  Float32 result;
  result.x = a.x ^ 0x80000000;
  return result;
}

EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC Float32& operator += (Float32& a, const Float32& b) {
  a = a + b;
  return a;
}

EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC Float32& operator *= (Float32& a, const Float32& b) {
  a = a * b;
  return a;
}

EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC Float32& operator *= (Float32& a, int b) {
  a = a * b;
  return a;
}

EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC Float32& operator -= (Float32& a, const Float32& b) {
  a = a - b;
  return a;
}
EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC Float32& operator /= (Float32& a, const Float32& b) {
  a = a / b;
  return a;
}
EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC bool operator == (const Float32& a, const Float32& b) {
  if( isnan(a)||isnan(b))
    return false;
  if((ispinf(a) && ispinf(b)) || (isninf(a) && isninf(b)))
    return true;
  if((isinf(a)||isinf(b)))
    return false;
  return (a.x == b.x);
}
EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC bool operator != (const Float32& a, const Float32& b) {
  if( isnan(a) ||  isnan(b))
    return true;
  return (a.x != b.x);
}
EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC bool operator < (const Float32& a, const Float32& b) {
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
EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC bool operator <= (const Float32& a, const Float32& b) {
  return Eigen::float32_impl::operator<(a,b) || Eigen::float32_impl::operator==(a,b);
}
EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC bool operator > (const Float32& a, const Float32& b) {
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
EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC bool operator >= (const Float32& a, const Float32& b) {
  return Eigen::float32_impl::operator>(a,b) || Eigen::float32_impl::operator==(a,b);
}

EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC Float32 operator / (const Float32& a, Index b) {
  return Float32(static_cast<float>(a) / static_cast<float>(b));
}

EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC __float32_raw raw_uint32_to_float32(uint32_t x) {
  __float32_raw h;
  h.x = x;
  return h;
}

union float32_bits {
  uint32_t u;
  float f;
};


EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC __float32_raw float_to_float32_rtne(float ff) {
  float32_bits f;
  __float32_raw row;
  f.f = ff;
  row.x = f.u;
  return row;
}

EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC float float32_to_float(__float32_raw h) {
  float32_bits f32_bits;
  f32_bits.u = h.x;
  return f32_bits.f;
}

EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC Float32 exp(const Float32& a) {
  return float32_t_to_float32(f32_exp(float32_to_float32_t(a)));
}
EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC Float32 log(const Float32& a) {
  return Float32(::logf(float(a)));
}
EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC Float32 log1p(const Float32& a) {
  return Float32(numext::log1p(float(a)));
}
EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC Float32 log10(const Float32& a) {
  return Float32(::log10f(float(a)));
}
EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC Float32 sqrt(const Float32& a) {
    return Float32(::sqrtf(float(a)));
}
EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC Float32 pow(const Float32& a, const Float32& b) {
  return Float32(::powf(float(a), float(b)));
}
EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC Float32 sin(const Float32& a) {
  return Float32(::sinf(float(a)));
}
EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC Float32 cos(const Float32& a) {
  return Float32(::cosf(float(a)));
}
EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC Float32 tan(const Float32& a) {
  return Float32(::tanf(float(a)));
}
EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC Float32 tanh(const Float32& a) {
  return Float32(::tanhf(float(a)));
}
EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC Float32 floor(const Float32& a) {
  return Float32(::floorf(float(a)));
}
EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC Float32 ceil(const Float32& a) {
  return Float32(::ceilf(float(a)));
}

EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC Float32 (min)(const Float32& a, const Float32& b) {
  if ( isnan(a) ||  isnan(b))
      return Eigen::float32_impl::NaN();
  if (((a.x == 0x80000000) && (b.x == 0x0)) || ((b.x == 0x80000000) && (a.x == 0x0)))
      return Eigen::float32_impl::raw_uint32_to_float32(0x80000000);
  const float f1 = static_cast<float>(a);
  const float f2 = static_cast<float>(b);
  return f2 < f1 ? b : a;
}
EIGEN_STRONG_INLINE EIGEN_DEVICE_FUNC Float32 (max)(const Float32& a, const Float32& b) {
  if ( isnan(a) ||  isnan(b)) 
      return Eigen::float32_impl::NaN();
  if (((a.x==0x80000000) && (b.x==0x0)) || ((b.x==0x80000000) && (a.x==0x0)))
      return Eigen::float32_impl::raw_uint32_to_float32(0x0);
  const float f1 = static_cast<float>(a);
  const float f2 = static_cast<float>(b);
  return f1 < f2 ? b : a;
}

EIGEN_ALWAYS_INLINE std::ostream& operator << (std::ostream& os, const Float32& v) {
  os << static_cast<float>(v) << "(0x" << std::hex << v.x << ")";
  return os;
}

} // end namespace float32_impl

// import Eigen::float32_impl::Float32 into Eigen namespace
// using float32_impl::Float32;

namespace internal {

template<>
struct random_default_impl<Float32, false, false>
{
  static inline Float32 run(const Float32& x, const Float32& y)
  {
    return x + (y-x) * Float32(float(std::rand()) / float(RAND_MAX));
  }
  static inline Float32 run()
  {
    return run(Float32(-1.f), Float32(1.f));
  }
};

template<> struct is_arithmetic<Float32> { enum { value = true }; };

} // end namespace internal

} // end namespace Eigen

namespace std {

#if __cplusplus > 199711L
template <>
struct hash<Eigen::Float32> {
  EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE std::size_t operator()(const Eigen::Float32& a) const {
    return static_cast<std::size_t>(a.x);
  }
};
#endif

} // end namespace std

#endif // EIGEN_HALF_CUDA_H
