/**
 * @file eigen3_ops.cc
 * @brief The Source Code About Eigen3 To Spike Interface
 *
 * Class CustomInsns 包含了所有的custom定制指令，但不包含vector指令，
 * custom指令具有数据类型明确的特点，不需要模板类就能轻松实现，所以这些都放在
 * 一个类里面（实际上没有任何抽象的意义）。vector指令更多的没有指定被操作数的数据
 * 类型，所以为了使代码简洁，使用了大量的模板类，同时，因为无法统一每一个vector指令
 * 的模板参数，所以基本上一类vector指令的实现封装在一个类中（实际上也没有经过抽象，纯粹是
 * 按照不同的vector指令去区分该不该放在一个类里面）
 *
 * @author chenhao
 * 
 */

#include "eigen3_ops.h"
#include "fp16a.c"
#include "fp16m.c"
#include "fp16ext.c"
#include "int32xfp16.c"
#include "int32xbf16.c"
#include "fp16convfp32.c"

/* dynamic stride for map */
typedef Stride<Dynamic, Dynamic> DynStride;

/* a method to define Matrix and Map */
#define MY_MATRIX_DEFINE(Type)                                                \
typedef Matrix<Type, Dynamic, Dynamic, RowMajor> Matrix_##Type;               \
typedef Map<Matrix_##Type, Unaligned, Stride<Dynamic, Dynamic> > Map_##Type;

/* Matrix_half   Map_half */
MY_MATRIX_DEFINE(half)
MY_MATRIX_DEFINE(Bfloat16)
/* Matrix_uint8_t     Map_uint8_t */
MY_MATRIX_DEFINE(uint8_t)
MY_MATRIX_DEFINE(int8_t)
/* Matrix_uint16_t     Map_uint16_t */
MY_MATRIX_DEFINE(uint16_t)
MY_MATRIX_DEFINE(int16_t)

MY_MATRIX_DEFINE(uint32_t)
MY_MATRIX_DEFINE(int32_t)

MY_MATRIX_DEFINE(Float32)
MY_MATRIX_DEFINE(float32_t)

/*Matrix_float_t     Map_float_t */
MY_MATRIX_DEFINE(float)
#undef MY_MATRIX_DEFINE

#define MECONV_INFO(ss) do {\
        if (debug) {\
            cout << endl << __FUNCTION__ << endl;\
            meconv_dbg(ss);\
        }\
    } while(0)

namespace Eigen {
  EIGEN_DEVICE_FUNC half::half(const Float32& f) {
    float32_t f32t;
    f32t.v = f.x;
    this->x = f32_to_f16(f32t).v;
  }

  EIGEN_DEVICE_FUNC half::operator Float32() const {
      float16_t f16t;
      f16t.v = x;
      float32_t f32t = f16_to_f32(f16t);
      Float32 f32;
      f32.x = f32t.v;
    return f32;
  }

  EIGEN_DEVICE_FUNC Float32::Float32(const half& f) {
      float16_t f16t;
      f16t.v = f.x;
      float32_t f32t = f16_to_f32(f16t);
      this->x = f32t.v;
  }

  EIGEN_DEVICE_FUNC Float32::Float32(const Bfloat16& bf) {
      bfloat16_t bf16t;
      bf16t.v = bf.x;
      float32_t f32t = bf16_to_f32(bf16t);
      this->x = f32t.v;
  }

  EIGEN_DEVICE_FUNC Float32::operator half() const {
      float32_t f32t;
      f32t.v = x;
      float16_t f16t = f32_to_f16(f32t);
      half res;
      res.x = f16t.v;
      return res;
  }

  EIGEN_DEVICE_FUNC Float32::operator Bfloat16() const {
      float32_t f32t;
      f32t.v = x;
      bfloat16_t bf16t = f32_to_bf16(f32t);
      Bfloat16 res;
      res.x = bf16t.v;
      return res;
  }

  EIGEN_DEVICE_FUNC Float32 Float32::mulConvert(const half& a, const half& b) {
      float16_t f16a, f16b;
      f16a.v = a.x;
      f16b.v = b.x;
      float32_t f32t;
      f32t = f16_mul32(f16a, f16b);
      Float32 f32;
      f32.x = f32t.v;
      return f32;
  }

  EIGEN_DEVICE_FUNC Float32 Float32::mulConvert(const Bfloat16& a, const Bfloat16& b) {
      bfloat16_t bf16a, bf16b;
      bf16a.v = a.x;
      bf16b.v = b.x;
      float32_t res;
      res = bf16_mul32(bf16a, bf16b);
      Float32 f32;
      f32.x = res.v;
      return f32;
  }

  EIGEN_DEVICE_FUNC Bfloat16::Bfloat16(const Float32& f32) {
      float32_t f32t;
      f32t.v = f32.x;
      bfloat16_t bf16t = f32_to_bf16(f32t);
      x = bf16t.v;
  }

  EIGEN_DEVICE_FUNC Bfloat16::Bfloat16(const half& h) {
      float16_t f16t;
      f16t.v = h.x;
      bfloat16_t bf16t = f16_to_bf16(f16t);
      x = bf16t.v;
  }

  EIGEN_DEVICE_FUNC Bfloat16::operator half() const {
      bfloat16_t bf16t;
      bf16t.v = x;
      float16_t f16t = bf16_to_f16(bf16t);
      half res;
      res.x = f16t.v;
      return res;
  }

  EIGEN_DEVICE_FUNC Bfloat16::operator Float32() const {
      bfloat16_t bf16t;
      bf16t.v = x;
      float32_t f32 = bf16_to_f32(bf16t);
      Float32 res;
      res.x = f32.v;
      return res;
  }
}



/**
 * CustomInsns() 构造函数
 * 
 * 默认不开启debug
 */
CustomInsns::CustomInsns(): debug(GLOBAL_DBG)
{
}

float16_t
CustomInsns::half_to_float16_t(half x)
{
    float16_t f16;
    f16.v = x.x;
    return f16;
}

half
CustomInsns::float16_t_to_half(float16_t f16)
{
    half h;
    h.x = f16.v;
    return h;
}

half
CustomInsns::f32_to_half(float32_t f32)
{
    half h;
    float16_t f16 = f32_to_f16(f32);
    h = float16_t_to_half(f16);
    return h;
}

float32_t 
CustomInsns::half_to_f32(half x)
{
    float32_t f32;
    f32 = f16_to_f32(half_to_float16_t(x));
    return f32;
}

float32_t
CustomInsns::half_mul_f32(half a, half b)
{
    float16_t a_f16, b_f16;
    float32_t res_f32;
    a_f16 = half_to_float16_t(a);
    b_f16 = half_to_float16_t(b);
    res_f32 = f16_mul32(a_f16, b_f16);
    return res_f32;
}
half
CustomInsns::int32_mul_f16(int a, float16_t b)
{
    half res;
    res.x = int32xfp16(a, b.v) & 0xffff;
    return res;
}

Bfloat16
CustomInsns::int32_mul_bf16(int a, Bfloat16 b)
{
    Bfloat16 res;
    res.x = int32xbf16(a, b.x);
    return res;
}
/**
 * shapestride_dbg() 打印ShapeStride信息
 * 
 * 用于调试
 * @param ss shapeStride 参数指针
 * @return 无返回值
 */
void CustomInsns::meconv_dbg(struct ConvShapeStride *ss)
{
    if (!debug)
        return;

    printf("\nMeconv info:\n");
    printf("conv_fm_in(0x%x): w = %d h = %d\n", ss->conv_fm_in, (ss->conv_fm_in >> 16) & 0xffff, ss->conv_fm_in & 0xffff);
    printf("conv_cin(0x%x): scin = %d depth = %d\n", ss->conv_cin, (ss->conv_cin >> 16) & 0xffff, ss->conv_cin & 0xffff);
    printf("conv_fm_out(0x%x): w = %d h = %d\n", ss->conv_fm_out, (ss->conv_fm_out >> 16) & 0xffff, ss->conv_fm_out & 0xffff);
    printf("conv_cout(0x%x): scout = %d depth = %d\n", ss->conv_cout, (ss->conv_cout >> 16) & 0xffff, ss->conv_cout & 0xffff);
    printf("conv_s_kernel(0x%x): stride = %d\n", ss->conv_kernel_params2, ss->conv_kernel_params2 & 0xffff);
    printf("conv_kernel(0x%x): kw = %d kh = %d dilation = %d sk = %d\n", ss->conv_kernel_params1,(ss->conv_kernel_params1 >> 24) & 0xff,(ss->conv_kernel_params1 >> 16) & 0xff,(ss->conv_kernel_params1 >> 8) & 0xff, (ss->conv_kernel_params1 >> 0) & 0xff);
    printf("conv_padding(0x%x): top = %d bottom = %d left = %d right = %d\n", ss->conv_padding, (ss->conv_padding >> 24) & 0xff,(ss->conv_padding >> 16) & 0xff, (ss->conv_padding >> 8) & 0xff, (ss->conv_padding >> 0) & 0xff);
}


/**
 * meconv_mm() meconv.mm
 *
 * 标量和矩阵元素广播乘 M = M1 * f
 * @param rs1 M1,源操作矩阵基地址
 * @param rd M,目的矩阵基地址
 * @param rs2 f,源标量操作数
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
int CustomInsns::meconv_mm(half *rs1, half *rd, half *rs2, struct ConvShapeStride *ss)
{
    int pad_top, pad_bottom, pad_left, pad_right;
    int kw, kh, okw, okh, k_stride, sk_h, sk_w;
    int in_w, in_h, in_c, in_stride;
    int out_w, out_h, out_c, out_stride;
    int w, h, c;
    int dilation_h, dilation_w;
    int i, j, k, ii, jj, kk, index_cin, counter;
    int row, col;
    half *rs1_start;
    half *left_val, *row_val, *col_val;
    half *start;
    half val;

    //get the padding
    pad_top = (ss->conv_padding >> 24) & 0xff;
    pad_bottom = (ss->conv_padding >> 16) & 0xff;
    pad_left = (ss->conv_padding >> 8) & 0xff;
    pad_right = ss->conv_padding & 0xff;

    //get the input shape
    in_w = (ss->conv_fm_in >> 16) & 0xffff;
    in_h = (ss->conv_fm_in) & 0xffff;
    in_c = (ss->conv_cin) & 0xffff;
    assert(in_w > 0 && in_h > 0 && in_c > 0);
    in_stride = (ss->conv_cin >> 16) & 0xffff;
    //assert((in_stride % 2) == 0); //half
    in_stride = in_stride > 0 ? in_stride : in_c;

    //get the output shape
    out_w = (ss->conv_fm_out >> 16) & 0xffff;
    out_h = (ss->conv_fm_out) & 0xffff;
    out_c = (ss->conv_cout) & 0xffff;
    assert(out_w > 0 && out_h > 0 && out_c > 0);
    out_stride = (ss->conv_cout >> 16) & 0xffff;
    //assert(out_stride % 2 == 0);
    out_stride = out_stride > 0 ? out_stride : out_c;

    //get the kernel shape
    kw = (ss->conv_kernel_params1 >> 24) & 0xff;
    kh = (ss->conv_kernel_params1 >> 16) & 0xff;
    dilation_h = (ss->conv_kernel_params1 >> 8) & 0xff;
    dilation_w = (ss->conv_kernel_params2 >> 24) & 0xff;
    sk_h = (ss->conv_kernel_params1) & 0xff;
    sk_w = (ss->conv_kernel_params2 >> 16) & 0xff;
    dilation_h = dilation_h == 0? 1 : dilation_h;
    dilation_w = dilation_w == 0? dilation_h : dilation_w;
    sk_h = sk_h == 0? 1 : sk_h;
    sk_w = sk_w == 0? sk_h : sk_w;
    assert(kw > 0 && kh > 0);
    assert(dilation_h > 0 && dilation_w > 0 && sk_h >0 && sk_w > 0);
    k_stride = ss->conv_kernel_params2 & 0xffff;
    //assert(k_stride % 2 == 0);
    k_stride = k_stride > 0 ? k_stride : out_c;

    /*calculate the kernel shape*/
    Map_half rs2_matrix(rs2, kh * kw * in_c, out_c, DynStride(k_stride, 1)); // the depth is same as in_c

    h = dilation_h > 1 ? dilation_h * (kh - 1) + 1 : kh;
    w = dilation_w > 1 ? dilation_w * (kw - 1) + 1 : kw;
    okh = kh;
    okw = kw;
    kh = h;
    kw = w;

    /*calculate the input shape*/
    Map_half rs1_matrix(rs1, in_h * in_w, in_c, DynStride(in_stride, 1));

    if (debug) {
        MECONV_INFO(ss);
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rs2:" << endl << rs2_matrix << endl;
    }

    /*calculate the output shape*/
    h = (in_h + pad_top + pad_bottom - kh + 1 + sk_h - 1) / sk_h;
    w = (in_w + pad_left + pad_right - kw + 1 + sk_w - 1) / sk_w;
    left_val = (half *)malloc(h * w * okh * okw * in_c * sizeof(half));

    for (i = 0; i < h; i++) {
        for (j = 0; j < w; j++) {
            start = left_val + i * w * okh * okw * in_c + j * okh * okw * in_c;
            rs1_start = rs1;

            for (ii = 0; ii < kh; ii++) {
                for (jj = 0; jj < kw; jj++) {
                    for (kk = 0; kk < in_c; kk++) {
                        row = i * sk_h + ii;
                        col = j * sk_w + jj;

                        if (ii % dilation_h)
                            continue;
                        else {
                            if (jj % dilation_w)
                                continue;
                            else {
				int start_offset = ii/dilation_h * okw * in_c + jj/dilation_w * in_c + kk;
				int rs1_offset = (row - pad_top) * in_w * in_stride + (col - pad_left) * in_stride + kk;
                                if (row >= pad_top && row < pad_top + in_h && col >= pad_left && col < pad_left + in_w) {
                                    *(start + start_offset) = *(rs1_start + rs1_offset);
                                    val = *(rs1_start + rs1_offset);
                                }
                                else {
                                    *(start + start_offset) = (half)0;
                                    val =  (half)0;
                                }

                                if (debug) {
                                    printf("rd offset= %d val = 0x%x ",start_offset, val.x);
                                    val = *(start + start_offset);
                                    printf("rd val = 0x%x\n", val.x);
                                }
                            }//jj % dilation_w
                        }//ii % dilation_h
                    }//kk
                }//jj
            }//ii
        }//j
    }//i

    /*param check*/
    if (debug)
        printf("h = %d w = %d out_h = %d out_w = %d\n", h, w, out_h, out_w);
    assert(h == out_h && w == out_w);

    /*calculate convolution*/
    Map_half left_matrix(left_val, h * w, okh * okw * in_c, DynStride(okh * okw * in_c, 1));
    Map_half rd_matrix(rd, out_h * out_w, out_c, DynStride(out_stride, 1));
    row_val = (half *)malloc(okh * okw * in_c * sizeof(half));
    col_val = (half *)malloc(okh * okw * in_c * sizeof(half));
    Map_half row_matrix(row_val, 1, okh * okw * in_c, DynStride(okh * okw * in_c, 1));
    Map_half col_matrix(col_val, 1, okh * okw * in_c, DynStride(okh * okw * in_c, 1));
    float32_t first, second, third, forth, res12, res34, res;
    float32_t res_tmp;
    if (debug) {
        cout << "rs1: " << rs1_matrix << endl;
        cout << "left: " << left_matrix << endl;
        cout << "rs2: " << rs2_matrix << endl;
    }
    //rd_matrix = left_matrix * rs2_matrix;
    for (i = 0; i < out_h * out_w; i++) {
        for (j = 0; j < out_c; j++) {
            row_matrix = left_matrix.row(i);
            col_matrix = rs2_matrix.col(j).transpose();
            first.v = 0x80000000;
            second.v = 0x80000000;
            third.v = 0x80000000;
            forth.v = 0x80000000;
            res12.v = 0x80000000;
            res34.v = 0x80000000;
            res.v = 0x80000000;
            counter = 0;
            
            if ((k_stride == out_c ) && (out_c <= 32)){
                if ((in_stride == in_c) && (dilation_h == 1)){
                     for (k = 0; k < okh; k++){
                        int offset = k * okw * in_c; 
                        for (index_cin = 0; index_cin < (okw * in_c); index_cin++){
                            res_tmp = half_mul_f32(row_matrix(0, offset + index_cin), col_matrix(0, offset + index_cin));
                            if(counter%4 == 0) first = f32_add(res_tmp, first);
                            else if(counter%4 == 1) second = f32_add(res_tmp, second);
                            else if(counter%4 == 2) third = f32_add(res_tmp, third);
                            else if(counter%4 == 3) forth = f32_add(res_tmp, forth);
                            ++counter;
                            if (((in_c*okw) % 2 == 1) && (index_cin == (in_c * okw - 1))) ++counter;
                        }
                    }
                    res12 = f32_add(first, third);
                    res34 = f32_add(second,forth);
                    rd_matrix(i, j) = f32_to_half(f32_add(res12, res34));
                } else {
                    if (!(in_c%2)){//输出列宽小于32和stride行连续,并且cin为偶数
                        for (index_cin = 0; index_cin < (okh * okw * in_c); index_cin++){
                            res_tmp = half_mul_f32(row_matrix(0, index_cin), col_matrix(0, index_cin));
                            if(counter%4 == 0) first = f32_add(res_tmp, first);
                            else if(counter%4 == 1) second = f32_add(res_tmp, second);
                            else if(counter%4 == 2) third = f32_add(res_tmp, third);
                            else if(counter%4 == 3) forth = f32_add(res_tmp, forth);
                            ++counter;
                        }
                        res12 = f32_add(first, third);
                        res34 = f32_add(second,forth);
                        rd_matrix(i, j) = f32_to_half(f32_add(res12, res34));
                    } else {//输出列宽小于32和stride行连续,并且cin为奇数
                        for (k = 0; k < (okh * okw); k++){
                            int offset = k * in_c;
                            for (index_cin = 0; index_cin < in_c; index_cin++){
                                res_tmp = half_mul_f32(row_matrix(0, offset + index_cin),                      col_matrix(0, offset + index_cin));
                                if(counter%4 == 0) first = f32_add(res_tmp, first);
                                else if(counter%4 == 1) second = f32_add(res_tmp, second);
                                else if(counter%4 == 2) third = f32_add(res_tmp, third);
                                else if(counter%4 == 3) forth = f32_add(res_tmp, forth);
                                ++counter;
                                if (index_cin == (in_c - 1)) ++counter;
                            }
                        }
                        res12 = f32_add(first, third);
                        res34 = f32_add(second,forth);
                        rd_matrix(i, j) = f32_to_half(f32_add(res12, res34));
                    }
                }
            } else {
                for (k = 0; k < okh * okw * in_c; k++) {
                    if (! (k % 2))
                        first = f32_add(half_mul_f32(row_matrix(0, k), col_matrix(0, k)), first);
                    else
                        second = f32_add(half_mul_f32(row_matrix(0, k), col_matrix(0, k)), second);
                }
                rd_matrix(i, j) = f32_to_half(f32_add(first, second));
            }
        }
    }

    if (debug)
        cout << "rd:" << endl << rd_matrix << endl;

    free(row_val);
    free(col_val);
    free(left_val);
    return 0;
}

/**
 * meconv_mm() meconv.mm
 *
 * 标量和矩阵元素广播乘 M = M1 * f
 * @param rs1 M1,源操作矩阵基地址
 * @param rd M,目的矩阵基地址
 * @param rs2 f,源标量操作数
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
int CustomInsns::meconv_mm(half *rs1, half *rd, int8_t *rs2, struct ConvShapeStride *ss)
{
    int pad_top, pad_bottom, pad_left, pad_right;
    int kw, kh, okw, okh, k_stride, sk_h, sk_w;
    int in_w, in_h, in_c, in_stride;
    int out_w, out_h, out_c, out_stride;
    int w, h, c;
    int dilation_h, dilation_w;
    int i, j, k, ii, jj, kk;
    int row, col;
    half *rs1_start;
    half *left_val, *row_val;
    int8_t *col_val;
    half *start;
    half val;

    //get the padding
    pad_top = (ss->conv_padding >> 24) & 0xff;
    pad_bottom = (ss->conv_padding >> 16) & 0xff;
    pad_left = (ss->conv_padding >> 8) & 0xff;
    pad_right = ss->conv_padding & 0xff;

    //get the input shape
    in_w = (ss->conv_fm_in >> 16) & 0xffff;
    in_h = (ss->conv_fm_in) & 0xffff;
    in_c = (ss->conv_cin) & 0xffff;
    assert(in_w > 0 && in_h > 0 && in_c > 0);
    in_stride = (ss->conv_cin >> 16) & 0xffff;
    //assert((in_stride % 2) == 0); //half
    in_stride = in_stride > 0 ? in_stride  : in_c;

    //get the output shape
    out_w = (ss->conv_fm_out >> 16) & 0xffff;
    out_h = (ss->conv_fm_out) & 0xffff;
    out_c = (ss->conv_cout) & 0xffff;
    assert(out_w > 0 && out_h > 0 && out_c > 0);
    out_stride = (ss->conv_cout >> 16) & 0xffff;
    //assert(out_stride % 2 == 0);
    out_stride = out_stride > 0 ? out_stride : out_c;

    //get the kernel shape
    kw = (ss->conv_kernel_params1 >> 24) & 0xff;
    kh = (ss->conv_kernel_params1 >> 16) & 0xff;
    dilation_h = (ss->conv_kernel_params1 >> 8) & 0xff;
    dilation_w = (ss->conv_kernel_params2 >> 24) & 0xff;
    sk_h = (ss->conv_kernel_params1) & 0xff;
    sk_w = (ss->conv_kernel_params2 >> 16) & 0xff;
    dilation_h = dilation_h == 0? 1 : dilation_h;
    dilation_w = dilation_w == 0? dilation_h : dilation_w;
    sk_h = sk_h == 0? 1 : sk_h;
    sk_w = sk_w == 0? sk_h : sk_w;
    assert(kw > 0 && kh > 0);
    assert(dilation_h > 0 && dilation_w > 0 && sk_h >0 && sk_w > 0);
    k_stride = ss->conv_kernel_params2 & 0xffff;
    // assert(k_stride % 2 == 0);
    k_stride = k_stride > 0 ? k_stride : out_c;

    //get de/quant coeff
    float16_t quant_coeff = f32_to_f16(ss->mme_quant_coeff);
    float16_t dequant_coeff = f32_to_f16(ss->mme_dequant_coeff);

    /*calculate the kernel shape*/
    Map_int8_t rs2_matrix(rs2, kh * kw * in_c, out_c, DynStride(k_stride, 1)); // the depth is same as in_c

    h = dilation_h > 1 ? dilation_h * (kh - 1) + 1 : kh;
    w = dilation_w > 1 ? dilation_w * (kw - 1) + 1 : kw;
    okh = kh;
    okw = kw;
    kh = h;
    kw = w;

    /*calculate the input shape*/
    Map_half rs1_matrix(rs1, in_h * in_w, in_c, DynStride(in_stride, 1));

    if (debug) {
        MECONV_INFO(ss);
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rs2:" << endl << rs2_matrix << endl;
    }

    /*calculate the output shape*/
    h = (in_h + pad_top + pad_bottom - kh + 1 + sk_h - 1) / sk_h;
    w = (in_w + pad_left + pad_right - kw + 1 + sk_w - 1) / sk_w;
    left_val = (half *)malloc(h * w * okh * okw * in_c * sizeof(half));

    for (i = 0; i < h; i++) {
        for (j = 0; j < w; j++) {
            start = left_val + i * w * okh * okw * in_c + j * okh * okw * in_c;
            rs1_start = rs1;

            for (ii = 0; ii < kh; ii++) {
                for (jj = 0; jj < kw; jj++) {
                    for (kk = 0; kk < in_c; kk++) {
                        row = i * sk_h + ii;
                        col = j * sk_w + jj;

                        if (ii % dilation_h)
                            continue;
                        else {
                            if (jj % dilation_w)
                                continue;
                            else {
				int start_offset = ii/dilation_h * okw * in_c + jj/dilation_w * in_c + kk;
				int rs1_offset = (row - pad_top) * in_w * in_stride + (col - pad_left) * in_stride + kk;
                                if (row >= pad_top && row < pad_top + in_h && col >= pad_left && col < pad_left + in_w) {
                                    *(start + start_offset) = *(rs1_start + rs1_offset);
                                    val = *(rs1_start + rs1_offset);
                                }
                                else {
                                    *(start + start_offset) = (half)0;
                                    val =  (half)0;
                                }

                                if (debug) {
                                    printf("rd offset= %d val = 0x%x ",start_offset, val.x);
                                    val = *(start + start_offset);
                                    printf("rd val = 0x%x\n", val.x);
                                }
                            }//jj % dilation_w
                        }//ii % dilation_h
                    }//kk
                }//jj
            }//ii
        }//j
    }//i

    /*param check*/
    if (debug)
        printf("h = %d w = %d out_h = %d out_w = %d\n", h, w, out_h, out_w);
    assert(h == out_h && w == out_w);

    /*calculate convolution*/
    Map_half left_matrix(left_val, h * w, okh * okw * in_c, DynStride(okh * okw * in_c, 1));
    Map_half rd_matrix(rd, out_h * out_w, out_c, DynStride(out_stride, 1));
    row_val = (half *)malloc(okh * okw * in_c * sizeof(half));
    col_val = (int8_t *)malloc(okh * okw * in_c * sizeof(int8_t));
    Map_half row_matrix(row_val, 1, okh * okw * in_c, DynStride(okh * okw * in_c, 1));
    Map_int8_t col_matrix(col_val, 1, okh * okw * in_c, DynStride(okh * okw * in_c, 1));
    int32_t res;
    float16_t rs1_f16;
    int8_t rs1_i8;
    //rd_matrix = left_matrix * rs2_matrix;
    for (i = 0; i < out_h * out_w; i++) {
        for (j = 0; j < out_c; j++) {
            row_matrix = left_matrix.row(i);
            col_matrix = rs2_matrix.col(j).transpose();
            res = 0;
            for (k = 0; k < okh * okw * in_c; k++) {
                rs1_f16 = f16_mul(half_to_float16_t(row_matrix(0, k)), quant_coeff);
                rs1_i8 = f16_to_i8(rs1_f16, softfloat_round_near_maxMag, true);
                res += rs1_i8 * col_matrix(0, k);
            }
            rd_matrix(i, j) = int32_mul_f16(res, dequant_coeff); 
        }
    }

    if (debug)
        cout << "rd:" << endl << rd_matrix << endl;

    return 0;
}

/**
 * meconv_mm() meconv.mm
 *
 * 标量和矩阵元素广播乘 M = M1 * f
 * @param rs1 M1,源操作矩阵基地址
 * @param rd M,目的矩阵基地址
 * @param rs2 f,源标量操作数
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
int CustomInsns::meconv_mm(int8_t *rs1, half *rd, int8_t *rs2, struct ConvShapeStride *ss)
{
    int pad_top, pad_bottom, pad_left, pad_right;
    int kw, kh, okw, okh, k_stride, sk_h, sk_w;
    int in_w, in_h, in_c, in_stride;
    int out_w, out_h, out_c, out_stride;
    int w, h, c;
    int dilation_h, dilation_w;
    int i, j, k, ii, jj, kk;
    int row, col;
    int8_t *rs1_start;
    int8_t *left_val, *row_val;
    int8_t *col_val;
    int8_t *start;
    int32_t val;

    //get the padding
    pad_top = (ss->conv_padding >> 24) & 0xff;
    pad_bottom = (ss->conv_padding >> 16) & 0xff;
    pad_left = (ss->conv_padding >> 8) & 0xff;
    pad_right = ss->conv_padding & 0xff;

    //get the input shape
    in_w = (ss->conv_fm_in >> 16) & 0xffff;
    in_h = (ss->conv_fm_in) & 0xffff;
    in_c = (ss->conv_cin) & 0xffff;
    assert(in_w > 0 && in_h > 0 && in_c > 0);
    in_stride = (ss->conv_cin >> 16) & 0xffff;
    //assert((in_stride % 2) == 0); //half
    in_stride = in_stride > 0 ? in_stride  : in_c;

    //get the output shape
    out_w = (ss->conv_fm_out >> 16) & 0xffff;
    out_h = (ss->conv_fm_out) & 0xffff;
    out_c = (ss->conv_cout) & 0xffff;
    assert(out_w > 0 && out_h > 0 && out_c > 0);
    out_stride = (ss->conv_cout >> 16) & 0xffff;
    //assert(out_stride % 2 == 0);
    out_stride = out_stride > 0 ? out_stride : out_c;

    //get the kernel shape
    kw = (ss->conv_kernel_params1 >> 24) & 0xff;
    kh = (ss->conv_kernel_params1 >> 16) & 0xff;
    dilation_h = (ss->conv_kernel_params1 >> 8) & 0xff;
    dilation_w = (ss->conv_kernel_params2 >> 24) & 0xff;
    sk_h = (ss->conv_kernel_params1) & 0xff;
    sk_w = (ss->conv_kernel_params2 >> 16) & 0xff;
    dilation_h = dilation_h == 0? 1 : dilation_h;
    dilation_w = dilation_w == 0? dilation_h : dilation_w;
    sk_h = sk_h == 0? 1 : sk_h;
    sk_w = sk_w == 0? sk_h : sk_w;
    assert(kw > 0 && kh > 0);
    assert(dilation_h > 0 && dilation_w > 0 && sk_h >0 && sk_w > 0);
    k_stride = ss->conv_kernel_params2 & 0xffff;
    // assert(k_stride % 2 == 0);
    k_stride = k_stride > 0 ? k_stride : out_c;

    //get de/quant coeff
    float16_t dequant_coeff = f32_to_f16(ss->mme_dequant_coeff);

    /*calculate the kernel shape*/
    Map_int8_t rs2_matrix(rs2, kh * kw * in_c, out_c, DynStride(k_stride, 1)); // the depth is same as in_c

    h = dilation_h > 1 ? dilation_h * (kh - 1) + 1 : kh;
    w = dilation_w > 1 ? dilation_w * (kw - 1) + 1 : kw;
    okh = kh;
    okw = kw;
    kh = h;
    kw = w;

    /*calculate the input shape*/
    Map_int8_t rs1_matrix(rs1, in_h * in_w, in_c, DynStride(in_stride, 1));

    if (debug) {
        MECONV_INFO(ss);
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rs2:" << endl << rs2_matrix << endl;
    }

    /*calculate the output shape*/
    h = (in_h + pad_top + pad_bottom - kh + 1 + sk_h - 1) / sk_h;
    w = (in_w + pad_left + pad_right - kw + 1 + sk_w - 1) / sk_w;
    left_val = (int8_t *)malloc(h * w * okh * okw * in_c * sizeof(int8_t));

    for (i = 0; i < h; i++) {
        for (j = 0; j < w; j++) {
            start = left_val + i * w * okh * okw * in_c + j * okh * okw * in_c;
            rs1_start = rs1;

            for (ii = 0; ii < kh; ii++) {
                for (jj = 0; jj < kw; jj++) {
                    for (kk = 0; kk < in_c; kk++) {
                        row = i * sk_h + ii;
                        col = j * sk_w + jj;

                        if (ii % dilation_h)
                            continue;
                        else {
                            if (jj % dilation_w)
                                continue;
                            else {
				int start_offset = ii/dilation_h * okw * in_c + jj/dilation_w * in_c + kk;
				int rs1_offset = (row - pad_top) * in_w * in_stride + (col - pad_left) * in_stride + kk;
                                if (row >= pad_top && row < pad_top + in_h && col >= pad_left && col < pad_left + in_w) {
                                    *(start + start_offset) = *(rs1_start + rs1_offset);
                                    val = *(rs1_start + rs1_offset);
                                }
                                else {
                                    *(start + start_offset) = 0;
                                    val =  0;
                                }

                                if (debug) {
                                    printf("rd offset= %d val = 0x%x ",start_offset, val);
                                    val = *(start + start_offset);
                                    printf("rd val = 0x%x\n", val);
                                }
                            }//jj % dilation_w
                        }//ii % dilation_h
                    }//kk
                }//jj
            }//ii
        }//j
    }//i

    /*param check*/
    if (debug)
        printf("h = %d w = %d out_h = %d out_w = %d\n", h, w, out_h, out_w);
    assert(h == out_h && w == out_w);

    /*calculate convolution*/
    Map_int8_t left_matrix(left_val, h * w, okh * okw * in_c, DynStride(okh * okw * in_c, 1));
    Map_half rd_matrix(rd, out_h * out_w, out_c, DynStride(out_stride, 1));
    row_val = (int8_t *)malloc(okh * okw * in_c * sizeof(int8_t));
    col_val = (int8_t *)malloc(okh * okw * in_c * sizeof(int8_t));
    Map_int8_t row_matrix(row_val, 1, okh * okw * in_c, DynStride(okh * okw * in_c, 1));
    Map_int8_t col_matrix(col_val, 1, okh * okw * in_c, DynStride(okh * okw * in_c, 1));
    int32_t res;
    //rd_matrix = left_matrix * rs2_matrix;
    for (i = 0; i < out_h * out_w; i++) {
        for (j = 0; j < out_c; j++) {
            row_matrix = left_matrix.row(i);
            col_matrix = rs2_matrix.col(j).transpose();
            res = 0;
            for (k = 0; k < okh * okw * in_c; k++) {
                res += (int32_t)(row_matrix(0, k) * col_matrix(0, k));
            }
            rd_matrix(i, j) = int32_mul_f16(res, dequant_coeff); 
        }
    }

    if (debug)
        cout << "rd:" << endl << rd_matrix << endl;

    return 0;
}

/**
 * meconv_mm() meconv.mm
 *
 * 标量和矩阵元素广播乘 M = M1 * f
 * @param rs1 M1,源操作矩阵基地址
 * @param rd M,目的矩阵基地址
 * @param rs2 f,源标量操作数
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
int CustomInsns::meconv_mm(float32_t *rs1, float32_t *rd, float32_t *rs2, struct ConvShapeStride *ss)
{
    int pad_top, pad_bottom, pad_left, pad_right;
    int kw, kh, okw, okh, k_stride, sk_h, sk_w;
    int in_w, in_h, in_c, in_stride;
    int out_w, out_h, out_c, out_stride;
    int w, h, c;
    int dilation_h, dilation_w;
    int i, j, k, ii, jj, kk, index_cin, counter;
    int row, col;
    float32_t *rs1_start;
    float32_t *left_val, *row_val, *col_val;
    float32_t *start;
    float32_t val;

    //get the padding
    pad_top = (ss->conv_padding >> 24) & 0xff;
    pad_bottom = (ss->conv_padding >> 16) & 0xff;
    pad_left = (ss->conv_padding >> 8) & 0xff;
    pad_right = ss->conv_padding & 0xff;

    //get the input shape
    in_w = (ss->conv_fm_in >> 16) & 0xffff;
    in_h = (ss->conv_fm_in) & 0xffff;
    in_c = (ss->conv_cin) & 0xffff;
    assert(in_w > 0 && in_h > 0 && in_c > 0);
    in_stride = (ss->conv_cin >> 16) & 0xffff;
    //assert((in_stride % 2) == 0); //half
    in_stride = in_stride > 0 ? in_stride : in_c;

    //get the output shape
    out_w = (ss->conv_fm_out >> 16) & 0xffff;
    out_h = (ss->conv_fm_out) & 0xffff;
    out_c = (ss->conv_cout) & 0xffff;
    assert(out_w > 0 && out_h > 0 && out_c > 0);
    out_stride = (ss->conv_cout >> 16) & 0xffff;
    //assert(out_stride % 2 == 0);
    out_stride = out_stride > 0 ? out_stride : out_c;

    //get the kernel shape
    kw = (ss->conv_kernel_params1 >> 24) & 0xff;
    kh = (ss->conv_kernel_params1 >> 16) & 0xff;
    dilation_h = (ss->conv_kernel_params1 >> 8) & 0xff;
    dilation_w = (ss->conv_kernel_params2 >> 24) & 0xff;
    sk_h = (ss->conv_kernel_params1) & 0xff;
    sk_w = (ss->conv_kernel_params2 >> 16) & 0xff;
    dilation_h = dilation_h == 0? 1 : dilation_h;
    dilation_w = dilation_w == 0? dilation_h : dilation_w;
    sk_h = sk_h == 0? 1 : sk_h;
    sk_w = sk_w == 0? sk_h : sk_w;
    assert(kw > 0 && kh > 0);
    assert(dilation_h > 0 && dilation_w > 0 && sk_h >0 && sk_w > 0);
    k_stride = ss->conv_kernel_params2 & 0xffff;
    //assert(k_stride % 2 == 0);
    k_stride = k_stride > 0 ? k_stride : out_c;

    /*calculate the kernel shape*/
    Map_float32_t rs2_matrix(rs2, kh * kw * in_c, out_c, DynStride(k_stride, 1)); // the depth is same as in_c

    h = dilation_h > 1 ? dilation_h * (kh - 1) + 1 : kh;
    w = dilation_w > 1 ? dilation_w * (kw - 1) + 1 : kw;
    okh = kh;
    okw = kw;
    kh = h;
    kw = w;

    /*calculate the input shape*/
    Map_float32_t rs1_matrix(rs1, in_h * in_w, in_c, DynStride(in_stride, 1));

    // if (debug) {
    //     MECONV_INFO(ss);
    //     cout << "rs1:" << endl << rs1_matrix << endl;
    //     cout << "rs2:" << endl << rs2_matrix << endl;
    // }

    /*calculate the output shape*/
    h = (in_h + pad_top + pad_bottom - kh + 1 + sk_h - 1) / sk_h;
    w = (in_w + pad_left + pad_right - kw + 1 + sk_w - 1) / sk_w;
    left_val = (float32_t *)malloc(h * w * okh * okw * in_c * sizeof(float32_t));

    for (i = 0; i < h; i++) {
        for (j = 0; j < w; j++) {
            start = left_val + i * w * okh * okw * in_c + j * okh * okw * in_c;
            rs1_start = rs1;

            for (ii = 0; ii < kh; ii++) {
                for (jj = 0; jj < kw; jj++) {
                    for (kk = 0; kk < in_c; kk++) {
                        row = i * sk_h + ii;
                        col = j * sk_w + jj;

                        if (ii % dilation_h)
                            continue;
                        else {
                            if (jj % dilation_w)
                                continue;
                            else {
				int start_offset = ii/dilation_h * okw * in_c + jj/dilation_w * in_c + kk;
				int rs1_offset = (row - pad_top) * in_w * in_stride + (col - pad_left) * in_stride + kk;
                                if (row >= pad_top && row < pad_top + in_h && col >= pad_left && col < pad_left + in_w) {
                                    *(start + start_offset) = *(rs1_start + rs1_offset);
                                    val = *(rs1_start + rs1_offset);
                                }
                                else {
                                    *(start + start_offset) = i32_to_f32(0);
                                    val =  i32_to_f32(0);
                                }

                                if (debug) {
                                    printf("rd offset= %d val = 0x%x ",start_offset, val.v);
                                    val = *(start + start_offset);
                                    printf("rd val = 0x%x\n", val.v);
                                }
                            }//jj % dilation_w
                        }//ii % dilation_h
                    }//kk
                }//jj
            }//ii
        }//j
    }//i

    /*param check*/
    if (debug)
        printf("h = %d w = %d out_h = %d out_w = %d\n", h, w, out_h, out_w);
    assert(h == out_h && w == out_w);

    /*calculate convolution*/
    Map_float32_t left_matrix(left_val, h * w, okh * okw * in_c, DynStride(okh * okw * in_c, 1));
    Map_float32_t rd_matrix(rd, out_h * out_w, out_c, DynStride(out_stride, 1));
    row_val = (float32_t *)malloc(okh * okw * in_c * sizeof(float32_t));
    col_val = (float32_t *)malloc(okh * okw * in_c * sizeof(float32_t));
    Map_float32_t row_matrix(row_val, 1, okh * okw * in_c, DynStride(okh * okw * in_c, 1));
    Map_float32_t col_matrix(col_val, 1, okh * okw * in_c, DynStride(okh * okw * in_c, 1));
    float32_t first, second, third, forth, res12, res34, res;
    float32_t res_tmp;
    // if (debug) {
    //     cout << "rs1: " << rs1_matrix << endl;
    //     cout << "left: " << left_matrix << endl;
    //     cout << "rs2: " << rs2_matrix << endl;
    // }
    //rd_matrix = left_matrix * rs2_matrix;
    for (i = 0; i < out_h * out_w; i++) {
        for (j = 0; j < out_c; j++) {
            row_matrix = left_matrix.row(i);
            col_matrix = rs2_matrix.col(j).transpose();
            first.v = 0x80000000;
            second.v = 0x80000000;
            third.v = 0x80000000;
            forth.v = 0x80000000;
            res12.v = 0x80000000;
            res34.v = 0x80000000;
            res.v = 0x80000000;
            counter = 0;
            
            if ((k_stride == out_c ) && (out_c <= 32)){
                if ((in_stride == in_c) && (dilation_h == 1)){
                     for (k = 0; k < okh; k++){
                        int offset = k * okw * in_c; 
                        for (index_cin = 0; index_cin < (okw * in_c); index_cin++){
                            res_tmp = tf32_mul(row_matrix(0, offset + index_cin), col_matrix(0, offset + index_cin));
                            if(counter%4 == 0) first = f32_add(res_tmp, first);
                            else if(counter%4 == 1) second = f32_add(res_tmp, second);
                            else if(counter%4 == 2) third = f32_add(res_tmp, third);
                            else if(counter%4 == 3) forth = f32_add(res_tmp, forth);
                            ++counter;
                            if (((in_c*okw) % 2 == 1) && (index_cin == (in_c * okw - 1))) 
                                ++counter;
                        }
                    }
                    res12 = f32_add(first, third);
                    res34 = f32_add(second,forth);
                    rd_matrix(i, j) = f32_add(res12, res34);
                } else {
                    if (!(in_c%2)){//输出列宽小于32和stride行连续,并且cin为偶数
                        for (index_cin = 0; index_cin < (okh * okw * in_c); index_cin++){
                            res_tmp = tf32_mul(row_matrix(0, index_cin), col_matrix(0, index_cin));
                            if(counter%4 == 0) first = f32_add(res_tmp, first);
                            else if(counter%4 == 1) second = f32_add(res_tmp, second);
                            else if(counter%4 == 2) third = f32_add(res_tmp, third);
                            else if(counter%4 == 3) forth = f32_add(res_tmp, forth);
                            ++counter;
                        }
                        res12 = f32_add(first, third);
                        res34 = f32_add(second,forth);
                        rd_matrix(i, j) = f32_add(res12, res34);
                    } else {//输出列宽小于32和stride行连续,并且cin为奇数
                        for (k = 0; k < (okh * okw); k++){
                            int offset = k * in_c;
                            for (index_cin = 0; index_cin < in_c; index_cin++){
                                res_tmp = tf32_mul(row_matrix(0, offset + index_cin), 
                                                   col_matrix(0, offset + index_cin));
                                if(counter%4 == 0) first = f32_add(res_tmp, first);
                                else if(counter%4 == 1) second = f32_add(res_tmp, second);
                                else if(counter%4 == 2) third = f32_add(res_tmp, third);
                                else if(counter%4 == 3) forth = f32_add(res_tmp, forth);
                                ++counter;
                                if (index_cin == (in_c - 1)) ++counter;
                            }
                        }
                        res12 = f32_add(first, third);
                        res34 = f32_add(second,forth);
                        rd_matrix(i, j) = f32_add(res12, res34);
                    }
                }
            } else {
                for (k = 0; k < okh * okw * in_c; k++) {
                    if (! (k % 2))
                        first = f32_add(tf32_mul(row_matrix(0, k), col_matrix(0, k)), first);
                    else
                        second = f32_add(tf32_mul(row_matrix(0, k), col_matrix(0, k)), second);
                }
                rd_matrix(i, j) = f32_add(first, second);
            }
        if (debug)
            cout << "vd" << hex << rd_matrix(i, j).v << endl;
        }
    }

    // if (debug)
    //     cout << "rd:" << endl << rd_matrix << endl;

    free(row_val);
    free(col_val);
    free(left_val);
    return 0;
}

int CustomInsns::meconv_sp_mm(half *rs1, half *rs2, uint8_t *sparseidx, half *rd, struct ConvShapeStride *ss)
{
    int pad_top, pad_bottom, pad_left, pad_right;
    int kw, kh, okw, okh, k_stride, sk_h, sk_w, stride_idx;
    int in_w, in_h, in_c, in_stride;
    int out_w, out_h, out_c, out_stride;
    int w, h, c;
    int dilation_h, dilation_w;
    int i, j, k, ii, jj, kk, index_cin, counter;
    int row, col;
    uint8_t sp_index1, sp_index2;
    half *rs1_start;
    half *left_val, *row_val, *col_val;
    half *start;
    half val;
    uint8_t *sp_idx_data, *idx_val;

    //get the padding
    pad_top = (ss->conv_padding >> 24) & 0xff;
    pad_bottom = (ss->conv_padding >> 16) & 0xff;
    pad_left = (ss->conv_padding >> 8) & 0xff;
    pad_right = ss->conv_padding & 0xff;

    //get the input shape
    in_w = (ss->conv_fm_in >> 16) & 0xffff;
    in_h = (ss->conv_fm_in) & 0xffff;
    in_c = (ss->conv_cin) & 0xffff;
    assert(in_w > 0 && in_h > 0 && in_c > 0);
    in_stride = (ss->conv_cin >> 16) & 0xffff;
    // assert((in_stride % 2) == 0); //half
    in_stride = in_stride > 0 ? in_stride : in_c;

    //get the output shape
    out_w = (ss->conv_fm_out >> 16) & 0xffff;
    out_h = (ss->conv_fm_out) & 0xffff;
    out_c = (ss->conv_cout) & 0xffff;
    assert(out_w > 0 && out_h > 0 && out_c > 0);
    out_stride = (ss->conv_cout >> 16) & 0xffff;
    // assert(out_stride % 2 == 0);
    out_stride = out_stride > 0 ? out_stride : out_c;

    //get the index stride
    stride_idx = ss->stride_idx? ss->stride_idx : out_c;

    //get the kernel shape
    kw = (ss->conv_kernel_params1 >> 24) & 0xff;
    kh = (ss->conv_kernel_params1 >> 16) & 0xff;
    dilation_h = (ss->conv_kernel_params1 >> 8) & 0xff;
    dilation_w = (ss->conv_kernel_params2 >> 24) & 0xff;
    sk_h = (ss->conv_kernel_params1) & 0xff;
    sk_w = (ss->conv_kernel_params2 >> 16) & 0xff;
    dilation_h = dilation_h == 0? 1 : dilation_h;
    dilation_w = dilation_w == 0? dilation_h : dilation_w;
    sk_h = sk_h == 0? 1 : sk_h;
    sk_w = sk_w == 0? sk_h : sk_w;
    assert(kw > 0 && kh > 0);
    assert(dilation_h > 0 && dilation_w > 0 && sk_h >0 && sk_w > 0);
    k_stride = ss->conv_kernel_params2 & 0xffff;
    //assert(k_stride % 2 == 0);
    k_stride = k_stride > 0 ? k_stride : out_c;

    /* split the 8bit index shape into 2bit */
    i = (kw * kh * in_c / 2 * stride_idx + 3)/4;
    Map_uint8_t tmp_matrix(sparseidx, 1, i, DynStride(i, 1));
    sp_idx_data = (uint8_t *)malloc(kw * kh * (in_c/2) * out_c *sizeof(uint8_t));
    Map_uint8_t sp_matrix(sp_idx_data, kh * kw * (in_c/2), out_c, DynStride(out_c, 1));
    ii = 0;
    
    for (i = 0; i < kw * kh * in_c/2; i++){
        for (k = 0; k < out_c; k++) {
            sp_matrix(i, k) = (tmp_matrix(0, ii/4) >> (ii%4 *2)) &3;
            ++ii;
        }
        for (k = 0; k < ( stride_idx - out_c); k++)
            ++ii;
    }
    /////////////////////////////////////////////////////////////

    /*calculate the kernel shape*/
    Map_half rs2_matrix(rs2, kh * kw * (in_c/2), out_c, DynStride(k_stride, 1)); // the depth is same as in_c

    // pad_vs2
    half *rs2_pad = (half*)malloc(kh * kw * in_c * out_c * sizeof(half));
    for (i = 0; i < kh * kw * in_c; i++){
        for (j = 0; j < out_c; j++){
            *(rs2_pad + i*out_c + j) = (half)0;
        }
    }
    Map_half rs2_pad_matrix(rs2_pad, kh * kw * in_c, out_c, DynStride(out_c, 1));
    for (i = 0; i < kh * kw * in_c; i+=4){
        for (j = 0; j < out_c; j++){
            sp_index1 = sp_matrix(i/2, j);
            sp_index2 = sp_matrix(i/2+1, j);
            rs2_pad_matrix(i+sp_index1, j) = rs2_matrix(i/2, j);
            rs2_pad_matrix(i+sp_index2, j) = rs2_matrix(i/2+1, j);
        }
    }


    h = dilation_h > 1 ? dilation_h * (kh - 1) + 1 : kh;
    w = dilation_w > 1 ? dilation_w * (kw - 1) + 1 : kw;
    okh = kh;
    okw = kw;
    kh = h;
    kw = w;

    /*calculate the input shape*/
    Map_half rs1_matrix(rs1, in_h * in_w, in_c, DynStride(in_stride, 1));

    /*calculate the output shape*/
    h = (in_h + pad_top + pad_bottom - kh + 1 + sk_h - 1) / sk_h;
    w = (in_w + pad_left + pad_right - kw + 1 + sk_w - 1) / sk_w;
    left_val = (half *)malloc(h * w * okh * okw * in_c * sizeof(half));

    for (i = 0; i < h; i++) {
        for (j = 0; j < w; j++) {
            start = left_val + i * w * okh * okw * in_c + j * okh * okw * in_c;
            rs1_start = rs1;

            for (ii = 0; ii < kh; ii++) {
                for (jj = 0; jj < kw; jj++) {
                    for (kk = 0; kk < in_c; kk++) {
                        row = i * sk_h + ii;
                        col = j * sk_w + jj;

                        if (ii % dilation_h)
                            continue;
                        else {
                            if (jj % dilation_w)
                                continue;
                            else {
				int start_offset = ii/dilation_h * okw * in_c + jj/dilation_w * in_c + kk;
				int rs1_offset = (row - pad_top) * in_w * in_stride + (col - pad_left) * in_stride + kk;
                                if (row >= pad_top && row < pad_top + in_h && col >= pad_left && col < pad_left + in_w) {
                                    *(start + start_offset) = *(rs1_start + rs1_offset);
                                    val = *(rs1_start + rs1_offset);
                                }
                                else {
                                    *(start + start_offset) = (half)0;
                                    val =  (half)0;
                                }

                                if (debug) {
                                    printf("rd offset= %d val = 0x%x ",start_offset, val.x);
                                    val = *(start + start_offset);
                                    printf("rd val = 0x%x\n", val.x);
                                }
                            }//jj % dilation_w
                        }//ii % dilation_h
                    }//kk
                }//jj
            }//ii
        }//j
    }//i

    /*param check*/
    if (debug)
        printf("h = %d w = %d out_h = %d out_w = %d\n", h, w, out_h, out_w);
    assert(h == out_h && w == out_w);

    /*calculate convolution*/
    Map_half left_matrix(left_val, h * w, okh * okw * in_c, DynStride(okh * okw * in_c, 1));
    Map_half rd_matrix(rd, out_h * out_w, out_c, DynStride(out_stride, 1));
    row_val = (half *)malloc(okh * okw * in_c * sizeof(half));
    col_val = (half *)malloc(okh * okw * in_c/2 * sizeof(half));
    idx_val = (uint8_t *)malloc(okh * okw * in_c/2 * sizeof(uint8_t));
    Map_half row_matrix(row_val, 1, okh * okw * in_c, DynStride(okh * okw * in_c, 1));
    Map_half col_matrix(col_val, 1, okh * okw * in_c / 2, DynStride(okh * okw * in_c/2, 1));
    Map_uint8_t idx_matrix(idx_val, 1, okh * okw * in_c / 2, DynStride(okh * okw * in_c/2, 1));
    float32_t odd, even;
    if (debug) {
        cout << "rs1: " << rs1_matrix << endl;
        cout << "left: " << left_matrix << endl;
        cout << "rs2: " << rs2_matrix << endl;
        cout << "rs2_pad: " << rs2_pad_matrix << endl;
        cout << "origin idx:\n";
        for (i = 0; i < (kw * kh * in_c/2 * (stride_idx) + 3)/4; i++)
            cout << (int32_t)tmp_matrix(0,i) << endl;
        cout << "trans idx:\n";
        for(i = 0; i < kw * kh * (in_c/2); i++){
            for (j = 0; j < out_c; j++)
                cout << (int32_t)sp_matrix(i,j) << "\t";
            cout << endl;
        }
    }
    for (i = 0; i < out_h * out_w; i++) {
        for (j = 0; j < out_c; j++) {
            row_matrix = left_matrix.row(i);
            col_matrix = rs2_matrix.col(j).transpose();
            idx_matrix = sp_matrix.col(j).transpose();
            odd.v = 0x80000000;
            even.v = 0x80000000;
            
            for (k = 0; k < okh * okw * in_c; k+=4) {
                sp_index1 = idx_matrix(0, k/2);
                sp_index2 = idx_matrix(0, k/2+1);
                even = f32_add(half_mul_f32(row_matrix(0, k+sp_index1), col_matrix(0, k/2)), even);
                odd = f32_add(half_mul_f32(row_matrix(0, k+sp_index2), col_matrix(0, k/2+1)), odd);
                if (debug) {
                    cout << "index: " << (int)sp_index1 << " " << (int)sp_index2 << endl;
                    cout << row_matrix(0, k+sp_index1) << " * " << col_matrix(0, k/2) << " = ";
                    cout << hex << even.v << endl;
                    cout << row_matrix(0, k+sp_index2) << " * " << col_matrix(0, k/2+1) << " = ";
                    cout << hex << odd.v << endl;
                }
            }
            rd_matrix(i, j) = f32_to_half(f32_add(even, odd));
        }
    }
    

    if (debug)
        cout << "rd:" << endl << rd_matrix << endl;

    free(row_val);
    free(col_val);
    free(left_val);
    free(idx_val);
    free(sp_idx_data);
    return 0;
}

int CustomInsns::meconv_sp_mm(half *rs1, int8_t *rs2, uint8_t *sparseidx, half *rd, struct ConvShapeStride *ss)
{
    int pad_top, pad_bottom, pad_left, pad_right;
    int kw, kh, okw, okh, k_stride, sk_h, sk_w, stride_idx;
    int in_w, in_h, in_c, in_stride;
    int out_w, out_h, out_c, out_stride;
    int w, h, c;
    int dilation_h, dilation_w;
    int i, j, k, ii, jj, kk, index_cin, counter;
    int row, col;
    uint32_t sp_index1, sp_index2;
    half *rs1_start;
    half *left_val, *row_val;
    int8_t *col_val;
    half *start;
    half val;
    uint8_t *sp_idx_data, * idx_val;

    //get the padding
    pad_top = (ss->conv_padding >> 24) & 0xff;
    pad_bottom = (ss->conv_padding >> 16) & 0xff;
    pad_left = (ss->conv_padding >> 8) & 0xff;
    pad_right = ss->conv_padding & 0xff;

    //get the input shape
    in_w = (ss->conv_fm_in >> 16) & 0xffff;
    in_h = (ss->conv_fm_in) & 0xffff;
    in_c = (ss->conv_cin) & 0xffff;
    assert(in_w > 0 && in_h > 0 && in_c > 0);
    in_stride = (ss->conv_cin >> 16) & 0xffff;
    in_stride = in_stride > 0 ? in_stride : in_c;

    //get the output shape
    out_w = (ss->conv_fm_out >> 16) & 0xffff;
    out_h = (ss->conv_fm_out) & 0xffff;
    out_c = (ss->conv_cout) & 0xffff;
    assert(out_w > 0 && out_h > 0 && out_c > 0);
    out_stride = (ss->conv_cout >> 16) & 0xffff;
    out_stride = out_stride > 0 ? out_stride : out_c;

    //get the index stride
    stride_idx = ss->stride_idx? ss->stride_idx : out_c;

    //get the kernel shape
    kw = (ss->conv_kernel_params1 >> 24) & 0xff;
    kh = (ss->conv_kernel_params1 >> 16) & 0xff;
    dilation_h = (ss->conv_kernel_params1 >> 8) & 0xff;
    dilation_w = (ss->conv_kernel_params2 >> 24) & 0xff;
    sk_h = (ss->conv_kernel_params1) & 0xff;
    sk_w = (ss->conv_kernel_params2 >> 16) & 0xff;
    dilation_h = dilation_h == 0? 1 : dilation_h;
    dilation_w = dilation_w == 0? dilation_h : dilation_w;
    sk_h = sk_h == 0? 1 : sk_h;
    sk_w = sk_w == 0? sk_h : sk_w;
    assert(kw > 0 && kh > 0);
    assert(dilation_h > 0 && dilation_w > 0 && sk_h >0 && sk_w > 0);
    k_stride = ss->conv_kernel_params2 & 0xffff;
    k_stride = k_stride > 0 ? k_stride : out_c;

    //get de/quant coeff
    float16_t quant_coeff = f32_to_f16(ss->mme_quant_coeff);
    float16_t dequant_coeff = f32_to_f16(ss->mme_dequant_coeff);
    
    /* split the 8bit index shape into 2bit */
    i = (kw * kh * in_c / 2 * stride_idx + 3)/4;
    Map_uint8_t tmp_matrix(sparseidx, 1, i, DynStride(i, 1));
    sp_idx_data = (uint8_t *)malloc(kw * kh * (in_c/2) * out_c *sizeof(uint8_t));
    Map_uint8_t sp_matrix(sp_idx_data, kh * kw * (in_c/2), out_c, DynStride(out_c, 1));
    ii = 0;
    
    for (i = 0; i < kw * kh * in_c/2; i++){
        for (k = 0; k < out_c; k++) {
            sp_matrix(i, k) = (tmp_matrix(0, ii/4) >> (ii%4 *2)) &3;
            ++ii;
        }
        for (k = 0; k < ( stride_idx - out_c); k++)
            ++ii;
    }
    /////////////////////////////////////////////////////////////

    /*calculate the kernel shape*/
    Map_int8_t rs2_matrix(rs2, kh * kw * (in_c/2), out_c, DynStride(k_stride, 1)); // the depth is same as in_c
    // pad_vs2
    int8_t *rs2_pad = (int8_t*)malloc(kh * kw * in_c * out_c * sizeof(int8_t));
    for (i = 0; i < kh * kw * in_c; i++){
        for (j = 0; j < out_c; j++){
            *(rs2_pad + i*out_c + j) = 0;
        }
    }
    Map_int8_t rs2_pad_matrix(rs2_pad, kh * kw * in_c, out_c, DynStride(out_c, 1));
    for (i = 0; i < kh * kw * in_c; i+=4){
        for (j = 0; j < out_c; j++){
            sp_index1 = sp_matrix(i/2, j);
            sp_index2 = sp_matrix(i/2+1, j);
            rs2_pad_matrix(i+sp_index1, j) = rs2_matrix(i/2, j);
            rs2_pad_matrix(i+sp_index2, j) = rs2_matrix(i/2+1, j);
        }
    }
    
    h = dilation_h > 1 ? dilation_h * (kh - 1) + 1 : kh;
    w = dilation_w > 1 ? dilation_w * (kw - 1) + 1 : kw;
    okh = kh;
    okw = kw;
    kh = h;
    kw = w;

    /*calculate the input shape*/
    Map_half rs1_matrix(rs1, in_h * in_w, in_c, DynStride(in_stride, 1));

    /*calculate the output shape*/
    h = (in_h + pad_top + pad_bottom - kh + 1 + sk_h - 1) / sk_h;
    w = (in_w + pad_left + pad_right - kw + 1 + sk_w - 1) / sk_w;
    left_val = (half *)malloc(h * w * okh * okw * in_c * sizeof(half));

    for (i = 0; i < h; i++) {
        for (j = 0; j < w; j++) {
            start = left_val + i * w * okh * okw * in_c + j * okh * okw * in_c;
            rs1_start = rs1;

            for (ii = 0; ii < kh; ii++) {
                for (jj = 0; jj < kw; jj++) {
                    for (kk = 0; kk < in_c; kk++) {
                        row = i * sk_h + ii;
                        col = j * sk_w + jj;

                        if (ii % dilation_h)
                            continue;
                        else {
                            if (jj % dilation_w)
                                continue;
                            else {
				int start_offset = ii/dilation_h * okw * in_c + jj/dilation_w * in_c + kk;
				int rs1_offset = (row - pad_top) * in_w * in_stride + (col - pad_left) * in_stride + kk;
                                if (row >= pad_top && row < pad_top + in_h && col >= pad_left && col < pad_left + in_w) {
                                    *(start + start_offset) = *(rs1_start + rs1_offset);
                                    val = *(rs1_start + rs1_offset);
                                }
                                else {
                                    *(start + start_offset) = (half)0;
                                    val =  (half)0;
                                }

                                if (debug) {
                                    printf("rd offset= %d val = 0x%x ",start_offset, val.x);
                                    val = *(start + start_offset);
                                    printf("rd val = 0x%x\n", val.x);
                                }
                            }//jj % dilation_w
                        }//ii % dilation_h
                    }//kk
                }//jj
            }//ii
        }//j
    }//i

    /*param check*/
    if (debug)
        printf("h = %d w = %d out_h = %d out_w = %d\n", h, w, out_h, out_w);
    assert(h == out_h && w == out_w);

    /*calculate convolution*/
    Map_half left_matrix(left_val, h * w, okh * okw * in_c, DynStride(okh * okw * in_c, 1));
    Map_half rd_matrix(rd, out_h * out_w, out_c, DynStride(out_stride, 1));
    row_val = (half *)malloc(okh * okw * in_c * sizeof(half));
    col_val = (int8_t *)malloc(okh * okw * in_c * sizeof(int8_t));
    Map_half row_matrix(row_val, 1, okh * okw * in_c, DynStride(okh * okw * in_c, 1));
    Map_int8_t col_matrix(col_val, 1, okh * okw * in_c, DynStride(okh * okw * in_c, 1));
    int32_t res;
    float16_t rs1_f16;
    int8_t rs1_i8;
    //rd_matrix = left_matrix * rs2_matrix;
    for (i = 0; i < out_h * out_w; i++) {
        for (j = 0; j < out_c; j++) {
            row_matrix = left_matrix.row(i);
            col_matrix = rs2_pad_matrix.col(j).transpose();
            res = 0;
            for (k = 0; k < okh * okw * in_c; k++) {
                rs1_f16 = f16_mul(half_to_float16_t(row_matrix(0, k)), quant_coeff);
                rs1_i8 = f16_to_i8(rs1_f16, softfloat_round_near_maxMag, true);
                res += rs1_i8 * col_matrix(0, k);
            }
            rd_matrix(i, j) = int32_mul_f16(res, dequant_coeff); 
        }
    }


    if (debug)
        cout << "rd:" << endl << rd_matrix << endl;

    free(row_val);
    free(col_val);
    free(left_val);
    free(idx_val);
    free(sp_idx_data);
    return 0;
}

int CustomInsns::meconv_sp_mm(int8_t *rs1, int8_t *rs2, uint8_t *sparseidx, half *rd, struct ConvShapeStride *ss)
{
    int pad_top, pad_bottom, pad_left, pad_right;
    int kw, kh, okw, okh, k_stride, sk_h, sk_w, stride_idx;
    int in_w, in_h, in_c, in_stride;
    int out_w, out_h, out_c, out_stride;
    int w, h, c;
    int dilation_h, dilation_w;
    int i, j, k, ii, jj, kk, index_cin, counter;
    int row, col;
    uint32_t sp_index1, sp_index2;
    int8_t *rs1_start;
    int8_t *left_val, *row_val;
    int8_t *col_val;
    int8_t *start;
    int32_t val;
    uint8_t *sp_idx_data, * idx_val;

    //get the padding
    pad_top = (ss->conv_padding >> 24) & 0xff;
    pad_bottom = (ss->conv_padding >> 16) & 0xff;
    pad_left = (ss->conv_padding >> 8) & 0xff;
    pad_right = ss->conv_padding & 0xff;

    //get the input shape
    in_w = (ss->conv_fm_in >> 16) & 0xffff;
    in_h = (ss->conv_fm_in) & 0xffff;
    in_c = (ss->conv_cin) & 0xffff;
    assert(in_w > 0 && in_h > 0 && in_c > 0);
    in_stride = (ss->conv_cin >> 16) & 0xffff;
    in_stride = in_stride > 0 ? in_stride : in_c;

    //get the output shape
    out_w = (ss->conv_fm_out >> 16) & 0xffff;
    out_h = (ss->conv_fm_out) & 0xffff;
    out_c = (ss->conv_cout) & 0xffff;
    assert(out_w > 0 && out_h > 0 && out_c > 0);
    out_stride = (ss->conv_cout >> 16) & 0xffff;
    out_stride = out_stride > 0 ? out_stride : out_c;

    //get the index stride
    stride_idx = ss->stride_idx? ss->stride_idx : out_c;

    //get the kernel shape
    kw = (ss->conv_kernel_params1 >> 24) & 0xff;
    kh = (ss->conv_kernel_params1 >> 16) & 0xff;
    dilation_h = (ss->conv_kernel_params1 >> 8) & 0xff;
    dilation_w = (ss->conv_kernel_params2 >> 24) & 0xff;
    sk_h = (ss->conv_kernel_params1) & 0xff;
    sk_w = (ss->conv_kernel_params2 >> 16) & 0xff;
    dilation_h = dilation_h == 0? 1 : dilation_h;
    dilation_w = dilation_w == 0? dilation_h : dilation_w;
    sk_h = sk_h == 0? 1 : sk_h;
    sk_w = sk_w == 0? sk_h : sk_w;
    assert(kw > 0 && kh > 0);
    assert(dilation_h > 0 && dilation_w > 0 && sk_h >0 && sk_w > 0);
    k_stride = ss->conv_kernel_params2 & 0xffff;
    k_stride = k_stride > 0 ? k_stride : out_c;

    //get de/quant coeff
    float16_t quant_coeff = f32_to_f16(ss->mme_quant_coeff);
    float16_t dequant_coeff = f32_to_f16(ss->mme_dequant_coeff);
    
    /* split the 8bit index shape into 2bit */
    i = (kw * kh * in_c / 2 * stride_idx + 3)/4;
    Map_uint8_t tmp_matrix(sparseidx, 1, i, DynStride(i, 1));
    sp_idx_data = (uint8_t *)malloc(kw * kh * (in_c/2) * out_c *sizeof(uint8_t));
    Map_uint8_t sp_matrix(sp_idx_data, kh * kw * (in_c/2), out_c, DynStride(out_c, 1));
    ii = 0;
    
    for (i = 0; i < kw * kh * in_c/2; i++){
        for (k = 0; k < out_c; k++) {
            sp_matrix(i, k) = (tmp_matrix(0, ii/4) >> (ii%4 *2)) &3;
            ++ii;
        }
        for (k = 0; k < ( stride_idx - out_c); k++)
            ++ii;
    }
    /////////////////////////////////////////////////////////////

    /*calculate the kernel shape*/
    Map_int8_t rs2_matrix(rs2, kh * kw * (in_c/2), out_c, DynStride(k_stride, 1)); // the depth is same as in_c
    // pad_vs2
    int8_t *rs2_pad = (int8_t*)malloc(kh * kw * in_c * out_c * sizeof(int8_t));
    for (i = 0; i < kh * kw * in_c; i++){
        for (j = 0; j < out_c; j++){
            *(rs2_pad + i*out_c + j) = 0;
        }
    }
    Map_int8_t rs2_pad_matrix(rs2_pad, kh * kw * in_c, out_c, DynStride(out_c, 1));
    for (i = 0; i < kh * kw * in_c; i+=4){
        for (j = 0; j < out_c; j++){
            sp_index1 = sp_matrix(i/2, j);
            sp_index2 = sp_matrix(i/2+1, j);
            rs2_pad_matrix(i+sp_index1, j) = rs2_matrix(i/2, j);
            rs2_pad_matrix(i+sp_index2, j) = rs2_matrix(i/2+1, j);
        }
    }

    h = dilation_h > 1 ? dilation_h * (kh - 1) + 1 : kh;
    w = dilation_w > 1 ? dilation_w * (kw - 1) + 1 : kw;
    okh = kh;
    okw = kw;
    kh = h;
    kw = w;

    /*calculate the input shape*/
    Map_int8_t rs1_matrix(rs1, in_h * in_w, in_c, DynStride(in_stride, 1));

    /*calculate the output shape*/
    h = (in_h + pad_top + pad_bottom - kh + 1 + sk_h - 1) / sk_h;
    w = (in_w + pad_left + pad_right - kw + 1 + sk_w - 1) / sk_w;
    left_val = (int8_t *)malloc(h * w * okh * okw * in_c * sizeof(int8_t));

    for (i = 0; i < h; i++) {
        for (j = 0; j < w; j++) {
            start = left_val + i * w * okh * okw * in_c + j * okh * okw * in_c;
            rs1_start = rs1;

            for (ii = 0; ii < kh; ii++) {
                for (jj = 0; jj < kw; jj++) {
                    for (kk = 0; kk < in_c; kk++) {
                        row = i * sk_h + ii;
                        col = j * sk_w + jj;

                        if (ii % dilation_h)
                            continue;
                        else {
                            if (jj % dilation_w)
                                continue;
                            else {
				int start_offset = ii/dilation_h * okw * in_c + jj/dilation_w * in_c + kk;
				int rs1_offset = (row - pad_top) * in_w * in_stride + (col - pad_left) * in_stride + kk;
                                if (row >= pad_top && row < pad_top + in_h && col >= pad_left && col < pad_left + in_w) {
                                    *(start + start_offset) = *(rs1_start + rs1_offset);
                                    val = *(rs1_start + rs1_offset);
                                }
                                else {
                                    *(start + start_offset) = 0;
                                    val = 0;
                                }

                                if (debug) {
                                    printf("rd offset= %d val = 0x%x ",start_offset, val);
                                    val = *(start + start_offset);
                                    printf("rd val = 0x%x\n", val);
                                }
                            }//jj % dilation_w
                        }//ii % dilation_h
                    }//kk
                }//jj
            }//ii
        }//j
    }//i

    /*param check*/
    if (debug)
        printf("h = %d w = %d out_h = %d out_w = %d\n", h, w, out_h, out_w);
    assert(h == out_h && w == out_w);

    /*calculate convolution*/
    Map_int8_t left_matrix(left_val, h * w, okh * okw * in_c, DynStride(okh * okw * in_c, 1));
    Map_half rd_matrix(rd, out_h * out_w, out_c, DynStride(out_stride, 1));
    row_val = (int8_t *)malloc(okh * okw * in_c * sizeof(int8_t));
    col_val = (int8_t *)malloc(okh * okw * in_c * sizeof(int8_t));
    Map_int8_t row_matrix(row_val, 1, okh * okw * in_c, DynStride(okh * okw * in_c, 1));
    Map_int8_t col_matrix(col_val, 1, okh * okw * in_c, DynStride(okh * okw * in_c, 1));
    
    int32_t res;
    for (i = 0; i < out_h * out_w; i++) {
        for (j = 0; j < out_c; j++) {
            row_matrix = left_matrix.row(i);
            col_matrix = rs2_pad_matrix.col(j).transpose();
            res = 0;
            for (k = 0; k < okh * okw * in_c; k++) {
                res += (int32_t)(row_matrix(0, k) * col_matrix(0, k));
            }
            rd_matrix(i, j) = int32_mul_f16(res, dequant_coeff); 
        }
    }

    if (debug)
        cout << "rd:" << endl << rd_matrix << endl;

    free(row_val);
    free(col_val);
    free(left_val);
    free(idx_val);
    free(sp_idx_data);
    return 0;
}


int CustomInsns::meconv_sp_mm(float32_t *rs1, float32_t *rs2, uint8_t *sparseidx, float32_t *rd, struct ConvShapeStride *ss)
{
    int pad_top, pad_bottom, pad_left, pad_right;
    int kw, kh, okw, okh, k_stride, sk_h, sk_w, stride_idx;
    int in_w, in_h, in_c, in_stride;
    int out_w, out_h, out_c, out_stride;
    int w, h, c;
    int dilation_h, dilation_w;
    int i, j, k, ii, jj, kk, index_cin, counter;
    int row, col;
    uint32_t sp_index1, sp_index2;
    float32_t *rs1_start;
    float32_t *left_val, *row_val, *col_val;
    float32_t *start;
    float32_t val;
    uint8_t *sp_idx_data, *idx_val;

    //get the padding
    pad_top = (ss->conv_padding >> 24) & 0xff;
    pad_bottom = (ss->conv_padding >> 16) & 0xff;
    pad_left = (ss->conv_padding >> 8) & 0xff;
    pad_right = ss->conv_padding & 0xff;

    //get the input shape
    in_w = (ss->conv_fm_in >> 16) & 0xffff;
    in_h = (ss->conv_fm_in) & 0xffff;
    in_c = (ss->conv_cin) & 0xffff;
    assert(in_w > 0 && in_h > 0 && in_c > 0);
    in_stride = (ss->conv_cin >> 16) & 0xffff;
    // assert((in_stride % 2) == 0); //half
    in_stride = in_stride > 0 ? in_stride : in_c;

    //get the output shape
    out_w = (ss->conv_fm_out >> 16) & 0xffff;
    out_h = (ss->conv_fm_out) & 0xffff;
    out_c = (ss->conv_cout) & 0xffff;
    assert(out_w > 0 && out_h > 0 && out_c > 0);
    out_stride = (ss->conv_cout >> 16) & 0xffff;
    // assert(out_stride % 2 == 0);
    out_stride = out_stride > 0 ? out_stride : out_c;

    //get the index stride
    stride_idx = ss->stride_idx? ss->stride_idx : out_c;

    //get the kernel shape
    kw = (ss->conv_kernel_params1 >> 24) & 0xff;
    kh = (ss->conv_kernel_params1 >> 16) & 0xff;
    dilation_h = (ss->conv_kernel_params1 >> 8) & 0xff;
    dilation_w = (ss->conv_kernel_params2 >> 24) & 0xff;
    sk_h = (ss->conv_kernel_params1) & 0xff;
    sk_w = (ss->conv_kernel_params2 >> 16) & 0xff;
    dilation_h = dilation_h == 0? 1 : dilation_h;
    dilation_w = dilation_w == 0? dilation_h : dilation_w;
    sk_h = sk_h == 0? 1 : sk_h;
    sk_w = sk_w == 0? sk_h : sk_w;
    assert(kw > 0 && kh > 0);
    assert(dilation_h > 0 && dilation_w > 0 && sk_h >0 && sk_w > 0);
    k_stride = ss->conv_kernel_params2 & 0xffff;
    //assert(k_stride % 2 == 0);
    k_stride = k_stride > 0 ? k_stride : out_c;

    /* split the 8bit index shape into 2bit */
    i = (kw * kh * in_c / 2 * stride_idx + 3)/4;
    Map_uint8_t tmp_matrix(sparseidx, 1, i, DynStride(i, 1));
    sp_idx_data = (uint8_t *)malloc(kw * kh * (in_c/2) * out_c *sizeof(uint8_t));
    Map_uint8_t sp_matrix(sp_idx_data, kh * kw * (in_c/2), out_c, DynStride(out_c, 1));
    ii = 0;
    
    for (i = 0; i < kw * kh * in_c/2; i++){
        for (k = 0; k < out_c; k++) {
            sp_matrix(i, k) = (tmp_matrix(0, ii/4) >> (ii%4 *2)) &3;
            ++ii;
        }
        for (k = 0; k < ( stride_idx - out_c); k++)
            ++ii;
    }
    /////////////////////////////////////////////////////////////

    /*calculate the kernel shape*/
    Map_float32_t rs2_matrix(rs2, kh * kw * (in_c/2), out_c, DynStride(k_stride, 1)); // the depth is same as in_c

    // pad_vs2
    float32_t *rs2_pad = (float32_t*)malloc(kh * kw * in_c * out_c * sizeof(float32_t));
    for (i = 0; i < kh * kw * in_c; i++){
        for (j = 0; j < out_c; j++){
            *(rs2_pad + i*out_c + j) = i32_to_f32(0);
        }
    }
    Map_float32_t rs2_pad_matrix(rs2_pad, kh * kw * in_c, out_c, DynStride(out_c, 1));
    for (i = 0; i < kh * kw * in_c; i+=4){
        for (j = 0; j < out_c; j++){
            sp_index1 = sp_matrix(i/2, j);
            sp_index2 = sp_matrix(i/2+1, j);
            rs2_pad_matrix(i+sp_index1, j) = rs2_matrix(i/2, j);
            rs2_pad_matrix(i+sp_index2, j) = rs2_matrix(i/2+1, j);
        }
    }

    h = dilation_h > 1 ? dilation_h * (kh - 1) + 1 : kh;
    w = dilation_w > 1 ? dilation_w * (kw - 1) + 1 : kw;
    okh = kh;
    okw = kw;
    kh = h;
    kw = w;

    /*calculate the input shape*/
    Map_float32_t rs1_matrix(rs1, in_h * in_w, in_c, DynStride(in_stride, 1));

    /*calculate the output shape*/
    h = (in_h + pad_top + pad_bottom - kh + 1 + sk_h - 1) / sk_h;
    w = (in_w + pad_left + pad_right - kw + 1 + sk_w - 1) / sk_w;
    left_val = (float32_t *)malloc(h * w * okh * okw * in_c * sizeof(float32_t));

    for (i = 0; i < h; i++) {
        for (j = 0; j < w; j++) {
            start = left_val + i * w * okh * okw * in_c + j * okh * okw * in_c;
            rs1_start = rs1;

            for (ii = 0; ii < kh; ii++) {
                for (jj = 0; jj < kw; jj++) {
                    for (kk = 0; kk < in_c; kk++) {
                        row = i * sk_h + ii;
                        col = j * sk_w + jj;

                        if (ii % dilation_h)
                            continue;
                        else {
                            if (jj % dilation_w)
                                continue;
                            else {
				int start_offset = ii/dilation_h * okw * in_c + jj/dilation_w * in_c + kk;
				int rs1_offset = (row - pad_top) * in_w * in_stride + (col - pad_left) * in_stride + kk;
                                if (row >= pad_top && row < pad_top + in_h && col >= pad_left && col < pad_left + in_w) {
                                    *(start + start_offset) = *(rs1_start + rs1_offset);
                                    val = *(rs1_start + rs1_offset);
                                }
                                else {
                                    *(start + start_offset) = i32_to_f32(0);
                                    val =  i32_to_f32(0);
                                }

                                if (debug) {
                                    printf("rd offset= %d val = 0x%x ",start_offset, val.v);
                                    val = *(start + start_offset);
                                    printf("rd val = 0x%x\n", val.v);
                                }
                            }//jj % dilation_w
                        }//ii % dilation_h
                    }//kk
                }//jj
            }//ii
        }//j
    }//i

    /*param check*/
    if (debug)
        printf("h = %d w = %d out_h = %d out_w = %d\n", h, w, out_h, out_w);
    assert(h == out_h && w == out_w);

    /*calculate convolution*/
    Map_float32_t left_matrix(left_val, h * w, okh * okw * in_c, DynStride(okh * okw * in_c, 1));
    Map_float32_t rd_matrix(rd, out_h * out_w, out_c, DynStride(out_stride, 1));
    row_val = (float32_t *)malloc(okh * okw * in_c * sizeof(float32_t));
    col_val = (float32_t *)malloc(okh * okw * in_c / 2 * sizeof(float32_t));
    idx_val = (uint8_t *)malloc(okh * okw * in_c/2 * sizeof(uint8_t));
    Map_float32_t row_matrix(row_val, 1, okh * okw * in_c, DynStride(okh * okw * in_c, 1));
    Map_float32_t col_matrix(col_val, 1, okh * okw * in_c/2, DynStride(okh * okw * in_c / 2, 1));
    Map_uint8_t idx_matrix(idx_val, 1, okh * okw * in_c / 2, DynStride(okh * okw * in_c/2, 1));
    float32_t odd, even;


    for (i = 0; i < out_h * out_w; i++) {
        for (j = 0; j < out_c; j++) {
            row_matrix = left_matrix.row(i);
            col_matrix = rs2_matrix.col(j).transpose();
            idx_matrix = sp_matrix.col(j).transpose();
            even.v = 0x80000000;
            odd.v = 0x80000000;
            
            for (k = 0; k < okh * okw * in_c; k+=4) {
                sp_index1 = idx_matrix(0, k/2);
                sp_index2 = idx_matrix(0, k/2+1);
                even = f32_add(tf32_mul(row_matrix(0, k+sp_index1), col_matrix(0, k/2)), even);
                odd = f32_add(tf32_mul(row_matrix(0, k+sp_index2), col_matrix(0, k/2+1)), odd);
            }
            rd_matrix(i, j) = f32_add(even, odd);
        }
    }

    free(row_val);
    free(col_val);
    free(left_val);
    free(idx_val);
    free(sp_idx_data);
    return 0;
}

int CustomInsns::medeconv_mm(half *rs1, half *rs2, half *rd, struct ConvShapeStride *ss)
{
    int pad_top, pad_bottom, pad_left, pad_right;
    int in_pad_top, in_pad_bottom, in_pad_left, in_pad_right;
    int kw, kh, k_stride, stride_idx;
    int in_w, in_h, in_c, in_stride;
    int out_w, out_h, out_c, out_stride;
    int w, h, c;
    int stride_w, stride_h;
    int i, j, k, m, ii, jj, kk, index_cin, counter;
    int row, col, row_pad, col_pad;
    int rs2_offset, ker_offset;
    uint32_t sp_index1, sp_index2;
    half *rs1_start, *rs1_pad;
    half *left_val, *row_val, *col_val, *ker_val;
    half *start;
    half val;

    //get the padding
    pad_top = (ss->conv_padding >> 24) & 0xff;
    pad_bottom = (ss->conv_padding >> 16) & 0xff;
    pad_left = (ss->conv_padding >> 8) & 0xff;
    pad_right = ss->conv_padding & 0xff;

    //get the input shape
    in_w = (ss->conv_fm_in >> 16) & 0xffff;
    in_h = (ss->conv_fm_in) & 0xffff;
    in_c = (ss->conv_cin) & 0xffff;
    assert(in_w > 0 && in_h > 0 && in_c > 0);
    in_stride = (ss->conv_cin >> 16) & 0xffff;
    // assert((in_stride % 2) == 0); //half
    in_stride = in_stride > 0 ? in_stride : in_c;

    //get the output shape
    out_w = (ss->conv_fm_out >> 16) & 0xffff;
    out_h = (ss->conv_fm_out) & 0xffff;
    out_c = (ss->conv_cout) & 0xffff;
    assert(out_w > 0 && out_h > 0 && out_c > 0);
    out_stride = (ss->conv_cout >> 16) & 0xffff;
    // assert(out_stride % 2 == 0);
    out_stride = out_stride > 0 ? out_stride : out_c;

    //get the kernel shape
    kw = (ss->conv_kernel_params1 >> 24) & 0xff;
    kh = (ss->conv_kernel_params1 >> 16) & 0xff;
    stride_h = (ss->conv_kernel_params1) & 0xff;
    stride_w = (ss-> conv_kernel_params2 >> 16) & 0xff;
    assert(kw > 0 && kh > 0 && stride_h > 0 && stride_w > 0);
    k_stride = ss->conv_kernel_params2 & 0xffff;
    // assert(k_stride % 2 == 0);
    k_stride = k_stride > 0 ? k_stride : out_c;

    /*calculate & pad the rs1 shape*/
    in_pad_top = kh - pad_top - 1;
    in_pad_bottom = kh - pad_bottom - 1;
    in_pad_left = kw - pad_left - 1;
    in_pad_right = kw - pad_right - 1;
    Map_half rs1_matrix(rs1, in_h * in_w, in_c, DynStride(in_stride, 1));
    row = in_h + (in_h - 1) * (stride_h - 1) + in_pad_top + in_pad_bottom;
    col = in_w + (in_w - 1) * (stride_w - 1) + in_pad_right + in_pad_left;
    rs1_pad = (half *)malloc(row * col * in_c * sizeof(half));
    Map_half rs1_pad_matrix(rs1_pad, row * col, in_c, DynStride(in_c, 1));
    for (i = 0; i < row * col * in_c; i++)
        (*(rs1_pad+i)).x = 0.0;
    ii=0;
    for (i = in_pad_top; i < (row-in_pad_bottom); i+=stride_h) {
        for (j = in_pad_left; j < (col-in_pad_right); j+=stride_w){
            for (k = 0; k < in_c; k++)
                rs1_pad_matrix(i*col+j, k) = rs1_matrix(ii,k);
            ++ii;
        }
    }

    // /*kernel 上下左右翻转*/
    // ker_val = (half *)malloc(kh * kw * out_c * in_c * sizeof(half));
    // for (i = 0; i < kh; i++){
    //     for (j = 0; j < kw; j++){
    //         for (k = 0; k < out_c; k++){
    //             for (m = 0; m < in_c; m++){
    //                 rs2_offset = i * kw * k_stride * out_c + j * k_stride * out_c + k * k_stride + m;
    //                 ker_offset = (kh-i-1) * kw * in_c * out_c + (kw-j-1) * in_c * out_c + m * out_c + k;
    //                 *(ker_val + ker_offset) = *(rs2 + rs2_offset);
    //             }
    //         }
    //     }
    // }

    Map_half rs2_matrix(rs2, kh * kw * in_c, out_c, DynStride(k_stride, 1));

    /* calculate the output shape;
       out_h = row + kernel_h -1; 
       out_w = col + kernel - 1
    */
    h = (in_h - 1) * stride_h - pad_top - pad_bottom + kh;
    w = (in_w - 1) * stride_w - pad_left - pad_right + kw;
    if (debug){
        MECONV_INFO(ss);
        printf("in_h = %d, in_w = %d, in_c = %d\n", in_h, in_w, in_c);
        printf("pad_t");
        printf("h = %d w = %d out_h = %d out_w = %d\n", h, w, out_h, out_w);
    }
    assert(h==out_h && w==out_w);
    left_val = (half *)malloc(h * w * kh * kw * in_c * sizeof(half));
    for (i = 0; i < h; i++){
        for (j = 0; j < w; j++){
            start = left_val + i * w * kh * kw * in_c + j * kh * kw * in_c; 
            rs1_start = rs1_pad;

            for (ii = 0; ii < kh; ii++){
                for (jj = 0; jj < kw; jj++){
                    row_pad = i + ii;
                    col_pad = j + jj;
                    for (kk = 0; kk < in_c; kk++){
                        int start_offset = ii * kw * in_c + jj * in_c + kk;
                        int rs1_offset = row_pad * col * in_c + col_pad * in_c + kk;
                        *(start + start_offset) = *(rs1_start + rs1_offset);
                        val = *(rs1_start + rs1_offset);

                        if (debug) {
                            printf("rs1 offset= %d val = 0x%x ",start_offset, val.x);
                            val = *(start + start_offset);
                            printf("left val = 0x%x\n", val.x); 
                        }
                    }
                }
            }
        }
    }

    /* calc the convolution*/
    Map_half left_matrix(left_val, h * w, kh * kw * in_c, DynStride(kh * kw * in_c, 1));
    Map_half rd_matrix(rd, out_h * out_w, out_c, DynStride(out_stride, 1));
    row_val = (half *)malloc(kh * kw * in_c * sizeof(half));
    col_val = (half *)malloc(kh * kw * in_c * sizeof(half));
    Map_half row_matrix(row_val, 1, kh * kw * in_c, DynStride(kh * kw * in_c, 1));
    Map_half col_matrix(col_val, 1, kh * kw * in_c, DynStride(kh * kw * in_c, 1));
    float32_t odd, even, first, second, third, forth, res_tmp, res12, res34;
    if (debug) {
        cout << "rs1: " << rs1_matrix << endl;
        cout << "rs1_extend: " << rs1_pad_matrix << endl;
        cout << "left: " << left_matrix << endl;
        cout << "rs2: " << rs2_matrix << endl;
    }

    for (i = 0; i < out_h * out_w; i++){
        for (j = 0; j < out_c; j++){
            row_matrix = left_matrix.row(i);
            col_matrix = rs2_matrix.col(j).transpose();
            odd.v = 0;
            even.v = 0;
            first.v = 0;
            second.v = 0;
            third.v = 0;
            forth.v = 0;
            counter = 0;
            if ((k_stride == out_c) && (out_c <= 32)) {
                for (k = 0; k < kh *kw; k++){
                    int offset = k * in_c;
                    for (index_cin = 0; index_cin < in_c; index_cin++){
                        res_tmp =  half_mul_f32(row_matrix(0, offset + index_cin),
                                                col_matrix(0, offset + index_cin));
                        if(counter%4 == 0) first = f32_add(res_tmp, first);
                        else if(counter%4 == 1) second = f32_add(res_tmp, second);
                        else if(counter%4 == 2) third = f32_add(res_tmp, third);
                        else if(counter%4 == 3) forth = f32_add(res_tmp, forth);
                        ++counter;
                        if ((in_c % 2) && (index_cin == (in_c - 1))) 
                            ++counter;
                    }
                }
                res12 = f32_add(first, third);
                res34 = f32_add(second,forth);
                rd_matrix(i, j) = f32_to_half(f32_add(res12, res34));
            } else {
                for (k = 0; k < kh * kw * in_c; k++){
                    if (! (k % 2))
                        even = f32_add(half_mul_f32(row_matrix(0, k), col_matrix(0, k)), even);
                    else
                        odd = f32_add(half_mul_f32(row_matrix(0, k), col_matrix(0, k)), odd);
                }
                rd_matrix(i, j) = f32_to_half(f32_add(odd, even));
            }
        } 
    }


    if (debug)
        cout << "rd:" << endl << rd_matrix << endl;

    free(row_val);
    free(col_val);
    free(left_val);
    free(rs1_pad);
    free(ker_val);
    return 0;
}

int CustomInsns::medeconv_mm(half *rs1, int8_t *rs2, half *rd, struct ConvShapeStride *ss)
{
    int pad_top, pad_bottom, pad_left, pad_right;
    int in_pad_top, in_pad_bottom, in_pad_left, in_pad_right;
    int kw, kh, k_stride, stride_idx;
    int in_w, in_h, in_c, in_stride;
    int out_w, out_h, out_c, out_stride;
    int w, h, c;
    int stride_w, stride_h;
    int i, j, k, m, ii, jj, kk;
    int row, col, row_pad, col_pad;
    int rs2_offset, ker_offset;
    uint32_t sp_index1, sp_index2;
    half *rs1_start, *rs1_pad;
    half *left_val, *row_val, *ker_val;
    int8_t *col_val;
    half *start;
    half val;

    //get the padding
    pad_top = (ss->conv_padding >> 24) & 0xff;
    pad_bottom = (ss->conv_padding >> 16) & 0xff;
    pad_left = (ss->conv_padding >> 8) & 0xff;
    pad_right = ss->conv_padding & 0xff;

    //get the input shape
    in_w = (ss->conv_fm_in >> 16) & 0xffff;
    in_h = (ss->conv_fm_in) & 0xffff;
    in_c = (ss->conv_cin) & 0xffff;
    assert(in_w > 0 && in_h > 0 && in_c > 0);
    in_stride = (ss->conv_cin >> 16) & 0xffff;
    // assert((in_stride % 2) == 0); //half
    in_stride = in_stride > 0 ? in_stride : in_c;

    //get the output shape
    out_w = (ss->conv_fm_out >> 16) & 0xffff;
    out_h = (ss->conv_fm_out) & 0xffff;
    out_c = (ss->conv_cout) & 0xffff;
    assert(out_w > 0 && out_h > 0 && out_c > 0);
    out_stride = (ss->conv_cout >> 16) & 0xffff;
    out_stride = out_stride > 0 ? out_stride : out_c;

    //get the kernel shape
    kw = (ss->conv_kernel_params1 >> 24) & 0xff;
    kh = (ss->conv_kernel_params1 >> 16) & 0xff;
    stride_h = (ss->conv_kernel_params1) & 0xff;
    stride_w = (ss-> conv_kernel_params2 >> 16) & 0xff;
    assert(kw > 0 && kh > 0 && stride_h > 0 && stride_w > 0);
    k_stride = ss->conv_kernel_params2 & 0xffff;
    k_stride = k_stride > 0 ? k_stride : out_c;

    //get de/quant coeff
    float16_t quant_coeff = f32_to_f16(ss->mme_quant_coeff);
    float16_t dequant_coeff = f32_to_f16(ss->mme_dequant_coeff);

    /*calculate & pad the rs1 shape*/
    in_pad_top = kh - pad_top - 1;
    in_pad_bottom = kh - pad_bottom - 1;
    in_pad_left = kw - pad_left - 1;
    in_pad_right = kw - pad_right - 1;
    Map_half rs1_matrix(rs1, in_h * in_w, in_c, DynStride(in_stride, 1));
    row = in_h + (in_h - 1) * (stride_h - 1) + in_pad_top + in_pad_bottom;
    col = in_w + (in_w - 1) * (stride_w - 1) + in_pad_right + in_pad_left;
    rs1_pad = (half *)malloc(row * col * in_c * sizeof(half));
    Map_half rs1_pad_matrix(rs1_pad, row * col, in_c, DynStride(in_c, 1));
    for (i = 0; i < row * col * in_c; i++)
        (*(rs1_pad+i)).x = 0.0;
    ii=0;
    for (i = in_pad_top; i < (row-in_pad_bottom); i+=stride_h) {
        for (j = in_pad_left; j < (col-in_pad_right); j+=stride_w){
            for (k = 0; k < in_c; k++)
                rs1_pad_matrix(i*col+j, k) = rs1_matrix(ii,k);
            ++ii;
        }
    }

    Map_int8_t rs2_matrix(rs2, kh * kw * in_c, out_c, DynStride(k_stride, 1));

    /* calculate the output shape;
       out_h = row + kernel_h -1; 
       out_w = col + kernel - 1
    */
    h = (in_h - 1) * stride_h - pad_top - pad_bottom + kh;
    w = (in_w - 1) * stride_w - pad_left - pad_right + kw;
    // if (debug){
    //     MECONV_INFO(ss);
    //     printf("in_h = %d, in_w = %d, in_c = %d\n", in_h, in_w, in_c);
    //     printf("pad_t");
    //     printf("h = %d w = %d out_h = %d out_w = %d\n", h, w, out_h, out_w);
    // }
    assert(h==out_h && w==out_w);
    left_val = (half *)malloc(h * w * kh * kw * in_c * sizeof(half));
    for (i = 0; i < h; i++){
        for (j = 0; j < w; j++){
            start = left_val + i * w * kh * kw * in_c + j * kh * kw * in_c; 
            rs1_start = rs1_pad;

            for (ii = 0; ii < kh; ii++){
                for (jj = 0; jj < kw; jj++){
                    row_pad = i + ii;
                    col_pad = j + jj;
                    for (kk = 0; kk < in_c; kk++){
                        int start_offset = ii * kw * in_c + jj * in_c + kk;
                        int rs1_offset = row_pad * col * in_c + col_pad * in_c + kk;
                        *(start + start_offset) = *(rs1_start + rs1_offset);
                        val = *(rs1_start + rs1_offset);

                        // if (debug) {
                        //     printf("rs1 offset= %d val = 0x%x ",start_offset, val.x);
                        //     val = *(start + start_offset);
                        //     printf("left val = 0x%x\n", val.x); 
                        // }
                    }
                }
            }
        }
    }

    /* calc the convolution*/
    Map_half left_matrix(left_val, h * w, kh * kw * in_c, DynStride(kh * kw * in_c, 1));
    Map_half rd_matrix(rd, out_h * out_w, out_c, DynStride(out_stride, 1));
    row_val = (half *)malloc(kh * kw * in_c * sizeof(half));
    col_val = (int8_t *)malloc(kh * kw * in_c * sizeof(int8_t));
    Map_half row_matrix(row_val, 1, kh * kw * in_c, DynStride(kh * kw * in_c, 1));
    Map_int8_t col_matrix(col_val, 1, kh * kw * in_c, DynStride(kh * kw * in_c, 1));
    int32_t res;
    float16_t rs1_f16;
    int8_t rs1_i8;
    

    for (i = 0; i < out_h * out_w; i++){
        for (j = 0; j < out_c; j++){
            row_matrix = left_matrix.row(i);
            col_matrix = rs2_matrix.col(j).transpose();
            res = 0;
            for (k = 0; k < kh * kw * in_c; k++){
                rs1_f16 = f16_mul(half_to_float16_t(row_matrix(0, k)), quant_coeff);
                rs1_i8 = f16_to_i8(rs1_f16, softfloat_round_near_maxMag, true);
                res += rs1_i8 * col_matrix(0, k);
            }
            rd_matrix(i, j) = int32_mul_f16(res, dequant_coeff); 
            if (debug) cout << "rd: " << hex << rd_matrix(i, j).x << endl;
        } 
    }

    free(row_val);
    free(col_val);
    free(left_val);
    free(rs1_pad);
    free(ker_val);
    return 0;
}

int CustomInsns::medeconv_mm(int8_t *rs1, int8_t *rs2, half *rd, struct ConvShapeStride *ss)
{
    int pad_top, pad_bottom, pad_left, pad_right;
    int in_pad_top, in_pad_bottom, in_pad_left, in_pad_right;
    int kw, kh, k_stride, stride_idx;
    int in_w, in_h, in_c, in_stride;
    int out_w, out_h, out_c, out_stride;
    int w, h, c;
    int stride_w, stride_h;
    int i, j, k, m, ii, jj, kk;
    int row, col, row_pad, col_pad;
    int rs2_offset, ker_offset;
    uint32_t sp_index1, sp_index2;
    int8_t *rs1_start, *rs1_pad;
    int8_t *left_val, *row_val, *ker_val;
    int8_t *col_val;
    int8_t *start;
    int32_t val;

    //get the padding
    pad_top = (ss->conv_padding >> 24) & 0xff;
    pad_bottom = (ss->conv_padding >> 16) & 0xff;
    pad_left = (ss->conv_padding >> 8) & 0xff;
    pad_right = ss->conv_padding & 0xff;

    //get the input shape
    in_w = (ss->conv_fm_in >> 16) & 0xffff;
    in_h = (ss->conv_fm_in) & 0xffff;
    in_c = (ss->conv_cin) & 0xffff;
    assert(in_w > 0 && in_h > 0 && in_c > 0);
    in_stride = (ss->conv_cin >> 16) & 0xffff;
    in_stride = in_stride > 0 ? in_stride : in_c;

    //get the output shape
    out_w = (ss->conv_fm_out >> 16) & 0xffff;
    out_h = (ss->conv_fm_out) & 0xffff;
    out_c = (ss->conv_cout) & 0xffff;
    assert(out_w > 0 && out_h > 0 && out_c > 0);
    out_stride = (ss->conv_cout >> 16) & 0xffff;
    out_stride = out_stride > 0 ? out_stride : out_c;

    //get the kernel shape
    kw = (ss->conv_kernel_params1 >> 24) & 0xff;
    kh = (ss->conv_kernel_params1 >> 16) & 0xff;
    stride_h = (ss->conv_kernel_params1) & 0xff;
    stride_w = (ss-> conv_kernel_params2 >> 16) & 0xff;
    assert(kw > 0 && kh > 0 && stride_h > 0 && stride_w > 0);
    k_stride = ss->conv_kernel_params2 & 0xffff;
    k_stride = k_stride > 0 ? k_stride : out_c;

    //get de/quant coeff
    float16_t quant_coeff = f32_to_f16(ss->mme_quant_coeff);
    float16_t dequant_coeff = f32_to_f16(ss->mme_dequant_coeff);

    /*calculate & pad the rs1 shape*/
    in_pad_top = kh - pad_top - 1;
    in_pad_bottom = kh - pad_bottom - 1;
    in_pad_left = kw - pad_left - 1;
    in_pad_right = kw - pad_right - 1;
    Map_int8_t rs1_matrix(rs1, in_h * in_w, in_c, DynStride(in_stride, 1));
    row = in_h + (in_h - 1) * (stride_h - 1) + in_pad_top + in_pad_bottom;
    col = in_w + (in_w - 1) * (stride_w - 1) + in_pad_right + in_pad_left;
    rs1_pad = (int8_t *)malloc(row * col * in_c * sizeof(int8_t));
    Map_int8_t rs1_pad_matrix(rs1_pad, row * col, in_c, DynStride(in_c, 1));
    for (i = 0; i < row * col * in_c; i++)
        (*(rs1_pad+i)) = 0;
    ii=0;
    for (i = in_pad_top; i < (row-in_pad_bottom); i+=stride_h) {
        for (j = in_pad_left; j < (col-in_pad_right); j+=stride_w){
            for (k = 0; k < in_c; k++)
                rs1_pad_matrix(i*col+j, k) = rs1_matrix(ii,k);
            ++ii;
        }
    }

    Map_int8_t rs2_matrix(rs2, kh * kw * in_c, out_c, DynStride(k_stride, 1));

    /* calculate the output shape;
       out_h = row + kernel_h -1; 
       out_w = col + kernel - 1
    */
    h = (in_h - 1) * stride_h - pad_top - pad_bottom + kh;
    w = (in_w - 1) * stride_w - pad_left - pad_right + kw;
    assert(h==out_h && w==out_w);
    left_val = (int8_t *)malloc(h * w * kh * kw * in_c * sizeof(int8_t));
    for (i = 0; i < h; i++){
        for (j = 0; j < w; j++){
            start = left_val + i * w * kh * kw * in_c + j * kh * kw * in_c; 
            rs1_start = rs1_pad;

            for (ii = 0; ii < kh; ii++){
                for (jj = 0; jj < kw; jj++){
                    row_pad = i + ii;
                    col_pad = j + jj;
                    for (kk = 0; kk < in_c; kk++){
                        int start_offset = ii * kw * in_c + jj * in_c + kk;
                        int rs1_offset = row_pad * col * in_c + col_pad * in_c + kk;
                        *(start + start_offset) = *(rs1_start + rs1_offset);
                        val = *(rs1_start + rs1_offset);
                    }
                }
            }
        }
    }

    /* calc the convolution*/
    Map_int8_t left_matrix(left_val, h * w, kh * kw * in_c, DynStride(kh * kw * in_c, 1));
    Map_half rd_matrix(rd, out_h * out_w, out_c, DynStride(out_stride, 1));
    row_val = (int8_t*)malloc(kh * kw * in_c * sizeof(int8_t));
    col_val = (int8_t*)malloc(kh * kw * in_c * sizeof(int8_t));
    Map_int8_t row_matrix(row_val, 1, kh * kw * in_c, DynStride(kh * kw * in_c, 1));
    Map_int8_t col_matrix(col_val, 1, kh * kw * in_c, DynStride(kh * kw * in_c, 1));
    int32_t res;

    for (i = 0; i < out_h * out_w; i++){
        for (j = 0; j < out_c; j++){
            row_matrix = left_matrix.row(i);
            col_matrix = rs2_matrix.col(j).transpose();
            res = 0;
            for (k = 0; k < kh * kw * in_c; k++){
                res += (int32_t)(row_matrix(0, k) * col_matrix(0, k));
            }
            rd_matrix(i, j) = int32_mul_f16(res, dequant_coeff); 
        } 
    }

    free(row_val);
    free(col_val);
    free(left_val);
    free(rs1_pad);
    free(ker_val);
    return 0;
}

int CustomInsns::medeconv_mm(float32_t *rs1, float32_t *rs2, float32_t *rd, struct ConvShapeStride *ss)
{
    int pad_top, pad_bottom, pad_left, pad_right;
    int in_pad_top, in_pad_bottom, in_pad_left, in_pad_right;
    int kw, kh, k_stride, stride_idx;
    int in_w, in_h, in_c, in_stride;
    int out_w, out_h, out_c, out_stride;
    int w, h, c;
    int stride_w, stride_h;
    int i, j, k, m, ii, jj, kk, index_cin, counter;
    int row, col, row_pad, col_pad;
    int rs2_offset, ker_offset;
    uint32_t sp_index1, sp_index2;
    float32_t *rs1_start, *rs1_pad;
    float32_t *left_val, *row_val, *col_val, *ker_val;
    float32_t *start;
    float32_t val;

    //get the padding
    pad_top = (ss->conv_padding >> 24) & 0xff;
    pad_bottom = (ss->conv_padding >> 16) & 0xff;
    pad_left = (ss->conv_padding >> 8) & 0xff;
    pad_right = ss->conv_padding & 0xff;

    //get the input shape
    in_w = (ss->conv_fm_in >> 16) & 0xffff;
    in_h = (ss->conv_fm_in) & 0xffff;
    in_c = (ss->conv_cin) & 0xffff;
    assert(in_w > 0 && in_h > 0 && in_c > 0);
    in_stride = (ss->conv_cin >> 16) & 0xffff;
    in_stride = in_stride > 0 ? in_stride : in_c;

    //get the output shape
    out_w = (ss->conv_fm_out >> 16) & 0xffff;
    out_h = (ss->conv_fm_out) & 0xffff;
    out_c = (ss->conv_cout) & 0xffff;
    assert(out_w > 0 && out_h > 0 && out_c > 0);
    out_stride = (ss->conv_cout >> 16) & 0xffff;
    out_stride = out_stride > 0 ? out_stride : out_c;

    //get the kernel shape
    kw = (ss->conv_kernel_params1 >> 24) & 0xff;
    kh = (ss->conv_kernel_params1 >> 16) & 0xff;
    stride_h = (ss->conv_kernel_params1) & 0xff;
    stride_w = (ss-> conv_kernel_params2 >> 16) & 0xff;
    assert(kw > 0 && kh > 0 && stride_h > 0 && stride_w > 0);
    k_stride = ss->conv_kernel_params2 & 0xffff;
    k_stride = k_stride > 0 ? k_stride : out_c;

    /*calculate & pad the rs1 shape*/
    in_pad_top = kh - pad_top - 1;
    in_pad_bottom = kh - pad_bottom - 1;
    in_pad_left = kw - pad_left - 1;
    in_pad_right = kw - pad_right - 1;
    Map_float32_t rs1_matrix(rs1, in_h * in_w, in_c, DynStride(in_stride, 1));
    row = in_h + (in_h - 1) * (stride_h - 1) + in_pad_top + in_pad_bottom;
    col = in_w + (in_w - 1) * (stride_w - 1) + in_pad_right + in_pad_left;
    rs1_pad = (float32_t *)malloc(row * col * in_c * sizeof(float32_t));
    Map_float32_t rs1_pad_matrix(rs1_pad, row * col, in_c, DynStride(in_c, 1));
    for (i = 0; i < row * col; i++)
        for (j = 0; j < in_c; j++)
            rs1_pad_matrix(i, j) = i32_to_f32(0);
    ii=0;
    for (i = in_pad_top; i < (row-in_pad_bottom); i+=stride_h) {
        for (j = in_pad_left; j < (col-in_pad_right); j+=stride_w){
            for (k = 0; k < in_c; k++){
                rs1_pad_matrix(i*col+j, k) = rs1_matrix(ii,k);
            }
            ++ii;
        }
    }

    Map_float32_t rs2_matrix(rs2, kh * kw * in_c, out_c, DynStride(k_stride, 1));

    /* calculate the output shape;
       out_h = row + kernel_h -1; 
       out_w = col + kernel - 1
    */
    h = (in_h - 1) * stride_h - pad_top - pad_bottom + kh;
    w = (in_w - 1) * stride_w - pad_left - pad_right + kw;
    if (debug){
        MECONV_INFO(ss);
        printf("in_h = %d, in_w = %d, in_c = %d\n", in_h, in_w, in_c);
        printf("pad_t");
        printf("h = %d w = %d out_h = %d out_w = %d\n", h, w, out_h, out_w);
    }
    assert(h==out_h && w==out_w);
    left_val = (float32_t *)malloc(h * w * kh * kw * in_c * sizeof(float32_t));
    for (i = 0; i < h; i++){
        for (j = 0; j < w; j++){
            start = left_val + i * w * kh * kw * in_c + j * kh * kw * in_c; 
            rs1_start = rs1_pad;

            for (ii = 0; ii < kh; ii++){
                for (jj = 0; jj < kw; jj++){
                    row_pad = i + ii;
                    col_pad = j + jj;
                    for (kk = 0; kk < in_c; kk++){
                        int start_offset = ii * kw * in_c + jj * in_c + kk;
                        int rs1_offset = row_pad * col * in_c + col_pad * in_c + kk;
                        *(start + start_offset) = *(rs1_start + rs1_offset);
                        val = *(rs1_start + rs1_offset);

                    }
                }
            }
        }
    }

    /* calc the convolution*/
    Map_float32_t left_matrix(left_val, h * w, kh * kw * in_c, DynStride(kh * kw * in_c, 1));
    Map_float32_t rd_matrix(rd, out_h * out_w, out_c, DynStride(out_stride, 1));
    row_val = (float32_t *)malloc(kh * kw * in_c * sizeof(float32_t));
    col_val = (float32_t *)malloc(kh * kw * in_c * sizeof(float32_t));
    Map_float32_t row_matrix(row_val, 1, kh * kw * in_c, DynStride(kh * kw * in_c, 1));
    Map_float32_t col_matrix(col_val, 1, kh * kw * in_c, DynStride(kh * kw * in_c, 1));
    float32_t odd, even, first, second, third, forth, res12, res34, res_tmp;

    for (i = 0; i < out_h * out_w; i++){
        for (j = 0; j < out_c; j++){
            row_matrix = left_matrix.row(i);
            col_matrix = rs2_matrix.col(j).transpose();
            odd.v = 0x80000000;
            even.v = 0x80000000;
            first.v = 0x80000000;
            second.v = 0x80000000;
            third.v = 0x80000000;
            forth.v = 0x80000000;
            counter = 0;
            if ((k_stride == out_c ) && (out_c <= 32)){
                for (k = 0; k < (kh * kw); k++){
                    int offset = k * in_c;
                    for (index_cin = 0; index_cin < in_c; index_cin++){
                        res_tmp = tf32_mul(row_matrix(0, offset + index_cin), 
                                           col_matrix(0, offset + index_cin));
                        if(counter%4 == 0) first = f32_add(res_tmp, first);
                        else if(counter%4 == 1) second = f32_add(res_tmp, second);
                        else if(counter%4 == 2) third = f32_add(res_tmp, third);
                        else if(counter%4 == 3) forth = f32_add(res_tmp, forth);
                        ++counter;
                        if ((in_c%2) && (index_cin == (in_c - 1))) 
                            ++counter;
                    }
                }
                res12 = f32_add(first, third);
                res34 = f32_add(second,forth);
                rd_matrix(i, j) = f32_add(res12, res34);
            } else { 
                for (k = 0; k < kh * kw * in_c; k++){
                    if (! (k % 2))
                        even = f32_add(tf32_mul(row_matrix(0, k), col_matrix(0, k)), even);
                    else
                        odd = f32_add(tf32_mul(row_matrix(0, k), col_matrix(0, k)), odd);
                }
                rd_matrix(i, j) = f32_add(odd, even);
            }
        } 
    }


    free(row_val);
    free(col_val);
    free(left_val);
    free(rs1_pad);
    return 0;
}








int CustomInsns::medeconv_sp_mm(half *rs1, half *rs2, uint8_t *sparseidx, half *rd, struct ConvShapeStride *ss)
{
    int pad_top, pad_bottom, pad_left, pad_right;
    int in_pad_top, in_pad_bottom, in_pad_left, in_pad_right;
    int kw, kh, k_stride, stride_idx;
    int in_w, in_h, in_c, ker_c, in_stride;
    int out_w, out_h, out_c, out_stride;
    int w, h, c;
    int stride_w, stride_h;
    int i, j, k, m, ii, jj, kk, index_cin, counter;
    int row, col, row_pad, col_pad;
    int rs2_offset, ker_offset, idx_offset;
    uint32_t sp_index1, sp_index2;
    half *rs1_start, *rs1_pad;
    half *left_val, *row_val, *col_val, *ker_val;
    half *start;
    half val;
    uint8_t *sp_idx_data, *idx_val, *sp_val;

    //get the padding
    pad_top = (ss->conv_padding >> 24) & 0xff;
    pad_bottom = (ss->conv_padding >> 16) & 0xff;
    pad_left = (ss->conv_padding >> 8) & 0xff;
    pad_right = ss->conv_padding & 0xff;

    //get the input shape
    in_w = (ss->conv_fm_in >> 16) & 0xffff;
    in_h = (ss->conv_fm_in) & 0xffff;
    in_c = (ss->conv_cin) & 0xffff;
    assert(in_w > 0 && in_h > 0 && in_c > 0);
    in_stride = (ss->conv_cin >> 16) & 0xffff;
    in_stride = in_stride > 0 ? in_stride : in_c;

    //get the output shape
    out_w = (ss->conv_fm_out >> 16) & 0xffff;
    out_h = (ss->conv_fm_out) & 0xffff;
    out_c = (ss->conv_cout) & 0xffff;
    assert(out_w > 0 && out_h > 0 && out_c > 0);
    out_stride = (ss->conv_cout >> 16) & 0xffff;
    out_stride = out_stride > 0 ? out_stride : out_c;

    //get the index stride
    stride_idx = ss->stride_idx? ss->stride_idx : out_c;

    //get the kernel shape
    kw = (ss->conv_kernel_params1 >> 24) & 0xff;
    kh = (ss->conv_kernel_params1 >> 16) & 0xff;
    stride_h = (ss->conv_kernel_params1) & 0xff;
    stride_w = (ss-> conv_kernel_params2 >> 16) & 0xff;
    assert(kw > 0 && kh > 0 && stride_h > 0 && stride_w > 0);
    k_stride = ss->conv_kernel_params2 & 0xffff;
    k_stride = k_stride > 0 ? k_stride : out_c;

    /*calculate & pad the rs1 shape*/
    in_pad_top = kh - pad_top - 1;
    in_pad_bottom = kh - pad_bottom - 1;
    in_pad_left = kw - pad_left - 1;
    in_pad_right = kw - pad_right - 1;
    Map_half rs1_matrix(rs1, in_h * in_w, in_c, DynStride(in_stride, 1));
    row = in_h + (in_h - 1) * (stride_h - 1) + in_pad_top + in_pad_bottom;
    col = in_w + (in_w - 1) * (stride_w - 1) + in_pad_right + in_pad_left;
    rs1_pad = (half *)malloc(row * col * in_c *sizeof(half));
    Map_half rs1_pad_matrix(rs1_pad, row * col, in_c, DynStride(in_c, 1));
    for (i = 0; i < row * col * in_c; i++)
        (*(rs1_pad+i)).x = 0.0;
    ii=0;
    for (i = in_pad_top; i < (row-in_pad_bottom); i+=stride_h) {
        for (j = in_pad_left; j < (col-in_pad_right); j+=stride_w){
            for (k = 0; k < in_c; k++)
                rs1_pad_matrix(i*col+j, k) = rs1_matrix(ii,k);
            ++ii;
        }
    }

    ker_c = in_c / 2;
    
    /* 1. split the 8bit index shape into 2bit */
    i = (kw * kh * in_c / 2 * stride_idx + 3)/4;
    Map_uint8_t tmp_matrix(sparseidx, 1, i, DynStride(i, 1));
    sp_idx_data = (uint8_t *)malloc(kw * kh * out_c * ker_c * sizeof(uint8_t));
    Map_uint8_t sp_matrix(sp_idx_data, kh * kw * ker_c, out_c, DynStride(out_c, 1));
    ii = 0;
    for (i = 0; i < kw * kh * ker_c; i++){
        for (j = 0; j < out_c; j++) {
            sp_matrix(i, j) = (tmp_matrix(0, ii/4) >> (ii%4 *2)) &3;
            ++ii;
        }
        for (j = 0; j < (stride_idx - out_c); j++)
            ++ii;
    }

    /* 2. kernel&sp_idx: kh, kw方向上下左右翻转， cin，cout方向做转置*/
    // ker_val = (half *)malloc(kh * kw * ker_c * out_c * sizeof(half));
    // sp_val = (uint8_t *)malloc(kh * kw * ker_c * out_c * sizeof(uint8_t));
    // for (i = 0; i < kh; i++){
    //     for (j = 0; j < kw; j++){
    //         for (k = 0; k < out_c; k++){
    //             for (m = 0; m < ker_c; m++){
    //                 rs2_offset = i * kw * k_stride * out_c + j * k_stride * out_c + k * k_stride + m;
    //                 idx_offset = i * kw * ker_c * out_c + j * ker_c * out_c + k * ker_c + m;
    //                 ker_offset = (kh-i-1) * kw * ker_c * out_c + (kw-j-1) * ker_c * out_c + m * out_c + k;
    //                 *(ker_val + ker_offset) = *(rs2 + rs2_offset);
    //                 *(sp_val + ker_offset) = *(sp_idx_data + idx_offset);
    //             }
    //         }
    //     }
    // }
    Map_half rs2_matrix(rs2, kh * kw * ker_c, out_c, DynStride(k_stride, 1));    
    // pad_vs2
    half *rs2_pad = (half*)malloc(kh * kw * in_c * out_c * sizeof(half));
    for (i = 0; i < kh * kw * in_c; i++){
        for (j = 0; j < out_c; j++){
            *(rs2_pad + i*out_c + j) = (half)0;
        }
    }
    Map_half rs2_pad_matrix(rs2_pad, kh * kw * in_c, out_c, DynStride(out_c, 1));
    for (i = 0; i < kh * kw * in_c; i+=4){
        for (j = 0; j < out_c; j++){
            sp_index1 = sp_matrix(i/2, j);
            sp_index2 = sp_matrix(i/2+1, j);
            rs2_pad_matrix(i+sp_index1, j) = rs2_matrix(i/2, j);
            rs2_pad_matrix(i+sp_index2, j) = rs2_matrix(i/2+1, j);
        }
    }

    /* 3. calculate the output shape;
       out_h = row + kernel_h -1; 
       out_w = col + kernel - 1
    */
    h = (in_h - 1) * stride_h - pad_top - pad_bottom + kh;
    w = (in_w - 1) * stride_w - pad_left - pad_right + kw;
    if (debug){
        MECONV_INFO(ss);
        printf("in_h = %d, in_w = %d, in_c = %d\n", in_h, in_w, in_c);
        printf("pad_t");
        printf("h = %d w = %d out_h = %d out_w = %d\n", h, w, out_h, out_w);
    }
    assert(h==out_h && w==out_w);
    left_val = (half *)malloc(h * w * kh * kw * in_c * sizeof(half));
    for (i = 0; i < h; i++){
        for (j = 0; j < w; j++){
            start = left_val + i * w * kh * kw * in_c + j * kh * kw * in_c; 
            rs1_start = rs1_pad;

            for (ii = 0; ii < kh; ii++){
                for (jj = 0; jj < kw; jj++){
                    row_pad = i + ii;
                    col_pad = j + jj;
                    for (kk = 0; kk < in_c; kk++){
                        int start_offset = ii * kw * in_c + jj * in_c + kk;
                        int rs1_offset = row_pad * col * in_c + col_pad * in_c + kk;
                        *(start + start_offset) = *(rs1_start + rs1_offset);
                        val = *(rs1_start + rs1_offset);

                        if (debug) {
                            printf("rs1 offset= %d val = 0x%x ",start_offset, val.x);
                            val = *(start + start_offset);
                            printf("left val = 0x%x\n", val.x); 
                        }
                    }
                }
            }
        }
    }

    /* 4. calc the convolution*/
    Map_half left_matrix(left_val, h * w, kh * kw * in_c, DynStride(kh * kw * in_c, 1));
    Map_half rd_matrix(rd, out_h * out_w, out_c, DynStride(out_stride, 1));
    row_val = (half *)malloc(kh * kw * in_c * sizeof(half));
    col_val = (half *)malloc(kh * kw * in_c/2 * sizeof(half));
    idx_val = (uint8_t *)malloc(kh * kw * in_c/2 * sizeof(uint8_t));
    Map_half row_matrix(row_val, 1, kh * kw * in_c, DynStride(kh * kw * in_c, 1));
    Map_half col_matrix(col_val, 1, kh * kw * in_c /2, DynStride(kh * kw * in_c/2, 1));
    Map_uint8_t idx_matrix(idx_val, 1, kh * kw * in_c / 2, DynStride(kh * kw * in_c/2, 1));
    float32_t odd, even;
    if (debug) {
        cout << "rs1: " << rs1_matrix << endl;
        cout << "rs1_extend: " << rs1_pad_matrix << endl;
        cout << "left: " << left_matrix << endl;
        cout << "rs2: " << rs2_matrix << endl;
    }

    for (i = 0; i < out_h * out_w; i++) {
        for (j = 0; j < out_c; j++) {
            row_matrix = left_matrix.row(i);
            col_matrix = rs2_matrix.col(j).transpose();
            idx_matrix = sp_matrix.col(j).transpose();
            odd.v = 0x80000000;
            even.v = 0x80000000;
            
            for (k = 0; k < kh * kw * in_c; k+=4) {
                sp_index1 = idx_matrix(0, k/2);
                sp_index2 = idx_matrix(0, k/2+1);
                even = f32_add(half_mul_f32(row_matrix(0, k+sp_index1), col_matrix(0, k/2)), even);
                odd = f32_add(half_mul_f32(row_matrix(0, k+sp_index2), col_matrix(0, k/2+1)), odd);
                if (debug) {
                    cout << "index: " << (int)sp_index1 << " " << (int)sp_index2 << endl;
                    cout << row_matrix(0, k+sp_index1) << " * " << col_matrix(0, k/2) << " = ";
                    cout << hex << even.v << endl;
                    cout << row_matrix(0, k+sp_index2) << " * " << col_matrix(0, k/2+1) << " = ";
                    cout << hex << odd.v << endl;
                }
            }
            rd_matrix(i, j) = f32_to_half(f32_add(even, odd));
        }
    }

    if (debug)
        cout << "rd:" << endl << rd_matrix << endl;

    free(row_val);
    free(col_val);
    free(left_val);
    free(rs1_pad);
    free(ker_val);
    free(idx_val);
    free(sp_idx_data);
    free(sp_val);
    return 0;
}

int CustomInsns::medeconv_sp_mm(half *rs1, int8_t *rs2, uint8_t *sparseidx, half *rd, struct ConvShapeStride *ss)
{
    int pad_top, pad_bottom, pad_left, pad_right;
    int in_pad_top, in_pad_bottom, in_pad_left, in_pad_right;
    int kw, kh, k_stride, stride_idx;
    int in_w, in_h, in_c, ker_c, in_stride;
    int out_w, out_h, out_c, out_stride;
    int w, h, c;
    int stride_w, stride_h;
    int i, j, k, m, ii, jj, kk;
    int row, col, row_pad, col_pad;
    int rs2_offset, ker_offset, idx_offset;
    uint32_t sp_index1, sp_index2;
    half *rs1_start, *rs1_pad;
    half *left_val, *row_val, *ker_val;
    int8_t *col_val;
    half *start;
    half val;
    uint8_t *sp_idx_data, *idx_val, *sp_val;

    //get the padding
    pad_top = (ss->conv_padding >> 24) & 0xff;
    pad_bottom = (ss->conv_padding >> 16) & 0xff;
    pad_left = (ss->conv_padding >> 8) & 0xff;
    pad_right = ss->conv_padding & 0xff;

    //get the input shape
    in_w = (ss->conv_fm_in >> 16) & 0xffff;
    in_h = (ss->conv_fm_in) & 0xffff;
    in_c = (ss->conv_cin) & 0xffff;
    assert(in_w > 0 && in_h > 0 && in_c > 0);
    in_stride = (ss->conv_cin >> 16) & 0xffff;
    in_stride = in_stride > 0 ? in_stride : in_c;

    //get the output shape
    out_w = (ss->conv_fm_out >> 16) & 0xffff;
    out_h = (ss->conv_fm_out) & 0xffff;
    out_c = (ss->conv_cout) & 0xffff;
    assert(out_w > 0 && out_h > 0 && out_c > 0);
    out_stride = (ss->conv_cout >> 16) & 0xffff;
    out_stride = out_stride > 0 ? out_stride : out_c;

    //get the index stride
    stride_idx = ss->stride_idx? ss->stride_idx : out_c;

    //get the kernel shape
    kw = (ss->conv_kernel_params1 >> 24) & 0xff;
    kh = (ss->conv_kernel_params1 >> 16) & 0xff;
    stride_h = (ss->conv_kernel_params1) & 0xff;
    stride_w = (ss-> conv_kernel_params2 >> 16) & 0xff;
    assert(kw > 0 && kh > 0 && stride_h > 0 && stride_w > 0);
    k_stride = ss->conv_kernel_params2 & 0xffff;
    k_stride = k_stride > 0 ? k_stride : out_c;
    
    //get de/quant coeff
    float16_t quant_coeff = f32_to_f16(ss->mme_quant_coeff);
    float16_t dequant_coeff = f32_to_f16(ss->mme_dequant_coeff);
    
    /*calculate & pad the rs1 shape*/
    in_pad_top = kh - pad_top - 1;
    in_pad_bottom = kh - pad_bottom - 1;
    in_pad_left = kw - pad_left - 1;
    in_pad_right = kw - pad_right - 1;
    Map_half rs1_matrix(rs1, in_h * in_w, in_c, DynStride(in_stride, 1));
    row = in_h + (in_h - 1) * (stride_h - 1) + in_pad_top + in_pad_bottom;
    col = in_w + (in_w - 1) * (stride_w - 1) + in_pad_right + in_pad_left;
    rs1_pad = (half *)malloc(row * col * in_c *sizeof(half));
    Map_half rs1_pad_matrix(rs1_pad, row * col, in_c, DynStride(in_c, 1));
    for (i = 0; i < row * col * in_c; i++)
        (*(rs1_pad+i)).x = 0.0;
    ii=0;
    for (i = in_pad_top; i < (row-in_pad_bottom); i+=stride_h) {
        for (j = in_pad_left; j < (col-in_pad_right); j+=stride_w){
            for (k = 0; k < in_c; k++)
                rs1_pad_matrix(i*col+j, k) = rs1_matrix(ii,k);
            ++ii;
        }
    }

    ker_c = in_c / 2;
    
    /* 1. split the 8bit index shape into 2bit */
    i = (kw * kh * in_c / 2 * stride_idx + 3)/4;
    Map_uint8_t tmp_matrix(sparseidx, 1, i, DynStride(i, 1));
    sp_idx_data = (uint8_t *)malloc(kw * kh * out_c * ker_c * sizeof(uint8_t));
    Map_uint8_t sp_matrix(sp_idx_data, kh * kw * ker_c, out_c, DynStride(out_c, 1));
    ii = 0;
    for (i = 0; i < kw * kh * ker_c; i++){
        for (j = 0; j < out_c; j++) {
            sp_matrix(i, j) = (tmp_matrix(0, ii/4) >> (ii%4 *2)) &3;
            ++ii;
        }
        for (j = 0; j < (stride_idx - out_c); j++)
            ++ii;
    }

  
    Map_int8_t rs2_matrix(rs2, kh * kw * ker_c, out_c, DynStride(k_stride, 1));    
    // pad_vs2
    int8_t *rs2_pad = (int8_t*)malloc(kh * kw * in_c * out_c * sizeof(int8_t));
    for (i = 0; i < kh * kw * in_c; i++){
        for (j = 0; j < out_c; j++){
            *(rs2_pad + i*out_c + j) = 0;
        }
    }
    Map_int8_t rs2_pad_matrix(rs2_pad, kh * kw * in_c, out_c, DynStride(out_c, 1));
    for (i = 0; i < kh * kw * in_c; i+=4){
        for (j = 0; j < out_c; j++){
            sp_index1 = sp_matrix(i/2, j);
            sp_index2 = sp_matrix(i/2+1, j);
            rs2_pad_matrix(i+sp_index1, j) = rs2_matrix(i/2, j);
            rs2_pad_matrix(i+sp_index2, j) = rs2_matrix(i/2+1, j);
        }
    }
    /* 3. calculate the output shape;
       out_h = row + kernel_h -1; 
       out_w = col + kernel - 1
    */
    h = (in_h - 1) * stride_h - pad_top - pad_bottom + kh;
    w = (in_w - 1) * stride_w - pad_left - pad_right + kw;
    assert(h==out_h && w==out_w);
    left_val = (half *)malloc(h * w * kh * kw * in_c * sizeof(half));
    for (i = 0; i < h; i++){
        for (j = 0; j < w; j++){
            start = left_val + i * w * kh * kw * in_c + j * kh * kw * in_c; 
            rs1_start = rs1_pad;

            for (ii = 0; ii < kh; ii++){
                for (jj = 0; jj < kw; jj++){
                    row_pad = i + ii;
                    col_pad = j + jj;
                    for (kk = 0; kk < in_c; kk++){
                        int start_offset = ii * kw * in_c + jj * in_c + kk;
                        int rs1_offset = row_pad * col * in_c + col_pad * in_c + kk;
                        *(start + start_offset) = *(rs1_start + rs1_offset);
                        val = *(rs1_start + rs1_offset);

                        if (debug) {
                            printf("rs1 offset= %d val = 0x%x ",start_offset, val.x);
                            val = *(start + start_offset);
                            printf("left val = 0x%x\n", val.x); 
                        }
                    }
                }
            }
        }
    }

    /* 4. calc the convolution*/
    Map_half left_matrix(left_val, h * w, kh * kw * in_c, DynStride(kh * kw * in_c, 1));
    Map_half rd_matrix(rd, out_h * out_w, out_c, DynStride(out_stride, 1));
    row_val = (half *)malloc(kh * kw * in_c * sizeof(half));
    col_val = (int8_t *)malloc(kh * kw * in_c * sizeof(int8_t));
    Map_half row_matrix(row_val, 1, kh * kw * in_c, DynStride(kh * kw * in_c, 1));
    Map_int8_t col_matrix(col_val, 1, kh * kw * in_c, DynStride(kh * kw * in_c, 1));
    int32_t res;
    float16_t rs1_f16;
    int8_t rs1_i8;
    
     for (i = 0; i < out_h * out_w; i++){
        for (j = 0; j < out_c; j++){
            row_matrix = left_matrix.row(i);
            col_matrix = rs2_pad_matrix.col(j).transpose();
            res = 0;
            for (k = 0; k < kh * kw * in_c; k++){
                rs1_f16 = f16_mul(half_to_float16_t(row_matrix(0, k)), quant_coeff);
                rs1_i8 = f16_to_i8(rs1_f16, softfloat_round_near_maxMag, true);
                res += rs1_i8 * col_matrix(0, k);
            }
            rd_matrix(i, j) = int32_mul_f16(res, dequant_coeff); 
            if (debug) cout << "rd: " << hex << rd_matrix(i, j).x << endl;
        } 
    }

    if (debug)
        cout << "rd:" << endl << rd_matrix << endl;

    free(row_val);
    free(col_val);
    free(left_val);
    free(rs1_pad);
    free(ker_val);
    free(idx_val);
    free(sp_idx_data);
    free(sp_val);
    return 0;
}

int CustomInsns::medeconv_sp_mm(int8_t *rs1, int8_t *rs2, uint8_t *sparseidx, half *rd, struct ConvShapeStride *ss)
{
    int pad_top, pad_bottom, pad_left, pad_right;
    int in_pad_top, in_pad_bottom, in_pad_left, in_pad_right;
    int kw, kh, k_stride, stride_idx;
    int in_w, in_h, in_c, ker_c, in_stride;
    int out_w, out_h, out_c, out_stride;
    int w, h, c;
    int stride_w, stride_h;
    int i, j, k, m, ii, jj, kk;
    int row, col, row_pad, col_pad;
    int rs2_offset, ker_offset, idx_offset;
    uint32_t sp_index1, sp_index2;
    int8_t *rs1_start, *rs1_pad;
    int8_t *left_val, *row_val, *ker_val;
    int8_t *col_val;
    int8_t *start;
    int32_t val;
    uint8_t *sp_idx_data, *idx_val, *sp_val;

    //get the padding
    pad_top = (ss->conv_padding >> 24) & 0xff;
    pad_bottom = (ss->conv_padding >> 16) & 0xff;
    pad_left = (ss->conv_padding >> 8) & 0xff;
    pad_right = ss->conv_padding & 0xff;

    //get the input shape
    in_w = (ss->conv_fm_in >> 16) & 0xffff;
    in_h = (ss->conv_fm_in) & 0xffff;
    in_c = (ss->conv_cin) & 0xffff;
    assert(in_w > 0 && in_h > 0 && in_c > 0);
    in_stride = (ss->conv_cin >> 16) & 0xffff;
    in_stride = in_stride > 0 ? in_stride : in_c;

    //get the output shape
    out_w = (ss->conv_fm_out >> 16) & 0xffff;
    out_h = (ss->conv_fm_out) & 0xffff;
    out_c = (ss->conv_cout) & 0xffff;
    assert(out_w > 0 && out_h > 0 && out_c > 0);
    out_stride = (ss->conv_cout >> 16) & 0xffff;
    out_stride = out_stride > 0 ? out_stride : out_c;

    //get the index stride
    stride_idx = ss->stride_idx? ss->stride_idx : out_c;

    //get the kernel shape
    kw = (ss->conv_kernel_params1 >> 24) & 0xff;
    kh = (ss->conv_kernel_params1 >> 16) & 0xff;
    stride_h = (ss->conv_kernel_params1) & 0xff;
    stride_w = (ss-> conv_kernel_params2 >> 16) & 0xff;
    assert(kw > 0 && kh > 0 && stride_h > 0 && stride_w > 0);
    k_stride = ss->conv_kernel_params2 & 0xffff;
    k_stride = k_stride > 0 ? k_stride : out_c;
    
    //get de/quant coeff
    float16_t quant_coeff = f32_to_f16(ss->mme_quant_coeff);
    float16_t dequant_coeff = f32_to_f16(ss->mme_dequant_coeff);
    
    /*calculate & pad the rs1 shape*/
    in_pad_top = kh - pad_top - 1;
    in_pad_bottom = kh - pad_bottom - 1;
    in_pad_left = kw - pad_left - 1;
    in_pad_right = kw - pad_right - 1;
    Map_int8_t rs1_matrix(rs1, in_h * in_w, in_c, DynStride(in_stride, 1));
    row = in_h + (in_h - 1) * (stride_h - 1) + in_pad_top + in_pad_bottom;
    col = in_w + (in_w - 1) * (stride_w - 1) + in_pad_right + in_pad_left;
    rs1_pad = (int8_t *)malloc(row * col * in_c *sizeof(int8_t));
    Map_int8_t rs1_pad_matrix(rs1_pad, row * col, in_c, DynStride(in_c, 1));
    for (i = 0; i < row * col * in_c; i++)
        (*(rs1_pad+i)) = 0;
    ii=0;
    for (i = in_pad_top; i < (row-in_pad_bottom); i+=stride_h) {
        for (j = in_pad_left; j < (col-in_pad_right); j+=stride_w){
            for (k = 0; k < in_c; k++)
                rs1_pad_matrix(i*col+j, k) = rs1_matrix(ii,k);
            ++ii;
        }
    }

    ker_c = in_c / 2;
    
    /* 1. split the 8bit index shape into 2bit */
    i = (kw * kh * in_c / 2 * stride_idx + 3)/4;
    Map_uint8_t tmp_matrix(sparseidx, 1, i, DynStride(i, 1));
    sp_idx_data = (uint8_t *)malloc(kw * kh * out_c * ker_c * sizeof(uint8_t));
    Map_uint8_t sp_matrix(sp_idx_data, kh * kw * ker_c, out_c, DynStride(out_c, 1));
    ii = 0;
    for (i = 0; i < kw * kh * ker_c; i++){
        for (j = 0; j < out_c; j++) {
            sp_matrix(i, j) = (tmp_matrix(0, ii/4) >> (ii%4 *2)) &3;
            ++ii;
        }
        for (j = 0; j < (stride_idx - out_c); j++)
            ++ii;
    }

  
    Map_int8_t rs2_matrix(rs2, kh * kw * ker_c, out_c, DynStride(k_stride, 1));    
    // pad_vs2
    int8_t *rs2_pad = (int8_t*)malloc(kh * kw * in_c * out_c * sizeof(int8_t));
    for (i = 0; i < kh * kw * in_c; i++){
        for (j = 0; j < out_c; j++){
            *(rs2_pad + i*out_c + j) = 0;
        }
    }
    Map_int8_t rs2_pad_matrix(rs2_pad, kh * kw * in_c, out_c, DynStride(out_c, 1));
    for (i = 0; i < kh * kw * in_c; i+=4){
        for (j = 0; j < out_c; j++){
            sp_index1 = sp_matrix(i/2, j);
            sp_index2 = sp_matrix(i/2+1, j);
            rs2_pad_matrix(i+sp_index1, j) = rs2_matrix(i/2, j);
            rs2_pad_matrix(i+sp_index2, j) = rs2_matrix(i/2+1, j);
        }
    }

    /* 3. calculate the output shape;
       out_h = row + kernel_h -1; 
       out_w = col + kernel - 1
    */
    h = (in_h - 1) * stride_h - pad_top - pad_bottom + kh;
    w = (in_w - 1) * stride_w - pad_left - pad_right + kw;
    assert(h==out_h && w==out_w);
    left_val = (int8_t *)malloc(h * w * kh * kw * in_c * sizeof(int8_t));
    for (i = 0; i < h; i++){
        for (j = 0; j < w; j++){
            start = left_val + i * w * kh * kw * in_c + j * kh * kw * in_c; 
            rs1_start = rs1_pad;

            for (ii = 0; ii < kh; ii++){
                for (jj = 0; jj < kw; jj++){
                    row_pad = i + ii;
                    col_pad = j + jj;
                    for (kk = 0; kk < in_c; kk++){
                        int start_offset = ii * kw * in_c + jj * in_c + kk;
                        int rs1_offset = row_pad * col * in_c + col_pad * in_c + kk;
                        *(start + start_offset) = *(rs1_start + rs1_offset);
                        val = *(rs1_start + rs1_offset);
                    }
                }
            }
        }
    }

    /* 4. calc the convolution*/
    Map_int8_t left_matrix(left_val, h * w, kh * kw * in_c, DynStride(kh * kw * in_c, 1));
    Map_half rd_matrix(rd, out_h * out_w, out_c, DynStride(out_stride, 1));
    row_val = (int8_t *)malloc(kh * kw * in_c * sizeof(int8_t));
    col_val = (int8_t *)malloc(kh * kw * in_c * sizeof(int8_t));
    Map_int8_t row_matrix(row_val, 1, kh * kw * in_c, DynStride(kh * kw * in_c, 1));
    Map_int8_t col_matrix(col_val, 1, kh * kw * in_c, DynStride(kh * kw * in_c, 1));
    int32_t res;

    for (i = 0; i < out_h * out_w; i++){
        for (j = 0; j < out_c; j++){
            row_matrix = left_matrix.row(i);
            col_matrix = rs2_pad_matrix.col(j).transpose();
            res = 0;
            for (k = 0; k < kh * kw * in_c; k++){
                res += (int32_t)(row_matrix(0, k) * col_matrix(0, k));
            }
            rd_matrix(i, j) = int32_mul_f16(res, dequant_coeff); 
        } 
    }
    

    free(row_val);
    free(col_val);
    free(left_val);
    free(rs1_pad);
    free(ker_val);
    free(idx_val);
    free(sp_idx_data);
    free(sp_val);
    return 0;
}

int CustomInsns::medeconv_sp_mm(float32_t *rs1, float32_t *rs2, uint8_t *sparseidx, 
                                float32_t *rd, struct ConvShapeStride *ss)
{
    int pad_top, pad_bottom, pad_left, pad_right;
    int in_pad_top, in_pad_bottom, in_pad_left, in_pad_right;
    int kw, kh, k_stride, stride_idx;
    int in_w, in_h, in_c, ker_c, in_stride;
    int out_w, out_h, out_c, out_stride;
    int w, h, c;
    int stride_w, stride_h;
    int i, j, k, m, ii, jj, kk, index_cin, counter;
    int row, col, row_pad, col_pad;
    int rs2_offset, ker_offset, idx_offset;
    uint32_t sp_index1, sp_index2;
    float32_t *rs1_start, *rs1_pad;
    float32_t *left_val, *row_val, *col_val, *ker_val;
    float32_t *start;
    float32_t val;
    uint8_t *sp_idx_data, *idx_val, *sp_val;

    //get the padding
    pad_top = (ss->conv_padding >> 24) & 0xff;
    pad_bottom = (ss->conv_padding >> 16) & 0xff;
    pad_left = (ss->conv_padding >> 8) & 0xff;
    pad_right = ss->conv_padding & 0xff;

    //get the input shape
    in_w = (ss->conv_fm_in >> 16) & 0xffff;
    in_h = (ss->conv_fm_in) & 0xffff;
    in_c = (ss->conv_cin) & 0xffff;
    assert(in_w > 0 && in_h > 0 && in_c > 0);
    in_stride = (ss->conv_cin >> 16) & 0xffff;
    in_stride = in_stride > 0 ? in_stride : in_c;

    //get the output shape
    out_w = (ss->conv_fm_out >> 16) & 0xffff;
    out_h = (ss->conv_fm_out) & 0xffff;
    out_c = (ss->conv_cout) & 0xffff;
    assert(out_w > 0 && out_h > 0 && out_c > 0);
    out_stride = (ss->conv_cout >> 16) & 0xffff;
    out_stride = out_stride > 0 ? out_stride : out_c;

    //get the index stride
    stride_idx = ss->stride_idx? ss->stride_idx : out_c;

    //get the kernel shape
    kw = (ss->conv_kernel_params1 >> 24) & 0xff;
    kh = (ss->conv_kernel_params1 >> 16) & 0xff;
    stride_h = (ss->conv_kernel_params1) & 0xff;
    stride_w = (ss-> conv_kernel_params2 >> 16) & 0xff;
    assert(kw > 0 && kh > 0 && stride_h > 0 && stride_w > 0);
    k_stride = ss->conv_kernel_params2 & 0xffff;
    k_stride = k_stride > 0 ? k_stride : out_c;

    /*calculate & pad the rs1 shape*/
    in_pad_top = kh - pad_top - 1;
    in_pad_bottom = kh - pad_bottom - 1;
    in_pad_left = kw - pad_left - 1;
    in_pad_right = kw - pad_right - 1;
    Map_float32_t rs1_matrix(rs1, in_h * in_w, in_c, DynStride(in_stride, 1));
    row = in_h + (in_h - 1) * (stride_h - 1) + in_pad_top + in_pad_bottom;
    col = in_w + (in_w - 1) * (stride_w - 1) + in_pad_right + in_pad_left;
    rs1_pad = (float32_t *)malloc(row * col * in_c *sizeof(float32_t));
    Map_float32_t rs1_pad_matrix(rs1_pad, row * col, in_c, DynStride(in_c, 1));
    for (i = 0; i < row * col * in_c; i++)
        (*(rs1_pad+i)).v = 0.0;
    ii=0;
    for (i = in_pad_top; i < (row-in_pad_bottom); i+=stride_h) {
        for (j = in_pad_left; j < (col-in_pad_right); j+=stride_w){
            for (k = 0; k < in_c; k++)
                rs1_pad_matrix(i*col+j, k) = rs1_matrix(ii,k);
            ++ii;
        }
    }

    ker_c = in_c / 2;
    
    /* 1. split the 8bit index shape into 2bit */
    i = (kw * kh * in_c / 2 * stride_idx + 3)/4;
    Map_uint8_t tmp_matrix(sparseidx, 1, i, DynStride(i, 1));
    sp_idx_data = (uint8_t *)malloc(kw * kh * out_c * ker_c * sizeof(uint8_t));
    Map_uint8_t sp_matrix(sp_idx_data, kh * kw * ker_c, out_c, DynStride(out_c, 1));
    ii = 0;
    for (i = 0; i < kw * kh * ker_c; i++){
        for (j = 0; j < out_c; j++) {
            sp_matrix(i, j) = (tmp_matrix(0, ii/4) >> (ii%4 *2)) &3;
            ++ii;
        }
        for (j = 0; j < (stride_idx - out_c); j++)
            ++ii;
    }


    Map_float32_t rs2_matrix(rs2, kh * kw * ker_c, out_c, DynStride(k_stride, 1));    
    // pad_vs2
    float32_t *rs2_pad = (float32_t*)malloc(kh * kw * in_c * out_c * sizeof(float32_t));
    for (i = 0; i < kh * kw * in_c; i++){
        for (j = 0; j < out_c; j++){
            *(rs2_pad + i*out_c + j) = i32_to_f32(0);
        }
    }
    Map_float32_t rs2_pad_matrix(rs2_pad, kh * kw * in_c, out_c, DynStride(out_c, 1));
    for (i = 0; i < kh * kw * in_c; i+=4){
        for (j = 0; j < out_c; j++){
            sp_index1 = sp_matrix(i/2, j);
            sp_index2 = sp_matrix(i/2+1, j);
            rs2_pad_matrix(i+sp_index1, j) = rs2_matrix(i/2, j);
            rs2_pad_matrix(i+sp_index2, j) = rs2_matrix(i/2+1, j);
        }
    }
    
    /* 3. calculate the output shape;
       out_h = row + kernel_h -1; 
       out_w = col + kernel - 1
    */
    h = (in_h - 1) * stride_h - pad_top - pad_bottom + kh;
    w = (in_w - 1) * stride_w - pad_left - pad_right + kw;
    assert(h==out_h && w==out_w);
    left_val = (float32_t *)malloc(h * w * kh * kw * in_c * sizeof(float32_t));
    for (i = 0; i < h; i++){
        for (j = 0; j < w; j++){
            start = left_val + i * w * kh * kw * in_c + j * kh * kw * in_c; 
            rs1_start = rs1_pad;

            for (ii = 0; ii < kh; ii++){
                for (jj = 0; jj < kw; jj++){
                    row_pad = i + ii;
                    col_pad = j + jj;
                    for (kk = 0; kk < in_c; kk++){
                        int start_offset = ii * kw * in_c + jj * in_c + kk;
                        int rs1_offset = row_pad * col * in_c + col_pad * in_c + kk;
                        *(start + start_offset) = *(rs1_start + rs1_offset);
                        val = *(rs1_start + rs1_offset);

                    }
                }
            }
        }
    }

    /* 4. calc the convolution*/
    Map_float32_t left_matrix(left_val, h * w, kh * kw * in_c, DynStride(kh * kw * in_c, 1));
    Map_float32_t rd_matrix(rd, out_h * out_w, out_c, DynStride(out_stride, 1));
    row_val = (float32_t *)malloc(kh * kw * in_c *  sizeof(float32_t));
    col_val = (float32_t *)malloc(kh * kw * in_c/2 * sizeof(float32_t));
    idx_val = (uint8_t *)malloc(kh * kw * in_c/2 * sizeof(uint8_t));
    Map_float32_t row_matrix(row_val, 1, kh * kw * in_c, DynStride(kh * kw * in_c, 1));
    Map_float32_t col_matrix(col_val, 1, kh * kw * in_c/2, DynStride(kh * kw * in_c/2, 1));
    Map_uint8_t idx_matrix(idx_val, 1, kh * kw * in_c / 2, DynStride(kh * kw * in_c/2, 1));
    float32_t odd, even;


    for (i = 0; i < out_h * out_w; i++) {
        for (j = 0; j < out_c; j++) {
            row_matrix = left_matrix.row(i);
            col_matrix = rs2_matrix.col(j).transpose();
            idx_matrix = sp_matrix.col(j).transpose();
            even.v = 0x80000000;
            odd.v = 0x80000000;
            
            for (k = 0; k < kh * kw * in_c; k+=4) {
                sp_index1 = idx_matrix(0, k/2);
                sp_index2 = idx_matrix(0, k/2+1);
                even = f32_add(tf32_mul(row_matrix(0, k+sp_index1), col_matrix(0, k/2)), even);
                odd = f32_add(tf32_mul(row_matrix(0, k+sp_index2), col_matrix(0, k/2+1)), odd);
            }
            rd_matrix(i, j) = f32_add(even, odd);
        }
    }

    free(row_val);
    free(col_val);
    free(left_val);
    free(rs1_pad);
    free(ker_val);
    free(idx_val);
    free(sp_idx_data);
    free(sp_val);
    return 0;
}



/**
 * medwconv_mm() medwconv.mm
 *
 * 标量和矩阵元素广播乘 M = M1 * f
 * @param rs1 M1,源操作矩阵基地址
 * @param rd M,目的矩阵基地址
 * @param rs2 f,源标量操作数
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
int CustomInsns::medwconv_mm(half *rs1, half *rd, half *rs2, struct ConvShapeStride *ss)
{
    int pad_top, pad_bottom, pad_left, pad_right;
    int kw, kh, okw, okh, k_stride, sk_h, sk_w;
    int in_w, in_h, in_c, in_stride;
    int out_w, out_h, out_stride;
    int w, h, c;
    int dilation_h, dilation_w;
    int i, j, k, ii, jj, kk, index_cin, counter;
    int row, col;
    half *rs1_start;
    half *left_val, *row_val, *col_val;
    half *start;
    half val;

    //get the padding
    pad_top = (ss->conv_padding >> 24) & 0xff;
    pad_bottom = (ss->conv_padding >> 16) & 0xff;
    pad_left = (ss->conv_padding >> 8) & 0xff;
    pad_right = ss->conv_padding & 0xff;

    //get the input shape
    in_w = (ss->conv_fm_in >> 16) & 0xffff;
    in_h = (ss->conv_fm_in) & 0xffff;
    in_c = (ss->conv_cin) & 0xffff;
    assert(in_w > 0 && in_h > 0 && in_c > 0);
    in_stride = (ss->conv_cin >> 16) & 0xffff;
    in_stride = in_stride > 0 ? in_stride : in_c;

    //get the output shape
    out_w = (ss->conv_fm_out >> 16) & 0xffff;
    out_h = (ss->conv_fm_out) & 0xffff;
    assert(out_w > 0 && out_h > 0);
    out_stride = (ss->conv_cout >> 16) & 0xffff;
    out_stride = out_stride > 0 ? out_stride : in_c;

    //get the kernel shape
    kw = (ss->conv_kernel_params1 >> 24) & 0xff;
    kh = (ss->conv_kernel_params1 >> 16) & 0xff;
    dilation_h = (ss->conv_kernel_params1 >> 8) & 0xff;
    dilation_w = (ss->conv_kernel_params2 >> 24) & 0xff;
    sk_h = (ss->conv_kernel_params1) & 0xff;
    sk_w = (ss->conv_kernel_params2 >> 16) & 0xff;
    dilation_h = dilation_h == 0? 1 : dilation_h;
    dilation_w = dilation_w == 0? dilation_h : dilation_w;
    sk_h = sk_h == 0? 1 : sk_h;
    sk_w = sk_w == 0? sk_h : sk_w;
    assert(kw > 0 && kh > 0);
    assert(dilation_h > 0 && dilation_w > 0 && sk_h >0 && sk_w > 0);
    k_stride = ss->conv_kernel_params2 & 0xffff;
    k_stride = k_stride > 0 ? k_stride : in_c;

    /*calculate the kernel shape*/
    Map_half rs2_matrix(rs2, kh * kw, in_c, DynStride(k_stride, 1)); // the depth is same as in_c

    h = dilation_h > 1 ? dilation_h * (kh - 1) + 1 : kh;
    w = dilation_w > 1 ? dilation_w * (kw - 1) + 1 : kw;
    okh = kh;
    okw = kw;
    kh = h;
    kw = w;

    /*calculate the input shape*/
    Map_half rs1_matrix(rs1, in_h * in_w, in_c, DynStride(in_stride, 1));

    if (debug) {
        MECONV_INFO(ss);
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rs2:" << endl << rs2_matrix << endl;
    }

    /*calculate the output shape*/
    h = (in_h + pad_top + pad_bottom - kh + 1 + sk_h - 1) / sk_h;
    w = (in_w + pad_left + pad_right - kw + 1 + sk_w - 1) / sk_w;
    left_val = (half *)malloc(h * w * okh * okw * in_c * sizeof(half));

    for (i = 0; i < h; i++) {
        for (j = 0; j < w; j++) {
            start = left_val + i * w * okh * okw * in_c + j * okh * okw * in_c;
            rs1_start = rs1;

            for (ii = 0; ii < kh; ii++) {
                for (jj = 0; jj < kw; jj++) {
                    for (kk = 0; kk < in_c; kk++) {
                        row = i * sk_h + ii;
                        col = j * sk_w + jj;

                        if (ii % dilation_h)
                            continue;
                        else {
                            if (jj % dilation_w)
                                continue;
                            else {
				int start_offset = ii/dilation_h * okw * in_c + jj/dilation_w * in_c + kk;
				int rs1_offset = (row - pad_top) * in_w * in_stride + (col - pad_left) * in_stride + kk;
                                if (row >= pad_top && row < pad_top + in_h && col >= pad_left && col < pad_left + in_w) {
                                    *(start + start_offset) = *(rs1_start + rs1_offset);
                                    val = *(rs1_start + rs1_offset);
                                }
                                else {
                                    *(start + start_offset) = (half)0;
                                    val =  (half)0;
                                }

                                if (debug) {
                                    printf("rd offset= %d val = 0x%x ",start_offset, val.x);
                                    val = *(start + start_offset);
                                    printf("rd val = 0x%x\n", val.x);
                                }
                            }//jj % dilation_w
                        }//ii % dilation_h
                    }//kk
                }//jj
            }//ii
        }//j
    }//i

    /*param check*/
    if (debug)
        printf("h = %d w = %d out_c = %d\n", h, w, in_c);
    assert(h == out_h && w == out_w);

    /*calculate convolution*/
    Map_half left_matrix(left_val, h * w, okh * okw * in_c, DynStride(okh * okw * in_c, 1));
    Map_half rd_matrix(rd, out_h * out_w, in_c, DynStride(out_stride, 1));
    float32_t first, second, third, forth, res12, res34, res;
    float32_t res_tmp;
    if (debug) {
        cout << "rs1: " << rs1_matrix << endl;
        cout << "left: " << left_matrix << endl;
        cout << "rs2: " << rs2_matrix << endl;
    }
    //rd_matrix = left_matrix * rs2_matrix;
    for (i = 0; i < out_h * out_w; i++) {
        for (j = 0; j < in_c; j++) {
            first.v = 0x80000000;
            for (k = 0; k < okh * okw; k++) {
                first = f32_add(half_mul_f32(left_matrix(i, k*in_c+j), rs2_matrix(k, j)), first);
                if (debug)
                    cout << left_matrix(i, k*in_c+j) << " * " << rs2_matrix(k, j) << endl;
            }
            rd_matrix(i, j) = f32_to_half(first);
        }
    }

    if (debug)
        cout << "rd:" << endl << rd_matrix << endl;

    free(row_val);
    free(col_val);
    free(left_val);
    return 0;
}

/**
 * medwconv_mm() medwconv.mm
 *
 * 标量和矩阵元素广播乘 M = M1 * f
 * @param rs1 M1,源操作矩阵基地址
 * @param rd M,目的矩阵基地址
 * @param rs2 f,源标量操作数
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
int CustomInsns::medwconv_mm(half *rs1, half *rd, int8_t *rs2, struct ConvShapeStride *ss)
{
    int pad_top, pad_bottom, pad_left, pad_right;
    int kw, kh, okw, okh, k_stride, sk_h, sk_w;
    int in_w, in_h, in_c, in_stride;
    int out_w, out_h, out_stride;
    int w, h, c;
    int dilation_h, dilation_w;
    int i, j, k, ii, jj, kk, index_cin, counter;
    int row, col;
    half *rs1_start;
    half *left_val, *row_val;
    int8_t *col_val;
    half *start;
    half val;

    //get the padding
    pad_top = (ss->conv_padding >> 24) & 0xff;
    pad_bottom = (ss->conv_padding >> 16) & 0xff;
    pad_left = (ss->conv_padding >> 8) & 0xff;
    pad_right = ss->conv_padding & 0xff;

    //get the input shape
    in_w = (ss->conv_fm_in >> 16) & 0xffff;
    in_h = (ss->conv_fm_in) & 0xffff;
    in_c = (ss->conv_cin) & 0xffff;
    assert(in_w > 0 && in_h > 0 && in_c > 0);
    in_stride = (ss->conv_cin >> 16) & 0xffff;
    in_stride = in_stride > 0 ? in_stride : in_c;

    //get the output shape
    out_w = (ss->conv_fm_out >> 16) & 0xffff;
    out_h = (ss->conv_fm_out) & 0xffff;
    assert(out_w > 0 && out_h > 0);
    out_stride = (ss->conv_cout >> 16) & 0xffff;
    out_stride = out_stride > 0 ? out_stride : in_c;

    //get the kernel shape
    kw = (ss->conv_kernel_params1 >> 24) & 0xff;
    kh = (ss->conv_kernel_params1 >> 16) & 0xff;
    dilation_h = (ss->conv_kernel_params1 >> 8) & 0xff;
    dilation_w = (ss->conv_kernel_params2 >> 24) & 0xff;
    sk_h = (ss->conv_kernel_params1) & 0xff;
    sk_w = (ss->conv_kernel_params2 >> 16) & 0xff;
    dilation_h = dilation_h == 0? 1 : dilation_h;
    dilation_w = dilation_w == 0? dilation_h : dilation_w;
    sk_h = sk_h == 0? 1 : sk_h;
    sk_w = sk_w == 0? sk_h : sk_w;
    assert(kw > 0 && kh > 0);
    assert(dilation_h > 0 && dilation_w > 0 && sk_h >0 && sk_w > 0);
    k_stride = ss->conv_kernel_params2 & 0xffff;
    k_stride = k_stride > 0 ? k_stride : in_c;

    //get de/quant coeff
    float16_t quant_coeff = f32_to_f16(ss->mme_quant_coeff);
    float16_t dequant_coeff = f32_to_f16(ss->mme_dequant_coeff);

    /*calculate the kernel shape*/
    Map_int8_t rs2_matrix(rs2, kh * kw, in_c, DynStride(k_stride, 1)); // the depth is same as in_c

    h = dilation_h > 1 ? dilation_h * (kh - 1) + 1 : kh;
    w = dilation_w > 1 ? dilation_w * (kw - 1) + 1 : kw;
    okh = kh;
    okw = kw;
    kh = h;
    kw = w;

    /*calculate the input shape*/
    Map_half rs1_matrix(rs1, in_h * in_w, in_c, DynStride(in_stride, 1));

    /*calculate the output shape*/
    h = (in_h + pad_top + pad_bottom - kh + 1 + sk_h - 1) / sk_h;
    w = (in_w + pad_left + pad_right - kw + 1 + sk_w - 1) / sk_w;
    left_val = (half *)malloc(h * w * okh * okw * in_c * sizeof(half));

    for (i = 0; i < h; i++) {
        for (j = 0; j < w; j++) {
            start = left_val + i * w * okh * okw * in_c + j * okh * okw * in_c;
            rs1_start = rs1;

            for (ii = 0; ii < kh; ii++) {
                for (jj = 0; jj < kw; jj++) {
                    for (kk = 0; kk < in_c; kk++) {
                        row = i * sk_h + ii;
                        col = j * sk_w + jj;

                        if (ii % dilation_h)
                            continue;
                        else {
                            if (jj % dilation_w)
                                continue;
                            else {
				int start_offset = ii/dilation_h * okw * in_c + jj/dilation_w * in_c + kk;
				int rs1_offset = (row - pad_top) * in_w * in_stride + (col - pad_left) * in_stride + kk;
                                if (row >= pad_top && row < pad_top + in_h && col >= pad_left && col < pad_left + in_w) {
                                    *(start + start_offset) = *(rs1_start + rs1_offset);
                                    val = *(rs1_start + rs1_offset);
                                }
                                else {
                                    *(start + start_offset) = (half)0;
                                    val =  (half)0;
                                }
                            }//jj % dilation_w
                        }//ii % dilation_h
                    }//kk
                }//jj
            }//ii
        }//j
    }//i

    /*param check*/
    assert(h == out_h && w == out_w);

    /*calculate convolution*/
    Map_half left_matrix(left_val, h * w, okh * okw * in_c, DynStride(okh * okw * in_c, 1));
    Map_half rd_matrix(rd, out_h * out_w, in_c, DynStride(out_stride, 1));
    int32_t res;
    float16_t rs1_f16;
    int8_t rs1_i8;
    if (debug) {
        cout << "rs1: " << rs1_matrix << endl;
        cout << "rs2: " << rs2_matrix << endl;
    }
    //rd_matrix = left_matrix * rs2_matrix;
    for (i = 0; i < out_h * out_w; i++) {
        for (j = 0; j < in_c; j++) {
            res = 0;
            for (k = 0; k < okh * okw; k++) {
                rs1_f16 = f16_mul(half_to_float16_t(left_matrix(i, k*in_c+j)), quant_coeff);
                rs1_i8 = f16_to_i8(rs1_f16, softfloat_round_near_maxMag, true);
                res += rs1_i8 * rs2_matrix(k, j);
            }
            rd_matrix(i, j) = int32_mul_f16(res, dequant_coeff);
            if (debug) cout << "rd: " << hex << rd_matrix(i, j).x << endl;
        }
    }


    free(row_val);
    free(col_val);
    free(left_val);
    return 0;
}
/**
 * medwconv_mm() medwconv.mm
 *
 * 标量和矩阵元素广播乘 M = M1 * f
 * @param rs1 M1,源操作矩阵基地址
 * @param rd M,目的矩阵基地址
 * @param rs2 f,源标量操作数
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
int CustomInsns::medwconv_mm(int8_t *rs1, half *rd, int8_t *rs2, struct ConvShapeStride *ss)
{
    int pad_top, pad_bottom, pad_left, pad_right;
    int kw, kh, okw, okh, k_stride, sk_h, sk_w;
    int in_w, in_h, in_c, in_stride;
    int out_w, out_h, out_stride;
    int w, h, c;
    int dilation_h, dilation_w;
    int i, j, k, ii, jj, kk, index_cin, counter;
    int row, col;
    int8_t *rs1_start;
    int8_t *left_val, *row_val, *col_val;
    int8_t *start;
    int32_t val;

    //get the padding
    pad_top = (ss->conv_padding >> 24) & 0xff;
    pad_bottom = (ss->conv_padding >> 16) & 0xff;
    pad_left = (ss->conv_padding >> 8) & 0xff;
    pad_right = ss->conv_padding & 0xff;

    //get the input shape
    in_w = (ss->conv_fm_in >> 16) & 0xffff;
    in_h = (ss->conv_fm_in) & 0xffff;
    in_c = (ss->conv_cin) & 0xffff;
    assert(in_w > 0 && in_h > 0 && in_c > 0);
    in_stride = (ss->conv_cin >> 16) & 0xffff;
    in_stride = in_stride > 0 ? in_stride : in_c;

    //get the output shape
    out_w = (ss->conv_fm_out >> 16) & 0xffff;
    out_h = (ss->conv_fm_out) & 0xffff;
    assert(out_w > 0 && out_h > 0);
    out_stride = (ss->conv_cout >> 16) & 0xffff;
    out_stride = out_stride > 0 ? out_stride : in_c;

    //get the kernel shape
    kw = (ss->conv_kernel_params1 >> 24) & 0xff;
    kh = (ss->conv_kernel_params1 >> 16) & 0xff;
    dilation_h = (ss->conv_kernel_params1 >> 8) & 0xff;
    dilation_w = (ss->conv_kernel_params2 >> 24) & 0xff;
    sk_h = (ss->conv_kernel_params1) & 0xff;
    sk_w = (ss->conv_kernel_params2 >> 16) & 0xff;
    dilation_h = dilation_h == 0? 1 : dilation_h;
    dilation_w = dilation_w == 0? dilation_h : dilation_w;
    sk_h = sk_h == 0? 1 : sk_h;
    sk_w = sk_w == 0? sk_h : sk_w;
    assert(kw > 0 && kh > 0);
    assert(dilation_h > 0 && dilation_w > 0 && sk_h >0 && sk_w > 0);
    k_stride = ss->conv_kernel_params2 & 0xffff;
    k_stride = k_stride > 0 ? k_stride : in_c;
    
    //get de/quant coeff
    float16_t dequant_coeff = f32_to_f16(ss->mme_dequant_coeff);

    /*calculate the kernel shape*/
    Map_int8_t rs2_matrix(rs2, kh * kw, in_c, DynStride(k_stride, 1)); // the depth is same as in_c

    h = dilation_h > 1 ? dilation_h * (kh - 1) + 1 : kh;
    w = dilation_w > 1 ? dilation_w * (kw - 1) + 1 : kw;
    okh = kh;
    okw = kw;
    kh = h;
    kw = w;

    /*calculate the input shape*/
    Map_int8_t rs1_matrix(rs1, in_h * in_w, in_c, DynStride(in_stride, 1));

    /*calculate the output shape*/
    h = (in_h + pad_top + pad_bottom - kh + 1 + sk_h - 1) / sk_h;
    w = (in_w + pad_left + pad_right - kw + 1 + sk_w - 1) / sk_w;
    left_val = (int8_t *)malloc(h * w * okh * okw * in_c * sizeof(int8_t));

    for (i = 0; i < h; i++) {
        for (j = 0; j < w; j++) {
            start = left_val + i * w * okh * okw * in_c + j * okh * okw * in_c;
            rs1_start = rs1;

            for (ii = 0; ii < kh; ii++) {
                for (jj = 0; jj < kw; jj++) {
                    for (kk = 0; kk < in_c; kk++) {
                        row = i * sk_h + ii;
                        col = j * sk_w + jj;

                        if (ii % dilation_h)
                            continue;
                        else {
                            if (jj % dilation_w)
                                continue;
                            else {
				int start_offset = ii/dilation_h * okw * in_c + jj/dilation_w * in_c + kk;
				int rs1_offset = (row - pad_top) * in_w * in_stride + (col - pad_left) * in_stride + kk;
                                if (row >= pad_top && row < pad_top + in_h && col >= pad_left && col < pad_left + in_w) {
                                    *(start + start_offset) = *(rs1_start + rs1_offset);
                                    val = *(rs1_start + rs1_offset);
                                }
                                else {
                                    *(start + start_offset) = 0;
                                    val =  0;
                                }
                            }//jj % dilation_w
                        }//ii % dilation_h
                    }//kk
                }//jj
            }//ii
        }//j
    }//i

    /*param check*/
    assert(h == out_h && w == out_w);

    /*calculate convolution*/
    Map_int8_t left_matrix(left_val, h * w, okh * okw * in_c, DynStride(okh * okw * in_c, 1));
    Map_half rd_matrix(rd, out_h * out_w, in_c, DynStride(out_stride, 1));
    int32_t res;
    
    //rd_matrix = left_matrix * rs2_matrix;
    for (i = 0; i < out_h * out_w; i++) {
        for (j = 0; j < in_c; j++) {
            res = 0;
            for (k = 0; k < okh * okw; k++) {
                res += (int32_t)(left_matrix(i, k*in_c+j) * rs2_matrix(k, j));
            }
            rd_matrix(i, j) = int32_mul_f16(res, dequant_coeff);
        }
    }

    free(row_val);
    free(col_val);
    free(left_val);
    return 0;
}




/**
 * medwconv_mm() medwconv.mm
 *
 * 标量和矩阵元素广播乘 M = M1 * f
 * @param rs1 M1,源操作矩阵基地址
 * @param rd M,目的矩阵基地址
 * @param rs2 f,源标量操作数
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
int CustomInsns::medwconv_mm(float32_t *rs1, float32_t *rd, float32_t *rs2, struct ConvShapeStride *ss)
{
    int pad_top, pad_bottom, pad_left, pad_right;
    int kw, kh, okw, okh, k_stride, sk_h, sk_w;
    int in_w, in_h, in_c, in_stride;
    int out_w, out_h, out_stride;
    int w, h, c;
    int dilation_h, dilation_w;
    int i, j, k, ii, jj, kk, index_cin, counter;
    int row, col;
    float32_t *rs1_start;
    float32_t *left_val, *row_val, *col_val;
    float32_t *start;
    float32_t val;

    //get the padding
    pad_top = (ss->conv_padding >> 24) & 0xff;
    pad_bottom = (ss->conv_padding >> 16) & 0xff;
    pad_left = (ss->conv_padding >> 8) & 0xff;
    pad_right = ss->conv_padding & 0xff;

    //get the input shape
    in_w = (ss->conv_fm_in >> 16) & 0xffff;
    in_h = (ss->conv_fm_in) & 0xffff;
    in_c = (ss->conv_cin) & 0xffff;
    assert(in_w > 0 && in_h > 0 && in_c > 0);
    in_stride = (ss->conv_cin >> 16) & 0xffff;
    in_stride = in_stride > 0 ? in_stride : in_c;

    //get the output shape
    out_w = (ss->conv_fm_out >> 16) & 0xffff;
    out_h = (ss->conv_fm_out) & 0xffff;
    assert(out_w > 0 && out_h > 0);
    out_stride = (ss->conv_cout >> 16) & 0xffff;
    out_stride = out_stride > 0 ? out_stride : in_c;

    //get the kernel shape
    kw = (ss->conv_kernel_params1 >> 24) & 0xff;
    kh = (ss->conv_kernel_params1 >> 16) & 0xff;
    dilation_h = (ss->conv_kernel_params1 >> 8) & 0xff;
    dilation_w = (ss->conv_kernel_params2 >> 24) & 0xff;
    sk_h = (ss->conv_kernel_params1) & 0xff;
    sk_w = (ss->conv_kernel_params2 >> 16) & 0xff;
    dilation_h = dilation_h == 0? 1 : dilation_h;
    dilation_w = dilation_w == 0? dilation_h : dilation_w;
    sk_h = sk_h == 0? 1 : sk_h;
    sk_w = sk_w == 0? sk_h : sk_w;
    assert(kw > 0 && kh > 0);
    assert(dilation_h > 0 && dilation_w > 0 && sk_h >0 && sk_w > 0);
    k_stride = ss->conv_kernel_params2 & 0xffff;
    k_stride = k_stride > 0 ? k_stride : in_c;

    /*calculate the kernel shape*/
    Map_float32_t rs2_matrix(rs2, kh * kw, in_c, DynStride(k_stride, 1)); // the depth is same as in_c

    h = dilation_h > 1 ? dilation_h * (kh - 1) + 1 : kh;
    w = dilation_w > 1 ? dilation_w * (kw - 1) + 1 : kw;
    okh = kh;
    okw = kw;
    kh = h;
    kw = w;

    /*calculate the input shape*/
    Map_float32_t rs1_matrix(rs1, in_h * in_w, in_c, DynStride(in_stride, 1));

    /*calculate the output shape*/
    h = (in_h + pad_top + pad_bottom - kh + 1 + sk_h - 1) / sk_h;
    w = (in_w + pad_left + pad_right - kw + 1 + sk_w - 1) / sk_w;
    left_val = (float32_t *)malloc(h * w * okh * okw * in_c * sizeof(float32_t));

    for (i = 0; i < h; i++) {
        for (j = 0; j < w; j++) {
            start = left_val + i * w * okh * okw * in_c + j * okh * okw * in_c;
            rs1_start = rs1;

            for (ii = 0; ii < kh; ii++) {
                for (jj = 0; jj < kw; jj++) {
                    for (kk = 0; kk < in_c; kk++) {
                        row = i * sk_h + ii;
                        col = j * sk_w + jj;

                        if (ii % dilation_h)
                            continue;
                        else {
                            if (jj % dilation_w)
                                continue;
                            else {
				int start_offset = ii/dilation_h * okw * in_c + jj/dilation_w * in_c + kk;
				int rs1_offset = (row - pad_top) * in_w * in_stride + (col - pad_left) * in_stride + kk;
                                if (row >= pad_top && row < pad_top + in_h && col >= pad_left && col < pad_left + in_w) {
                                    *(start + start_offset) = *(rs1_start + rs1_offset);
                                    val = *(rs1_start + rs1_offset);
                                }
                                else {
                                    *(start + start_offset) = i32_to_f32(0);
                                    val =  i32_to_f32(0);
                                }
                            }//jj % dilation_w
                        }//ii % dilation_h
                    }//kk
                }//jj
            }//ii
        }//j
    }//i

    /*param check*/
    assert(h == out_h && w == out_w);

    /*calculate convolution*/
    Map_float32_t left_matrix(left_val, h * w, okh * okw * in_c, DynStride(okh * okw * in_c, 1));
    Map_float32_t rd_matrix(rd, out_h * out_w, in_c, DynStride(out_stride, 1));
    float32_t res;

    //rd_matrix = left_matrix * rs2_matrix;
    for (i = 0; i < out_h * out_w; i++) {
        for (j = 0; j < in_c; j++) {
            res.v = 0x80000000;
            for (k = 0; k < okh * okw; k++) {
                res = f32_add(tf32_mul(left_matrix(i, k*in_c+j), rs2_matrix(k, j)), res);
            }
            rd_matrix(i, j) = res;
        }
    }


    free(row_val);
    free(col_val);
    free(left_val);
    return 0;
}
/**
 * veemul_x32_mf() veemul.x32.mf
 *
 * 标量和矩阵元素广播乘 M = M1 * f
 * @param rs1 M1,源操作矩阵基地址
 * @param rd M,目的矩阵基地址
 * @param rs2 f,源标量操作数
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
int CustomInsns::veemul_x32_mf(int32_t *rs1, half *rd, half rs2, struct ShapeStride *ss)
{

    Map_int32_t rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    if (debug) {
        SHAPE_STRIDE_INFO(ss);
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rs2:" << endl << rs2 << endl;
    }

    for (int row = 0; row < rs1_matrix.rows(); row++)
        for (int col = 0; col < rs1_matrix.cols(); col++)
            rd_matrix(row, col) = rs2 * rs1_matrix(row, col);
 
    if (debug)
        cout << "rd:" << endl << rd_matrix << endl;

    return 0;
}

/**
 * veemul_x32_mv() veemul.x32.mv
 * 
 * 向量和矩阵元素广播乘 M = V*M1
 * @param rs1 M1,源操作矩阵基地址
 * @param rs2 M2,源操作向量基地址
 * @param rd M,目的矩阵基地址
 * @param ss 矩阵形状描述
 * @param dim 方向 dim = 0 v为行向量， dim = 1 v为列向量
 * @return 执行结果
 */
int CustomInsns::veemul_x32_mv(int32_t *rs1, half *rd, half *rs2, struct ShapeStride *ss)
{
    Map_int32_t rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));
    Map_half vector_dim0(rs2, 1, ss->shape1_column, DynStride(1, 1));
    
    if (debug) {
        SHAPE_STRIDE_INFO(ss);
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rs2:" << endl << vector_dim0 << endl;
    }

    for (int row = 0; row < rs1_matrix.rows(); row++)
        for (int col = 0; col < rs1_matrix.cols(); col++)
            rd_matrix(row, col) = vector_dim0(0, col) * rs1_matrix(row, col);
   
    if (debug)
        cout << "rd:" << endl << rd_matrix << endl;

    return 0;
}

/**
 * memul_mm() memul.mm
 * 
 * 矩阵和矩阵算术乘，正常算术运算 M = M1.M2
 * 源操作矩阵一的列值必须和源操作矩阵二的行值相等，如果不等则直接返回错误
 * @param rs1 M1,源操作矩阵一基地址
 * @param rd M,目的矩阵基地址
 * @param rs2 M2,源操作矩阵二基地址
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
int CustomInsns::memul_mm(half *rs1, half *rs2, half *rd, struct ShapeStride *ss)
{
    half *row_val;
    half *col_val;
    int i, j, k, counter;
    float32_t first, second, third, forth, res_tmp, res12, res34;
    /* param check */
    if (ss->shape1_column != ss->shape2_row) {
        cout << __FUNCTION__ << ": shape1_column must equal shape2_row" << endl;
        return -BR_EPARAM;
    }

    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    Map_half rs2_matrix(rs2, ss->shape2_row, ss->shape2_column, DynStride(ss->stride_rs2, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape2_column);
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape2_column, DynStride(ss->stride_rd, 1));

    if (debug) {
        SHAPE_STRIDE_INFO(ss);
        cout << "rs1:\n" << rs1_matrix << endl;
        cout << "rs2:\n" << rs2_matrix << endl;
    }

    /* dot only support vector not support matrix, so we use '*' to do calculation */
    //rd_matrix = rs1_matrix * rs2_matrix;
    for (i = 0; i < ss->shape1_row; i++) {
        for (j = 0; j < ss->shape2_column; j++) {
            first.v = 0x80000000;
            second.v = 0x80000000;
            third.v = 0x80000000;
            forth.v = 0x80000000;
            counter = 0;
            if (ss->shape2_column <= 32 && ss->stride_rd == ss->shape2_column){
                for (k = 0; k < ss->shape1_column; k++) {
                    res_tmp = half_mul_f32(rs1_matrix(i, k), rs2_matrix(k, j));
                    if(counter%4 == 0) first = f32_add(res_tmp, first);
                    else if(counter%4 == 1) second = f32_add(res_tmp, second);
                    else if(counter%4 == 2) third = f32_add(res_tmp, third);
                    else if(counter%4 == 3) forth = f32_add(res_tmp, forth);
                    ++counter;
                }
                res12 = f32_add(first, third);
                res34 = f32_add(second, forth);
                rd_matrix(i, j) = f32_to_half(f32_add(res12, res34));
            } else {
                for (k = 0; k < ss->shape1_column; k++) {
                    if (! (k % 2))
                        first = f32_add(half_mul_f32(rs1_matrix(i, k), rs2_matrix(k, j)), first);
                    else
                        second = f32_add(half_mul_f32(rs1_matrix(i, k), rs2_matrix(k, j)), second);
                }
                rd_matrix(i, j) = f32_to_half(f32_add(first, second));
            }
        }
    }
    if (debug)
        cout << "rd:\n" << rd_matrix << endl;

    return 0;
}

int CustomInsns::memul_mm(half *rs1, half *rs2, float32_t *rd, struct ShapeStride *ss)
{
    int i, j, k, counter;
    float32_t first, second, third, forth, res_tmp, res12, res34;
    /* param check */
    if (ss->shape1_column != ss->shape2_row) {
        cout << __FUNCTION__ << ": shape1_column must equal shape2_row" << endl;
        return -BR_EPARAM;
    }

    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    Map_half rs2_matrix(rs2, ss->shape2_row, ss->shape2_column, DynStride(ss->stride_rs2, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape2_column);
    Map_float32_t rd_matrix(rd, ss->shape1_row, ss->shape2_column, DynStride(ss->stride_rd, 1));

    /* dot only support vector not support matrix, so we use '*' to do calculation */
    //rd_matrix = rs1_matrix * rs2_matrix;
    for (i = 0; i < ss->shape1_row; i++) {
        for (j = 0; j < ss->shape2_column; j++) {
            first.v = 0x80000000;
            second.v = 0x80000000;
            third.v = 0x80000000;
            forth.v = 0x80000000;
            counter = 0;
            if (ss->shape2_column <= 32 && ss->stride_rd == ss->shape2_column){
                for (k = 0; k < ss->shape1_column; k++) {
                    res_tmp = half_mul_f32(rs1_matrix(i, k), rs2_matrix(k, j));
                    if(counter%4 == 0) first = f32_add(res_tmp, first);
                    else if(counter%4 == 1) second = f32_add(res_tmp, second);
                    else if(counter%4 == 2) third = f32_add(res_tmp, third);
                    else if(counter%4 == 3) forth = f32_add(res_tmp, forth);
                    ++counter;
                }
                res12 = f32_add(first, third);
                res34 = f32_add(second, forth);
                rd_matrix(i, j) = f32_add(res12, res34);
            } else {
                for (k = 0; k < ss->shape1_column; k++) {
                    if (! (k % 2))
                        first = f32_add(half_mul_f32(rs1_matrix(i, k), rs2_matrix(k, j)), first);
                    else
                        second = f32_add(half_mul_f32(rs1_matrix(i, k), rs2_matrix(k, j)), second);
                }
                rd_matrix(i, j) = f32_add(first, second);
            }
        }
    }

    return 0;
}

int CustomInsns::memul_mm(Bfloat16 *rs1, Bfloat16 *rs2, Bfloat16 *rd, struct ShapeStride *ss)
{
    int i, j, k, counter;
    Float32 first, second, third, forth, res_tmp, res12, res34;
    /* param check */
    if (ss->shape1_column != ss->shape2_row) {
        cout << __FUNCTION__ << ": shape1_column must equal shape2_row" << endl;
        return -BR_EPARAM;
    }

    Map_Bfloat16 rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    Map_Bfloat16 rs2_matrix(rs2, ss->shape2_row, ss->shape2_column, DynStride(ss->stride_rs2, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape2_column);
    Map_Bfloat16 rd_matrix(rd, ss->shape1_row, ss->shape2_column, DynStride(ss->stride_rd, 1));

    /* dot only support vector not support matrix, so we use '*' to do calculation */
    //rd_matrix = rs1_matrix * rs2_matrix;
    for (i = 0; i < ss->shape1_row; i++) {
        for (j = 0; j < ss->shape2_column; j++) {
            first.x = 0x80000000;
            second.x = 0x80000000;
            third.x = 0x80000000;
            forth.x = 0x80000000;
            counter = 0;
            if (ss->shape2_column <= 32 && ss->stride_rd == ss->shape2_column){
                for (k = 0; k < ss->shape1_column; k++) {
                    res_tmp = Float32::mulConvert(rs1_matrix(i, k), rs2_matrix(k, j));
                    if(counter%4 == 0) first =  res_tmp + first;
                    else if(counter%4 == 1) second = res_tmp + second;
                    else if(counter%4 == 2) third = res_tmp + third;
                    else if(counter%4 == 3) forth = res_tmp + forth;
                    ++counter;
                }
                res12 = first + third;
                res34 = second + forth;
                rd_matrix(i, j) = Bfloat16(res12 + res34);
            } else {
                for (k = 0; k < ss->shape1_column; k++) {
                    if (! (k % 2))
                        first = Float32::mulConvert(rs1_matrix(i, k), rs2_matrix(k, j)) + first;
                    else
                        second = Float32::mulConvert(rs1_matrix(i, k), rs2_matrix(k, j)) + second;
                }
                rd_matrix(i, j) = Bfloat16(first + second);
            }
        }
    }
    
    if (debug)
        cout << "rd:\n" << rd_matrix << endl;

    return 0;
}

int CustomInsns::memul_mm(Bfloat16 *rs1, Bfloat16 *rs2, Float32 *rd, struct ShapeStride *ss)
{
    int i, j, k, counter;
    Float32 first, second, third, forth, res_tmp, res12, res34;
    /* param check */
    if (ss->shape1_column != ss->shape2_row) {
        cout << __FUNCTION__ << ": shape1_column must equal shape2_row" << endl;
        return -BR_EPARAM;
    }

    Map_Bfloat16 rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    Map_Bfloat16 rs2_matrix(rs2, ss->shape2_row, ss->shape2_column, DynStride(ss->stride_rs2, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape2_column);
    Map_Float32 rd_matrix(rd, ss->shape1_row, ss->shape2_column, DynStride(ss->stride_rd, 1));

    /* dot only support vector not support matrix, so we use '*' to do calculation */
    //rd_matrix = rs1_matrix * rs2_matrix;
    for (i = 0; i < ss->shape1_row; i++) {
        for (j = 0; j < ss->shape2_column; j++) {
            first.x = 0x80000000;
            second.x = 0x80000000;
            third.x = 0x80000000;
            forth.x = 0x80000000;
            counter = 0;
            if (ss->shape2_column <= 32 && ss->stride_rd == ss->shape2_column){
                for (k = 0; k < ss->shape1_column; k++) {
                    res_tmp = Float32::mulConvert(rs1_matrix(i, k), rs2_matrix(k, j));
                    if(counter%4 == 0) first =  res_tmp + first;
                    else if(counter%4 == 1) second = res_tmp + second;
                    else if(counter%4 == 2) third = res_tmp + third;
                    else if(counter%4 == 3) forth = res_tmp + forth;
                    ++counter;
                }
                res12 = first + third;
                res34 = second + forth;
                rd_matrix(i, j) = res12 + res34;
            } else {
                for (k = 0; k < ss->shape1_column; k++) {
                    if (! (k % 2))
                        first = Float32::mulConvert(rs1_matrix(i, k), rs2_matrix(k, j)) + first;
                    else
                        second = Float32::mulConvert(rs1_matrix(i, k), rs2_matrix(k, j)) + second;
                }
                rd_matrix(i, j) = first + second;
            }
        }
    }

    if (debug)
       cout << "rd:\n" << rd_matrix << endl;

    return 0;
}
/**
 * memul_mm() memul.mm
 * 
 * 矩阵和矩阵算术乘，正常算术运算 M = M1.M2
 * 源操作矩阵一的列值必须和源操作矩阵二的行值相等，如果不等则直接返回错误
 * @param rs1 M1,源操作矩阵一基地址
 * @param rd M,目的矩阵基地址
 * @param rs2 M2,源操作矩阵二基地址
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
int CustomInsns::memul_mm(half *rs1, int8_t *rs2, half *rd, struct ShapeStride *ss,
                          bool isSign, half *deq_addr)
{
    int i, j, k;
    float16_t rs1_f16;
    int32_t res;

    float16_t quant_coeff = f32_to_f16(ss->mme_quant_coeff);
    /* param check */
    if (ss->shape1_column != ss->shape2_row) {
        cout << __FUNCTION__ << ": shape1_column must equal shape2_row" << endl;
        return -BR_EPARAM;
    }

    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    Map_int8_t rs2_matrix(rs2, ss->shape2_row, ss->shape2_column, DynStride(ss->stride_rs2, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape2_column);
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape2_column, DynStride(ss->stride_rd, 1));

    half *deq_val =  nullptr;
    if (deq_addr) {
        deq_val = deq_addr;
    } else {
        deq_val = (half *)malloc(ss->shape2_column * sizeof(half));
        half dequant = f32_to_half(ss->mme_dequant_coeff);
        for (i = 0; i < ss->shape2_column; i++)
           *(deq_val + i) = dequant;
    }
    Map_half dequant_matrix(deq_val, 1, ss->shape2_column, DynStride(ss->shape2_column, 1));

    /* dot only support vector not support matrix, so we use '*' to do calculation */
    //rd_matrix = rs1_matrix * rs2_matrix;
    for (i = 0; i < ss->shape1_row; i++) {
        for (j = 0; j < ss->shape2_column; j++) {
            res = 0;
            for (k = 0; k < ss->shape1_column; k++) {
                rs1_f16 = f16_mul(half_to_float16_t(rs1_matrix(i, k)), quant_coeff);
                if(isSign) {
                    int8_t rs1_i8 = f16_to_i8(rs1_f16, softfloat_round_near_maxMag, true);
                    res += rs1_i8 * rs2_matrix(k, j);
                } else {
                    uint8_t rs1_ui8 = f16_to_ui8(rs1_f16, softfloat_round_near_maxMag, true);
                    res += rs1_ui8 * rs2_matrix(k, j);
                }
            }
            rd_matrix(i, j) = int32_mul_f16(res, half_to_float16_t(dequant_matrix(0, j)));
        }
    }

    if (debug)
       cout << "rd:\n" << rd_matrix << endl;

    return 0;
}

int CustomInsns::memul_mm(Bfloat16 *rs1, int8_t *rs2, Bfloat16 *rd, struct ShapeStride *ss,
                          bool isSign, Bfloat16 *deq_addr)
{
    int i, j, k;
    Bfloat16 rs1_f16;
    int32_t res;

    Bfloat16 quant_coeff = Bfloat16(ss->mme_quant_coeff);
 
    /* param check */
    if (ss->shape1_column != ss->shape2_row) {
        cout << __FUNCTION__ << ": shape1_column must equal shape2_row" << endl;
        return -BR_EPARAM;
    }

    Map_Bfloat16 rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    Map_int8_t rs2_matrix(rs2, ss->shape2_row, ss->shape2_column, DynStride(ss->stride_rs2, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape2_column);
    Map_Bfloat16 rd_matrix(rd, ss->shape1_row, ss->shape2_column, DynStride(ss->stride_rd, 1));

    Bfloat16 *deq_val =  (Bfloat16 *)malloc(ss->shape2_column * sizeof(Bfloat16));
    if (deq_addr) {
        deq_val = deq_addr;
    } else {
        Bfloat16 dequant = Bfloat16(ss->mme_dequant_coeff);
        for (i = 0; i < ss->shape2_column; i++)
           *(deq_val + i) = dequant;
    }
    Map_Bfloat16 dequant_matrix(deq_val, 1, ss->shape2_column, DynStride(ss->shape2_column, 1));

    /* dot only support vector not support matrix, so we use '*' to do calculation */
    //rd_matrix = rs1_matrix * rs2_matrix;
    for (i = 0; i < ss->shape1_row; i++) {
        for (j = 0; j < ss->shape2_column; j++) {
            res = -0;
            for (k = 0; k < ss->shape1_column; k++) {
                rs1_f16 = rs1_matrix(i, k) * quant_coeff;
                if (isSign) {
                    int8_t rs1_i8 = int8_t(rs1_f16);
                    res += rs1_i8 * rs2_matrix(k, j);
                } else {
                    uint8_t rs1_i8 = uint8_t(rs1_f16);
                    res += rs1_i8 * rs2_matrix(k, j);
                }
                
            }
            rd_matrix(i, j) = int32_mul_bf16(res, dequant_matrix(0, j));        
        }
    }

    if (debug)
       cout << "rd:\n" << rd_matrix << endl;


    return 0;
}


/**
 * memul_mm() memul.mm
 * 
 * 矩阵和矩阵算术乘，正常算术运算 M = M1.M2
 * 源操作矩阵一的列值必须和源操作矩阵二的行值相等，如果不等则直接返回错误
 * @param rs1 M1,源操作矩阵一基地址
 * @param rd M,目的矩阵基地址
 * @param rs2 M2,源操作矩阵二基地址
 * @param ss 矩阵形状描述
 * @return 执行结果
 */


int CustomInsns::memul_mm(int8_t *rs1, int8_t *rs2, half *rd, struct ShapeStride *ss, half *deq_addr)
{
    int i, j, k;
    int even, odd;
    int32_t res;

    /* param check */
    if (ss->shape1_column != ss->shape2_row) {
        cout << __FUNCTION__ << ": shape1_column must equal shape2_row" << endl;
        return -BR_EPARAM;
    }
    
    Map_int8_t rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    Map_int8_t rs2_matrix(rs2, ss->shape2_row, ss->shape2_column, DynStride(ss->stride_rs2, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape2_column);
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape2_column, DynStride(ss->stride_rd, 1));
    
    half *deq_val =  (half *)malloc(ss->shape2_column * sizeof(half));
    if (deq_addr) {
        deq_val = deq_addr;
    } else {
        half dequant = f32_to_half(ss->mme_dequant_coeff);
        for (i = 0; i < ss->shape2_column; i++)
           *(deq_val + i) = dequant;
    }
    Map_half dequant_matrix(deq_val, 1, ss->shape2_column, DynStride(ss->shape2_column, 1));


    for (i = 0; i < ss->shape1_row; i++) {
        for (j = 0; j < ss->shape2_column; j++) {
            res = 0;
            for (k = 0; k < ss->shape1_column; k++) {
                res += (int32_t)(rs1_matrix(i, k) * rs2_matrix(k, j));
            }
            rd_matrix(i, j) = int32_mul_f16(res, half_to_float16_t(dequant_matrix(0, j)));
        }
    }
    if (debug)
        cout << "rd:\n" << rd_matrix << endl;
    

    return 0;
}


int CustomInsns::memul_mm(uint8_t *rs1, int8_t *rs2, half *rd, struct ShapeStride *ss, half *deq_addr)
{
    int i, j, k;
    int even, odd;
    int32_t res;

    /* param check */
    if (ss->shape1_column != ss->shape2_row) {
        cout << __FUNCTION__ << ": shape1_column must equal shape2_row" << endl;
        return -BR_EPARAM;
    }

    Map_uint8_t rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    Map_int8_t rs2_matrix(rs2, ss->shape2_row, ss->shape2_column, DynStride(ss->stride_rs2, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape2_column);
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape2_column, DynStride(ss->stride_rd, 1));

    half *deq_val =  (half *)malloc(ss->shape2_column * sizeof(half));
    if (deq_addr) {
        deq_val = deq_addr;
    } else {
        half dequant = f32_to_half(ss->mme_dequant_coeff);
        for (i = 0; i < ss->shape2_column; i++)
           *(deq_val + i) = dequant;
    }
    Map_half dequant_matrix(deq_val, 1, ss->shape2_column, DynStride(ss->shape2_column, 1));


    for (i = 0; i < ss->shape1_row; i++) {
        for (j = 0; j < ss->shape2_column; j++) {
            res = 0;
            for (k = 0; k < ss->shape1_column; k++) {
                res += (int32_t)(rs1_matrix(i, k) * rs2_matrix(k, j));
            }
            
            rd_matrix(i, j) = int32_mul_f16(res, half_to_float16_t(dequant_matrix(0, j)));
        }
    }

    return 0;
}


int CustomInsns::memul_mm(int8_t *rs1, int8_t *rs2, Bfloat16 *rd, struct ShapeStride *ss, Bfloat16 *deq_addr)
{
    int i, j, k;
    int even, odd;
    int32_t res;

    /* param check */
    if (ss->shape1_column != ss->shape2_row) {
        cout << __FUNCTION__ << ": shape1_column must equal shape2_row" << endl;
        return -BR_EPARAM;
    }

    Map_int8_t rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    Map_int8_t rs2_matrix(rs2, ss->shape2_row, ss->shape2_column, DynStride(ss->stride_rs2, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape2_column);
    Map_Bfloat16 rd_matrix(rd, ss->shape1_row, ss->shape2_column, DynStride(ss->stride_rd, 1));

    Bfloat16 *deq_val =  (Bfloat16 *)malloc(ss->shape2_column * sizeof(Bfloat16));
    if (deq_addr) {
        deq_val = deq_addr;
    } else {
        Bfloat16 dequant = Bfloat16(ss->mme_dequant_coeff);
        for (i = 0; i < ss->shape2_column; i++)
           *(deq_val + i) = dequant;
    }
    Map_Bfloat16 dequant_matrix(deq_val, 1, ss->shape2_column, DynStride(ss->shape2_column, 1));

    for (i = 0; i < ss->shape1_row; i++) {
        for (j = 0; j < ss->shape2_column; j++) {
            res = 0;
            for (k = 0; k < ss->shape1_column; k++)
                res += (int32_t)(rs1_matrix(i, k) * rs2_matrix(k, j));
            
            rd_matrix(i, j) = int32_mul_bf16(res, dequant_matrix(0, j)); 
        }
    }
    
    if (debug)
        cout << "rd:\n" << rd_matrix << endl;


    return 0;
}


int CustomInsns::memul_mm(uint8_t *rs1, int8_t *rs2, Bfloat16 *rd, struct ShapeStride *ss, Bfloat16 *deq_addr)
{
    int i, j, k;
    int even, odd;
    int32_t res;

    /* param check */
    if (ss->shape1_column != ss->shape2_row) {
        cout << __FUNCTION__ << ": shape1_column must equal shape2_row" << endl;
        return -BR_EPARAM;
    }

    Map_uint8_t rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    Map_int8_t rs2_matrix(rs2, ss->shape2_row, ss->shape2_column, DynStride(ss->stride_rs2, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape2_column);
    Map_Bfloat16 rd_matrix(rd, ss->shape1_row, ss->shape2_column, DynStride(ss->stride_rd, 1));

    Bfloat16 *deq_val =  (Bfloat16 *)malloc(ss->shape2_column * sizeof(Bfloat16));
    if (deq_addr) {
        deq_val = deq_addr;
    } else {
        Bfloat16 dequant = Bfloat16(ss->mme_dequant_coeff);
        for (i = 0; i < ss->shape2_column; i++)
           *(deq_val + i) = dequant;
    }
    Map_Bfloat16 dequant_matrix(deq_val, 1, ss->shape2_column, DynStride(ss->shape2_column, 1));

    for (i = 0; i < ss->shape1_row; i++) {
        for (j = 0; j < ss->shape2_column; j++) {
            res = 0;
            for (k = 0; k < ss->shape1_column; k++)
                res += (int32_t)(rs1_matrix(i, k) * rs2_matrix(k, j));
            
            rd_matrix(i, j) = int32_mul_bf16(res, dequant_matrix(0, j));
        }
    }
    
    if (debug)
        cout << "rd:\n" << rd_matrix << endl;

    return 0;
}


/**
 * memul_mm() memul.mm
 * 
 * 矩阵和矩阵算术乘，正常算术运算 M = M1.M2
 * 源操作矩阵一的列值必须和源操作矩阵二的行值相等，如果不等则直接返回错误
 * @param rs1 M1,源操作矩阵一基地址
 * @param rd M,目的矩阵基地址
 * @param rs2 M2,源操作矩阵二基地址
 * @param ss 矩阵形状描述
 * @return 执行结果
 */

int CustomInsns::memul_mm(float32_t *rs1, float32_t *rs2, float32_t *rd, struct ShapeStride *ss)
{
    int i, j, k, counter;
    float32_t first, second, third, forth, res12, res34, res_tmp;
    float32_t res;

    /* param check */
    if (ss->shape1_column != ss->shape2_row) {
        cout << __FUNCTION__ << ": shape1_column must equal shape2_row" << endl;
        return -BR_EPARAM;
    }

    Map_float32_t rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    Map_float32_t rs2_matrix(rs2, ss->shape2_row, ss->shape2_column, DynStride(ss->stride_rs2, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape2_column);
    Map_float32_t rd_matrix(rd, ss->shape1_row, ss->shape2_column, DynStride(ss->stride_rd, 1));
     

    for (i = 0; i < ss->shape1_row; i++) {
        for (j = 0; j < ss->shape2_column; j++) {
            first.v = 0x80000000;
            second.v = 0x80000000;
            third.v = 0x80000000;
            forth.v = 0x80000000;
            counter = 0;
            if (ss->shape2_column <= 32 && ss->stride_rd == ss->shape2_column){
                for (k = 0; k < ss->shape1_column; k++) {
                    res_tmp = tf32_mul(rs1_matrix(i, k), rs2_matrix(k, j));
                    if(counter%4 == 0) first = f32_add(res_tmp, first);
                    else if(counter%4 == 1) second = f32_add(res_tmp, second);
                    else if(counter%4 == 2) third = f32_add(res_tmp, third);
                    else if(counter%4 == 3) forth = f32_add(res_tmp, forth);
                    ++counter;
                }
                res12 = f32_add(first, third);
                res34 = f32_add(second, forth);
                rd_matrix(i, j) = f32_add(res12, res34);
            } else {
                for (k = 0; k < ss->shape1_column; k++) {
                    if (! (k % 2))
                        first = f32_add(tf32_mul(rs1_matrix(i, k), rs2_matrix(k, j)), first);
                    else
                        second = f32_add(tf32_mul(rs1_matrix(i, k), rs2_matrix(k, j)), second);
                }
                rd_matrix(i, j) = f32_add(first, second);
            }
        }
    }
    /* dot only support vector not support matrix, so we use '*' to do calculation */
    //rd_matrix = rs1_matrix * rs2_matrix;
    // if (debug)
        // cout << "rd:\n" << rd_matrix << endl;

    return 0;
}

/**
 * memul_sp_mm() memul.sp.mm
 * 
 * 矩阵和矩阵算术乘，正常算术运算 M = M1.M2
 * 源操作矩阵一的列值必须和源操作矩阵二的行值相等，如果不等则直接返回错误
 * @param rs1 M1,源操作矩阵一基地址
 * @param rd M,目的矩阵基地址
 * @param rs2 M2,源操作矩阵二基地址
 * @param ss 矩阵形状描述
 * @return 执行结果
 * 
 * ss->sride_idx: bit
 */
int CustomInsns::memul_sp_mm(half *rs1, half *rs2, uint8_t *sparseidx, half *rd, struct ShapeStride *ss)
{
    uint8_t *sp_idx_data;
    int i, j, k;
    uint32_t sp_index1, sp_index2;
    float32_t odd, even;
    /* param check */
    if (ss->shape1_column != (ss->shape2_row * 2) ) {
        cout << __FUNCTION__ << ": shape1_column must equal shape2_row * 2" << endl;
        return -BR_EPARAM;
    }

    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    Map_half rs2_matrix(rs2, ss->shape2_row, ss->shape2_column, DynStride(ss->stride_rs2, 1));
    i = (ss->shape2_row * ss->stride_idx + 3)/4;
    Map_uint8_t tmp_matrix(sparseidx, 1, i, DynStride(i, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape2_column);
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape2_column, DynStride(ss->shape2_column, 1));

    if (debug) {
        SHAPE_STRIDE_INFO(ss);
        cout << "rs1:\n" << rs1_matrix << endl;
        cout << "rs2:\n" << rs2_matrix << endl;
        cout << "idx:\n";
        for(j = 0; j < i; j++)
            cout << (int32_t)tmp_matrix(0,j) << endl;
    }

    sp_idx_data = (uint8_t *)malloc(ss->shape2_row * ss->shape2_column * sizeof(uint8_t));
    Map_uint8_t sp_matrix(sp_idx_data, ss->shape2_row, ss->shape2_column, DynStride(ss->shape2_column, 1));

    k = 0;
    for (i = 0; i < ss->shape2_row; i++){
        for(j = 0; j < ss->shape2_column; j++){
            sp_matrix(i, j) = (tmp_matrix(0, k/4) >> (k%4 * 2)) &3;
            ++k;
        }
        for (j = 0; j < (ss->stride_idx - ss->shape2_column); j++){
            ++k;
        }
    }

    /* dot only support vector not support matrix, so we use '*' to do calculation */
    //rd_matrix = rs1_matrix * rs2_matrix;
    for (i = 0; i < ss->shape1_row; i++) {
        for (j = 0; j < ss->shape2_column; j++) {
            odd.v = 0x0000000;
            even.v = 0x0000000;
            for (k = 0; k < ss->shape1_column; k+=4) {
                sp_index1 = sp_matrix(k/2, j);
                sp_index2 = sp_matrix(k/2+1, j);
                odd = f32_add(odd, half_mul_f32(rs1_matrix(i, k+sp_index1), rs2_matrix(k/2, j)));
                even = f32_add(even, half_mul_f32(rs1_matrix(i, k+sp_index2), rs2_matrix(k/2+1, j)));
                if (debug ) {
                    cout << sp_index1 << ":"<< rs1_matrix(i, k+sp_index1) << "*" << rs2_matrix(k/2, j) << endl;
                    cout << sp_index2 << ":"<< rs1_matrix(i, k+sp_index2) << "*" << rs2_matrix(k/2+1, j);
                    cout << endl;
                }
            }
            rd_matrix(i, j) = f32_to_half(f32_add(odd, even));
        }
    }
    if (debug)
        cout << "rd:\n" << rd_matrix << endl;

    free(sp_idx_data);
    return 0;
}


/**
 * memul_sp_mm() memul.sp.mm
 * 
 * 矩阵和矩阵算术乘，正常算术运算 M = M1.M2
 * 源操作矩阵一的列值必须和源操作矩阵二的行值相等，如果不等则直接返回错误
 * @param rs1 M1,源操作矩阵一基地址
 * @param rd M,目的矩阵基地址
 * @param rs2 M2,源操作矩阵二基地址
 * @param ss 矩阵形状描述
 * @return 执行结果
 * 
 * ss->sride_idx: bit
 */
int CustomInsns::memul_sp_mm(half *rs1, int8_t *rs2, uint8_t *sparseidx, half *rd, struct ShapeStride *ss)
{
    uint8_t *sp_idx_data;
    int i, j, k;
    float16_t rs1_f16;
    int8_t rs1_i8;
    int32_t res;
    uint32_t sp_index1, sp_index2;
    int32_t odd, even;
     
    float16_t quant_coeff = f32_to_f16(ss->mme_quant_coeff);
    float16_t dequant_coeff = f32_to_f16(ss->mme_dequant_coeff);

    /* param check */
    if (ss->shape1_column != (ss->shape2_row * 2) ) {
        cout << __FUNCTION__ << ": shape1_column must equal shape2_row * 2" << endl;
        return -BR_EPARAM;
    }

    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    Map_int8_t rs2_matrix(rs2, ss->shape2_row, ss->shape2_column, DynStride(ss->stride_rs2, 1));
    i = (ss->shape2_row * ss->stride_idx + 3)/4;
    Map_uint8_t tmp_matrix(sparseidx, 1, i, DynStride(i, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape2_column);
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape2_column, DynStride(ss->shape2_column, 1));

    sp_idx_data = (uint8_t *)malloc(ss->shape2_row * ss->shape2_column * sizeof(uint8_t));
    Map_uint8_t sp_matrix(sp_idx_data, ss->shape2_row, ss->shape2_column, DynStride(ss->shape2_column, 1));

    k = 0;
    for (i = 0; i < ss->shape2_row; i++){
        for(j = 0; j < ss->shape2_column; j++){
            sp_matrix(i, j) = (tmp_matrix(0, k/4) >> (k%4 * 2)) &3;
            ++k;
        }
        for (j = 0; j < (ss->stride_idx - ss->shape2_column); j++){
            ++k;
        }
    }

    /* dot only support vector not support matrix, so we use '*' to do calculation */
    //rd_matrix = rs1_matrix * rs2_matrix;
    for (i = 0; i < ss->shape1_row; i++) {
        for (j = 0; j < ss->shape2_column; j++) {
            odd = 0x0000000;
            even = 0x0000000;
            for (k = 0; k < ss->shape1_column; k+=4) {
                sp_index1 = sp_matrix(k/2, j);
                sp_index2 = sp_matrix(k/2+1, j);
                rs1_f16 = f16_mul(half_to_float16_t(rs1_matrix(i, k+sp_index1)), quant_coeff);
                rs1_i8 = f16_to_i8(rs1_f16, softfloat_round_near_maxMag, true);
                even += rs1_i8 * rs2_matrix(k/2, j);
                rs1_f16 = f16_mul(half_to_float16_t(rs1_matrix(i, k+sp_index2)), quant_coeff);
                rs1_i8 = f16_to_i8(rs1_f16, softfloat_round_near_maxMag, true);
                odd += rs1_i8 * rs2_matrix(k/2+1, j);
            }
            rd_matrix(i ,j) = int32_mul_f16(odd+even, dequant_coeff);
        }
    }

    free(sp_idx_data);
    return 0;
}


/**
 * memul_sp_mm() memul.sp.mm
 * 
 * 矩阵和矩阵算术乘，正常算术运算 M = M1.M2
 * 源操作矩阵一的列值必须和源操作矩阵二的行值相等，如果不等则直接返回错误
 * @param rs1 M1,源操作矩阵一基地址
 * @param rd M,目的矩阵基地址
 * @param rs2 M2,源操作矩阵二基地址
 * @param ss 矩阵形状描述
 * @return 执行结果
 * 
 * ss->sride_idx: bit
 */
int CustomInsns::memul_sp_mm(int8_t *rs1, int8_t *rs2, uint8_t *sparseidx, half *rd, struct ShapeStride *ss)
{
    uint8_t *sp_idx_data;
    int i, j, k;
    int32_t res;
    uint32_t sp_index1, sp_index2;
    int32_t odd, even;
     
    float16_t quant_coeff = f32_to_f16(ss->mme_quant_coeff);
    float16_t dequant_coeff = f32_to_f16(ss->mme_dequant_coeff);

    /* param check */
    if (ss->shape1_column != (ss->shape2_row * 2) ) {
        cout << __FUNCTION__ << ": shape1_column must equal shape2_row * 2" << endl;
        return -BR_EPARAM;
    }

    Map_int8_t rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    Map_int8_t rs2_matrix(rs2, ss->shape2_row, ss->shape2_column, DynStride(ss->stride_rs2, 1));
    i = (ss->shape2_row * ss->stride_idx + 3)/4;
    Map_uint8_t tmp_matrix(sparseidx, 1, i, DynStride(i, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape2_column);
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape2_column, DynStride(ss->shape2_column, 1));

    sp_idx_data = (uint8_t *)malloc(ss->shape2_row * ss->shape2_column * sizeof(uint8_t));
    Map_uint8_t sp_matrix(sp_idx_data, ss->shape2_row, ss->shape2_column, DynStride(ss->shape2_column, 1));

    k = 0;
    for (i = 0; i < ss->shape2_row; i++){
        for(j = 0; j < ss->shape2_column; j++){
            sp_matrix(i, j) = (tmp_matrix(0, k/4) >> (k%4 * 2)) &3;
            ++k;
        }
        for (j = 0; j < (ss->stride_idx - ss->shape2_column); j++){
            ++k;
        }
    }

    /* dot only support vector not support matrix, so we use '*' to do calculation */
    //rd_matrix = rs1_matrix * rs2_matrix;
    for (i = 0; i < ss->shape1_row; i++) {
        for (j = 0; j < ss->shape2_column; j++) {
            odd = 0x0000000;
            even = 0x0000000;
            for (k = 0; k < ss->shape1_column; k+=4) {
                sp_index1 = sp_matrix(k/2, j);
                sp_index2 = sp_matrix(k/2+1, j);
                even += (rs1_matrix(i, k+sp_index1) * rs2_matrix(k/2, j));
                odd += (rs1_matrix(i, k+sp_index2) * rs2_matrix(k/2 + 1, j));
            }
            rd_matrix(i ,j) = int32_mul_f16(odd+even, dequant_coeff);
        }
    }
    

    free(sp_idx_data);
    return 0;
}

/**
 * memul_sp_mm() memul.sp.mm
 * 
 * 矩阵和矩阵算术乘，正常算术运算 M = M1.M2
 * 源操作矩阵一的列值必须和源操作矩阵二的行值相等，如果不等则直接返回错误
 * @param rs1 M1,源操作矩阵一基地址
 * @param rd M,目的矩阵基地址
 * @param rs2 M2,源操作矩阵二基地址
 * @param ss 矩阵形状描述
 * @return 执行结果
 * 
 * ss->sride_idx: bit
 */
int CustomInsns::memul_sp_mm(float32_t *rs1, float32_t *rs2, uint8_t *sparseidx, float32_t *rd, struct ShapeStride *ss)
{
    uint8_t *sp_idx_data;
    int i, j, k;
    uint32_t sp_index1, sp_index2;
    float32_t odd, even;
    /* param check */
    if (ss->shape1_column != (ss->shape2_row * 2) ) {
        cout << __FUNCTION__ << ": shape1_column must equal shape2_row * 2" << endl;
        return -BR_EPARAM;
    }

    Map_float32_t rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    Map_float32_t rs2_matrix(rs2, ss->shape2_row, ss->shape2_column, DynStride(ss->stride_rs2, 1));
    i = (ss->shape2_row * ss->stride_idx + 3)/4;
    Map_uint8_t tmp_matrix(sparseidx, 1, i, DynStride(i, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape2_column);
    Map_float32_t rd_matrix(rd, ss->shape1_row, ss->shape2_column, DynStride(ss->shape2_column, 1));


    sp_idx_data = (uint8_t *)malloc(ss->shape2_row * ss->shape2_column * sizeof(uint8_t));
    Map_uint8_t sp_matrix(sp_idx_data, ss->shape2_row, ss->shape2_column, DynStride(ss->shape2_column, 1));

    k = 0;
    for (i = 0; i < ss->shape2_row; i++){
        for(j = 0; j < ss->shape2_column; j++){
            sp_matrix(i, j) = (tmp_matrix(0, k/4) >> (k%4 * 2)) &3;
            ++k;
        }
        for (j = 0; j < (ss->stride_idx - ss->shape2_column); j++){
            ++k;
        }
    }

    /* dot only support vector not support matrix, so we use '*' to do calculation */
    //rd_matrix = rs1_matrix * rs2_matrix;
    for (i = 0; i < ss->shape1_row; i++) {
        for (j = 0; j < ss->shape2_column; j++) {
            odd.v = 0x0000000;
            even.v = 0x0000000;
            for (k = 0; k < ss->shape1_column; k+=4) {
                sp_index1 = sp_matrix(k/2, j);
                sp_index2 = sp_matrix(k/2+1, j);
                odd = f32_add(odd, tf32_mul(rs1_matrix(i, k+sp_index1), rs2_matrix(k/2, j)));
                even = f32_add(even, tf32_mul(rs1_matrix(i, k+sp_index2), rs2_matrix(k/2+1, j)));
            }
            rd_matrix(i, j) = f32_add(odd, even);
        }
    }
    free(sp_idx_data);
    return 0;
}

/**
 * memul_mm() memul.mm
 * 
 * 矩阵和矩阵算术乘，正常算术运算 M = M1.M2
 * 源操作矩阵一的列值必须和源操作矩阵二的行值相等，如果不等则直接返回错误
 * @param rs1 M1,源操作矩阵一基地址
 * @param rd M,目的矩阵基地址
 * @param rs2 M2,源操作矩阵二基地址
 * @param ss 矩阵形状描述
 * @param ts: ts mode: 0-rs1转置，1-rs2转置，2-rs1和rs2都转置
 * @return 执行结果
 */
int CustomInsns::memul_ts_mm(half *rs1, half *rs2, half *rd, struct ShapeStride *ss)
{
    half *row_val;
    half *col_val;
    int i, j, k;
    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    Map_half rs2_matrix(rs2, ss->shape2_row, ss->shape2_column, DynStride(ss->stride_rs2, 1));
    
    if (debug) {
        SHAPE_STRIDE_INFO(ss);
        cout << "rs1:\n" << rs1_matrix << endl;
        cout << "rs2:\n" << rs2_matrix << endl;
    }

    /* param check */
    if (ss->shape1_row != ss->shape2_row) {
        cout << __FUNCTION__ << ": shape1_column must equal shape2_row" << endl;
        return -BR_EPARAM;
    }
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape2_column);
    Map_half rd_matrix(rd, ss->shape1_column, ss->shape2_column, DynStride(ss->stride_rd, 1));
    /* dot only support vector not support matrix, so we use '*' to do calculation */
    //rd_matrix = rs1_matrix * rs2_matrix;
    for (i = 0; i < ss->shape1_column; i++) {
        for (j = 0; j < ss->shape2_column; j++) {
            float32_t even = i32_to_f32(0);
            float32_t odd = i32_to_f32(0);
            for (k = 0; k < ss->shape1_row; k++) {
                if (!(k % 2))
                    even = f32_add(even, half_mul_f32(rs1_matrix(k, i), rs2_matrix(k, j)));
                else
                    odd = f32_add(odd, half_mul_f32(rs1_matrix(k, i), rs2_matrix(k, j)));
            }
            rd_matrix(i, j) = f32_to_half(f32_add(even, odd));
        }
    }

    if (debug)
        cout << "rd:\n" << rd_matrix << endl;

    return 0;
}


int CustomInsns::memul_ts_mm(half *rs1, int8_t *rs2, half *rd, struct ShapeStride *ss)
{
    int i, j, k;
    float16_t rs1_f16;
    int8_t rs1_i8;
    int32_t res;

    float16_t quant_coeff = f32_to_f16(ss->mme_quant_coeff);
    float16_t dequant_coeff = f32_to_f16(ss->mme_dequant_coeff);

    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    Map_int8_t rs2_matrix(rs2, ss->shape2_row, ss->shape2_column, DynStride(ss->stride_rs2, 1));
    

    /* param check */
    if (ss->shape1_row != ss->shape2_row) {
        cout << __FUNCTION__ << ": shape1_column must equal shape2_row" << endl;
        return -BR_EPARAM;
    }
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape2_column);
    Map_half rd_matrix(rd, ss->shape1_column, ss->shape2_column, DynStride(ss->stride_rd, 1));
    /* dot only support vector not support matrix, so we use '*' to do calculation */
    //rd_matrix = rs1_matrix * rs2_matrix;
    for (i = 0; i < ss->shape1_column; i++) {
        for (j = 0; j < ss->shape2_column; j++) {
            res = 0;
            for (k = 0; k < ss->shape1_row; k++) {
                rs1_f16 = f16_mul(half_to_float16_t(rs1_matrix(k, i)), quant_coeff);
                rs1_i8 = f16_to_i8(rs1_f16, softfloat_round_near_maxMag, true);
                res += rs1_i8 * rs2_matrix(k, j);
            }
            rd_matrix(i, j) = int32_mul_f16(res, dequant_coeff);
        }
    }

    return 0;
}

int CustomInsns::memul_ts_mm(int8_t *rs1, int8_t *rs2, half *rd, struct ShapeStride *ss)
{
    int i, j, k;
    float16_t rs1_f16;
    int8_t rs1_i8;
    int32_t res;

    float16_t quant_coeff = f32_to_f16(ss->mme_quant_coeff);
    float16_t dequant_coeff = f32_to_f16(ss->mme_dequant_coeff);

    Map_int8_t rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    Map_int8_t rs2_matrix(rs2, ss->shape2_row, ss->shape2_column, DynStride(ss->stride_rs2, 1));
    
 
    /* param check */
    if (ss->shape1_row != ss->shape2_row) {
        cout << __FUNCTION__ << ": shape1_column must equal shape2_row" << endl;
        return -BR_EPARAM;
    }
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape2_column);
    Map_half rd_matrix(rd, ss->shape1_column, ss->shape2_column, DynStride(ss->stride_rd, 1));
    /* dot only support vector not support matrix, so we use '*' to do calculation */
    //rd_matrix = rs1_matrix * rs2_matrix;
    for (i = 0; i < ss->shape1_column; i++) {
        for (j = 0; j < ss->shape2_column; j++) {
            res = 0;
            for (k = 0; k < ss->shape1_row; k++) {
                res += (int32_t)(rs1_matrix(k, i) * rs2_matrix(k, j));
            }
            rd_matrix(i, j) = int32_mul_f16(res, dequant_coeff);
       }
    }
 

    return 0;
}



int CustomInsns::memul_ts_mm(float32_t *rs1, float32_t *rs2, float32_t *rd, struct ShapeStride *ss)
{
    int i, j, k;
    Map_float32_t rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    Map_float32_t rs2_matrix(rs2, ss->shape2_row, ss->shape2_column, DynStride(ss->stride_rs2, 1));
    

    if (ss->shape1_row != ss->shape2_row) {
        cout << __FUNCTION__ << ": shape1_column must equal shape2_row" << endl;
        return -BR_EPARAM;
    }
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape2_column);
    Map_float32_t rd_matrix(rd, ss->shape1_column, ss->shape2_column, DynStride(ss->stride_rd, 1));
    /* dot only support vector not support matrix, so we use '*' to do calculation */
    //rd_matrix = rs1_matrix * rs2_matrix;
    for (i = 0; i < ss->shape1_column; i++) {
        for (j = 0; j < ss->shape2_column; j++) {
            float32_t even = i32_to_f32(0);
            float32_t odd = i32_to_f32(0);
            for (k = 0; k < ss->shape1_row; k++) {
                if (!(k % 2))
                    even = f32_add(even, tf32_mul(rs1_matrix(k, i), rs2_matrix(k, j)));
                else
                    odd = f32_add(odd, tf32_mul(rs1_matrix(k, i), rs2_matrix(k, j)));
            }
            rd_matrix(i, j) = f32_add(even, odd);
        }
    }

    return 0;
}


/**
 * memin_m() memin.m
 * 
 * 矩阵列元素(行向量)求最小值s=min(M1i)
 * @param rs1 M1,源操作矩阵基地址
 * @param rd V,目的向量基地址
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
int CustomInsns::memin_m(half *rs1, half *rd, struct ShapeStride *ss)
{
    int i, j;
    /* param check */

    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, 1);
    Map_half rd_matrix(rd, ss->shape1_row, 1, DynStride(ss->stride_rd, 1));

    if (debug) {
        SHAPE_STRIDE_INFO(ss);
        cout << "rs1:\n" << rs1_matrix << endl;
    }

    /* dot only support vector not support matrix, so we use '*' to do calculation */
    //rd_matrix = rs1_matrix * rs2_matrix;
    for (i = 0; i < ss->shape1_row; i++) {
        float16_t res= half_to_float16_t(rs1_matrix(i, 0));
        for(j = 1; j < ss->shape1_column; j++){
            bool isLt = f16_lt(res, half_to_float16_t(rs1_matrix(i, j)));
            if (!isLt)
                res = half_to_float16_t(rs1_matrix(i, j));
        }
        rd_matrix(i, 0) = float16_t_to_half(res);
    }
    if (debug)
        cout << "rd:\n" << rd_matrix << endl;

    return 0;
}

int CustomInsns::memin_m(Bfloat16 *rs1, Bfloat16 *rd, struct ShapeStride *ss)
{
    int i, j;
    /* param check */
    Map_Bfloat16 rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, 1);
    Map_Bfloat16 rd_matrix(rd, ss->shape1_row, 1, DynStride(ss->stride_rd, 1));

    /* dot only support vector not support matrix, so we use '*' to do calculation */
    //rd_matrix = rs1_matrix * rs2_matrix;
    for (i = 0; i < ss->shape1_row; i++) {
        Bfloat16 res= rs1_matrix(i, 0);
        for(j = 1; j < ss->shape1_column; j++){
            res = res < rs1_matrix(i, j)? res : rs1_matrix(i, j);
        }
        rd_matrix(i, 0) = res;
    }
    return 0;
}

int CustomInsns::memin_m(float32_t *rs1, float32_t *rd, struct ShapeStride *ss)
{
    int i, j;
    /* param check */
    Map_float32_t rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, 1);
    Map_float32_t rd_matrix(rd, ss->shape1_row, 1, DynStride(ss->stride_rd, 1));

    for (i = 0; i < ss->shape1_row; i++) {
        float32_t res= rs1_matrix(i, 0);
        for(j = 1; j < ss->shape1_column; j++){
            bool isLt = f32_lt(res, rs1_matrix(i, j));
            if (!isLt)
                res = rs1_matrix(i, j);
        }
        rd_matrix(i, 0) = res;
    }
    return 0;
}
/**
 * memax_m() memax.m
 * 
 * 矩阵列元素(行向量)最大值s=max(M1i)
 * @param rs1 M1,源操作矩阵基地址
 * @param rd V,目的向量基地址
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
int CustomInsns::memax_m(half *rs1, half *rd, struct ShapeStride *ss)
{
    int i, j;
    /* param check */

    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, 1);
    Map_half rd_matrix(rd, ss->shape1_row, 1, DynStride(ss->stride_rd, 1));

    if (debug) {
        SHAPE_STRIDE_INFO(ss);
        cout << "rs1:\n" << rs1_matrix << endl;
    }

    for (i = 0; i < ss->shape1_row; i++) {
        float16_t res= half_to_float16_t(rs1_matrix(i, 0));
        for(j = 1; j < ss->shape1_column; j++){
            bool isLt = f16_lt(res, half_to_float16_t(rs1_matrix(i, j)));
            if (isLt)
                res = half_to_float16_t(rs1_matrix(i, j));
        }
        rd_matrix(i, 0) = float16_t_to_half(res);
    }
    if (debug)
        cout << "rd:\n" << rd_matrix << endl;
        
    return 0;
}
int CustomInsns::memax_m(Bfloat16 *rs1, Bfloat16 *rd, struct ShapeStride *ss)
{
    int i, j;
    /* param check */

    Map_Bfloat16 rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, 1);
    Map_Bfloat16 rd_matrix(rd, ss->shape1_row, 1, DynStride(ss->stride_rd, 1));

    for (i = 0; i < ss->shape1_row; i++) {
        Bfloat16 res= rs1_matrix(i, 0);
        for(j = 1; j < ss->shape1_column; j++){
            res = res > rs1_matrix(i, j)? res : rs1_matrix(i, j);
        }
        rd_matrix(i, 0) = res;
    }

    return 0;
}

int CustomInsns::memax_m(float32_t *rs1, float32_t *rd, struct ShapeStride *ss)
{
    int i, j;
    /* param check */

    Map_float32_t rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, 1);
    Map_float32_t rd_matrix(rd, ss->shape1_row, 1, DynStride(ss->stride_rd, 1));

    for (i = 0; i < ss->shape1_row; i++) {
        float32_t res= rs1_matrix(i, 0);
        for(j = 1; j < ss->shape1_column; j++){
            bool isLt = f32_lt(res, rs1_matrix(i, j));
            if (isLt)
                res = rs1_matrix(i, j);
        }
        rd_matrix(i, 0) = res;
    }
        
    return 0;
}
/**
 * memacc_m() meacc.m
 * 
 * 矩阵列元素(行向量)和s=sum(M1i)
 * @param rs1 M1,源操作矩阵基地址
 * @param rd V,目的向量基地址
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
int CustomInsns::meacc_m(half *rs1, half *rd, struct ShapeStride *ss)
{
    int i, j;
    /* param check */

    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, 1);
    Map_half rd_matrix(rd, ss->shape1_row, 1, DynStride(ss->stride_rd, 1));

    if (debug) {
        SHAPE_STRIDE_INFO(ss);
        cout << "rs1:\n" << rs1_matrix << endl;
    }

    for (i = 0; i < ss->shape1_row; i++) {
        float32_t odd = i32_to_f32(0);
        float32_t even = i32_to_f32(0);
        for(j = 0; j < ss->shape1_column; j++){
            if (i%2)
                odd = f32_add(odd, half_to_f32(rs1_matrix(i, j)));
            else
                even = f32_add(even, half_to_f32(rs1_matrix(i, j)));
        }
        rd_matrix(i, 0) = f32_to_half(f32_add(odd, even));
    }
    if (debug)
        cout << "rd:\n" << rd_matrix << endl;
        
    return 0;
}

int CustomInsns::meacc_m(float32_t *rs1, float32_t *rd, struct ShapeStride *ss)
{
    int i, j;
    /* param check */

    Map_float32_t rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, 1);
    Map_float32_t rd_matrix(rd, ss->shape1_row, 1, DynStride(ss->stride_rd, 1));

    for (i = 0; i < ss->shape1_row; i++) {
        float32_t odd = i32_to_f32(0);
        float32_t even = i32_to_f32(0);
        for(j = 0; j < ss->shape1_column; j++){
            if (j%2)
                odd = f32_add(odd, rs1_matrix(i,j));
            else 
                even = f32_add(even, rs1_matrix(i, j));
        }
        rd_matrix(i, 0) = f32_add(odd, even);
    }
        
    return 0;
}

int CustomInsns::meacc_m(Bfloat16 *rs1, Bfloat16 *rd, struct ShapeStride *ss)
{
    int i, j;
    /* param check */

    Map_Bfloat16 rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, 1);
    Map_Bfloat16 rd_matrix(rd, ss->shape1_row, 1, DynStride(ss->stride_rd, 1));

    for (i = 0; i < ss->shape1_row; i++) {
        Float32 odd = Float32(0);
        Float32 even = Float32(0);
        for(j = 0; j < ss->shape1_column; j++){
            if (j%2)
                odd += Float32(rs1_matrix(i, j));
            else
                even += Float32(rs1_matrix(i, j));
        }
        rd_matrix(i, 0) = Bfloat16(odd + even);
    }
    if (debug) {
        cout << "rs1: \n" << rs1_matrix << endl;
        cout << "rd: \n" << rd_matrix << endl;
    }

    return 0;
}

/**
 * metr_m() metr.m
 *
 * 将矩阵从一个地方搬移到另一个地方
 * @param rs1 M1,源操作矩阵基地址
 * @param rd V,目的矩阵基地址
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
int CustomInsns::metr_m(half *rs1, half *rd, struct ShapeStride *ss)
{
    int i, j;
    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_row);
    Map_half rd_matrix(rd, ss->shape1_column, ss->shape1_row, DynStride(ss->stride_rd, 1));

    if (debug) {
        SHAPE_STRIDE_INFO(ss);
        cout << "rs1:" << endl << rs1_matrix << endl;
    }

    rd_matrix = rs1_matrix.transpose();

    if (debug)
        cout << "rd:" << endl << rd_matrix << endl;

    return 0;
}

int CustomInsns::metr_m(int8_t *rs1, int8_t *rd, struct ShapeStride *ss)
{
    int i, j;
    Map_int8_t rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_row);
    Map_int8_t rd_matrix(rd, ss->shape1_column, ss->shape1_row, DynStride(ss->stride_rd, 1));

    rd_matrix = rs1_matrix.transpose();

    return 0;
}

int CustomInsns::metr_m(float32_t *rs1, float32_t *rd, struct ShapeStride *ss)
{
    int i, j;
    Map_float32_t rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_row);
    Map_float32_t rd_matrix(rd, ss->shape1_column, ss->shape1_row, DynStride(ss->stride_rd, 1));

    rd_matrix = rs1_matrix.transpose();

    return 0;
}


/**
 * vecvt_hf_x8_m() vecvt.hf.x8.m
 * 
 * 将矩阵中的元素由 int8 格式转换为 fp16格式
 * @param rs1 M1,源操作矩阵基地址
 * @param rd M,目的矩阵基地址
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
int vecvt_hf_x8_m(int8_t *rs1, half *rd, struct ShapeStride *ss)
{
    Map_int8_t rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    SHAPE_STRIDE_INFO(ss);

    for (int row = 0; row < rs1_matrix.rows(); row ++)
    for (int col = 0; col < rs1_matrix.cols(); col ++) {
        rd_matrix(row, col).x = i8_to_f16( rs1_matrix(row, col) ).v;
    }


    if (GLOBAL_DBG) {
        cout << "vecvt_hf_x8_m-rs1:" << endl << rs1_matrix << endl;
        cout << "vecvt_hf_x8_m-rd:" << endl << rd_matrix << endl;
    }

    return 0;
}

/**
 * vecvt_hf_xu8_m() vecvt.hf.xu8.m
 * 
 * 将矩阵中的元素由 uint8 格式转换为 fp16格式
 * @param rs1 M1,源操作矩阵基地址
 * @param rd M,目的矩阵基地址
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
int vecvt_hf_xu8_m(uint8_t *rs1, half *rd, struct ShapeStride *ss)
{
    Map_uint8_t rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    SHAPE_STRIDE_INFO(ss);
    for (int row = 0; row < rs1_matrix.rows(); row ++)
    for (int col = 0; col < rs1_matrix.cols(); col ++) {
        rd_matrix(row, col).x = ui8_to_f16( rs1_matrix(row, col) ).v;
    }

    if (GLOBAL_DBG) {
        cout << "vecvt_hf_xu8_m-rs1:" << endl << rs1_matrix << endl;
        cout << "vecvt_hf_xu8_m-rd:" << endl << rd_matrix << endl;
    }

    return 0;
}

/**
 * vecvt_x8_hf_m() vecvt.x8.hf.m
 * 
 * 将矩阵中的元素由 fp16 格式转换为 int8
 * @param rs1 M1,源操作矩阵基地址
 * @param rd M,目的矩阵基地址
 * @param ss 矩阵形状描述
 * @param rounding_mode 圆整模式
 * @return 执行结果
 */
int vecvt_x8_hf_m(half *rs1, int8_t *rd, struct ShapeStride *ss, uint32_t rounding_mode)
{
    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_int8_t rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    SHAPE_STRIDE_INFO(ss);
    float16_t rs1_f16;
    for (int row = 0; row < rs1_matrix.rows(); row ++)
    for (int col = 0; col < rs1_matrix.cols(); col ++) {
        rs1_f16.v = rs1_matrix(row, col).x;
        rd_matrix(row, col) = f16_to_i8( rs1_f16, rounding_mode, true );
    }

    if (GLOBAL_DBG) {
        cout << "vecvt_x8_hf_m-rs1:" << endl << rs1_matrix << endl;
        cout << "vecvt_x8_hf_m-rd:" << endl << rd_matrix << endl;
    }

    return 0;
}

/**
 * vecvt_xu8_hf_m() vecvt.xu8.hf.m
 * 
 * 将矩阵中的元素由 fp16 格式转换为 uint8
 * @param rs1 M1,源操作矩阵基地址
 * @param rd M,目的矩阵基地址
 * @param ss 矩阵形状描述
 * @param rounding_mode 圆整模式
 * @return 执行结果
 */
int vecvt_xu8_hf_m(half *rs1, uint8_t *rd, struct ShapeStride *ss, uint32_t rounding_mode)
{
    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_uint8_t rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    SHAPE_STRIDE_INFO(ss);

    float16_t rs1_f16;
    for (int row = 0; row < rs1_matrix.rows(); row ++)
    for (int col = 0; col < rs1_matrix.cols(); col ++) {
        rs1_f16.v = rs1_matrix(row, col).x;
        rd_matrix(row, col) = f16_to_ui8( rs1_f16, rounding_mode, true );
    }

    if (GLOBAL_DBG) {
        cout << "vecvt_xu8_hf_m-rs1:" << endl << rs1_matrix << endl;
        cout << "vecvt_xu8_hf_m-rd:" << endl << rd_matrix << endl;
    }

    return 0;
}

/**
 * vecvt_hf_x16_m() vecvt.hf.x16.m
 * 
 * 将矩阵中的元素由 int16 格式转换为 fp16
 * @param rs1 M1,源操作矩阵基地址
 * @param rd M,目的矩阵基地址
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
int vecvt_hf_x16_m(short *rs1, half *rd,  struct ShapeStride *ss)
{
    Map_int16_t rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    SHAPE_STRIDE_INFO(ss);
    for (int row = 0; row < rs1_matrix.rows(); row ++)
    for (int col = 0; col < rs1_matrix.cols(); col ++) {
        rd_matrix(row, col).x = i16_to_f16( rs1_matrix(row, col) ).v;
    }

    if (GLOBAL_DBG) {
        cout << "vecvt_hf_x16_m-rs1:" << endl << rs1_matrix << endl;
        cout << "vecvt_hf_x16_m-rd:" << endl << rd_matrix << endl;
    }

    return 0;
}

/**
 * vecvt_x16_hf_m() vecvt.x16.hf.m
 * 
 * 将矩阵中的元素由 fp16 格式转换为 int16
 * @param rs1 M1,源操作矩阵基地址
 * @param rd M,目的矩阵基地址
 * @param ss 矩阵形状描述
 * @param rounding_mode 圆整模式
 * @return 执行结果
 */
int vecvt_x16_hf_m(half *rs1, short *rd, struct ShapeStride *ss, uint32_t rounding_mode)
{
    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_int16_t rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    SHAPE_STRIDE_INFO(ss);

    float16_t rs1_f16;
    for (int row = 0; row < rs1_matrix.rows(); row ++)
    for (int col = 0; col < rs1_matrix.cols(); col ++) {
        rs1_f16.v = rs1_matrix(row, col).x;
        rd_matrix(row, col) = f16_to_i16( rs1_f16, rounding_mode, true );
    }


    if (GLOBAL_DBG) {
        cout << "vecvt_x16_hf_m-rs1:" << endl << rs1_matrix << endl;
        cout << "vecvt_x16_hf_m-rd:" << endl << rd_matrix << endl;
    }

    return 0;
}

/**
 * vecvt_f32_hf_m() vecvt.f32.hf.m
 * 
 * 将矩阵中的元素由 fp16 格式转换为 fp32
 * @param rs1 M1,源操作矩阵基地址
 * @param rd M,目的矩阵基地址
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
int vecvt_f32_hf_m(half *rs1, Float32 *rd, struct ShapeStride *ss)
{
    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_Float32 rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    SHAPE_STRIDE_INFO(ss);

    float16_t rs1_f16;
    for (int row = 0; row < rs1_matrix.rows(); row ++)
    for (int col = 0; col < rs1_matrix.cols(); col ++) {
        rs1_f16.v = rs1_matrix(row, col).x;
        rd_matrix(row, col).x = f16_to_f32( rs1_f16 ).v;
    }

    if (GLOBAL_DBG) {
        cout << "vecvt_f32_hf_m-rs1:" << endl << rs1_matrix << endl;
        cout << "vecvt_f32_hf_m-rd:" << endl << rd_matrix << endl;
    }

    return 0;
}

/**
 * vecvt_hf_f32_m() vecvt.hf.f32.m
 * 
 * 将矩阵中的元素由 fp32 格式转换为 fp16
 * @param rs1 M1,源操作矩阵基地址
 * @param rd M,目的矩阵基地址
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
int vecvt_hf_f32_m(Float32 *rs1, half *rd, struct ShapeStride *ss)
{
    Map_Float32 rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    SHAPE_STRIDE_INFO(ss);

    float32_t rs1_f32;
    for (int row = 0; row < rs1_matrix.rows(); row ++)
    for (int col = 0; col < rs1_matrix.cols(); col ++) {
        rs1_f32.v = rs1_matrix(row, col).x;
        rd_matrix(row, col).x = f32_to_f16( rs1_f32 ).v;
    }

    if (GLOBAL_DBG) {
        cout << "vecvt_hf_f32_m-rs1:" << endl << rs1_matrix << endl;
        cout << "vecvt_hf_f32_m-rd:" << endl << rd_matrix << endl;
    }

    return 0;
}

/**
 * vecvt_bf_x8_m() vecvt.bf.x8.m
 * 
 * 将矩阵中的元素由 int8 格式转换为 bfp16
 * @param rs1 M1,源操作矩阵基地址
 * @param rd M,目的矩阵基地址
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
int vecvt_bf_x8_m(int8_t *rs1, Bfloat16 *rd, struct ShapeStride *ss)
{
    Map_int8_t rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_Bfloat16 rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    SHAPE_STRIDE_INFO(ss);

    for (int row = 0; row < rs1_matrix.rows(); row ++)
    for (int col = 0; col < rs1_matrix.cols(); col ++) {
        rd_matrix(row, col).x = i8_to_bf16( rs1_matrix(row, col) ).v;
    }
    if (GLOBAL_DBG) {
        cout << "vecvt_bf_x8_m-rs1:" << endl << rs1_matrix << endl;
        cout << "vecvt_bf_x8_m-rd:" << endl << rd_matrix << endl;
    }

    return 0;
}

/**
 * vecvt_bf_xu8_m() vecvt.bf.xu8.m
 * 
 * 将矩阵中的元素由 uint8 格式转换为 bfp16
 * @param rs1 M1,源操作矩阵基地址
 * @param rd M,目的矩阵基地址
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
int vecvt_bf_xu8_m(uint8_t *rs1, Bfloat16 *rd, struct ShapeStride *ss)
{
    Map_uint8_t rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_Bfloat16 rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    SHAPE_STRIDE_INFO(ss);

    for (int row = 0; row < rs1_matrix.rows(); row ++)
    for (int col = 0; col < rs1_matrix.cols(); col ++) {
        rd_matrix(row, col).x = ui8_to_bf16( rs1_matrix(row, col) ).v;
    }

    if (GLOBAL_DBG) {
        cout << "vecvt_bf_xu8_m-rs1:" << endl << rs1_matrix << endl;
        cout << "vecvt_bf_xu8_m-rd:" << endl << rd_matrix << endl;
    }

    return 0;
}

/**
 * vecvt_x8_bf_m() vecvt.x8.bf.m
 * 
 * 将矩阵中的元素由 bfp16 格式转换为 int8
 * @param rs1 M1,源操作矩阵基地址
 * @param rd M,目的矩阵基地址
 * @param ss 矩阵形状描述
 * @param rounding_mode 圆整模式
 * @return 执行结果
 */
int vecvt_x8_bf_m(Bfloat16 *rs1, int8_t *rd, struct ShapeStride *ss, uint32_t rounding_mode)
{
    Map_Bfloat16 rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_int8_t rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    SHAPE_STRIDE_INFO(ss);

    bfloat16_t rs1_bf16;
    for (int row = 0; row < rs1_matrix.rows(); row ++)
    for (int col = 0; col < rs1_matrix.cols(); col ++) {
        rs1_bf16.v = rs1_matrix(row, col).x;
        rd_matrix(row, col) = bf16_to_i8( rs1_bf16, rounding_mode, true );
    }

    if (GLOBAL_DBG) {
        cout << "vecvt_x8_bf_m-rs1:" << endl << rs1_matrix << endl;
        cout << "vecvt_x8_bf_m-rd:" << endl << rd_matrix << endl;
    }

    return 0;
}

/**
 * vecvt_xu8_bf_m() vecvt.xu8.bf.m
 * 
 * 将矩阵中的元素由 bfp16 格式转换为 uint8
 * @param rs1 M1,源操作矩阵基地址
 * @param rd M,目的矩阵基地址
 * @param ss 矩阵形状描述
 * @param rounding_mode 圆整模式
 * @return 执行结果
 */
int vecvt_xu8_bf_m(Bfloat16 *rs1, uint8_t *rd, struct ShapeStride *ss, uint32_t rounding_mode )
{
    Map_Bfloat16 rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_uint8_t rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    SHAPE_STRIDE_INFO(ss);

    bfloat16_t rs1_bf16;
    for (int row = 0; row < rs1_matrix.rows(); row ++)
    for (int col = 0; col < rs1_matrix.cols(); col ++) {
        rs1_bf16.v = rs1_matrix(row, col).x;
        rd_matrix(row, col) = bf16_to_ui8( rs1_bf16, rounding_mode, true );
    }

    if (GLOBAL_DBG) {
        cout << "vecvt_xu8_bf_m-rs1:" << endl << rs1_matrix << endl;
        cout << "vecvt_xu8_bf_m-rd:" << endl << rd_matrix << endl;
    }

    return 0;
}

/**
 * vecvt_bf_x16_m() vecvt.bf.x16.m
 * 
 * 将矩阵中的元素由 int16 格式转换为 bfp16
 * @param rs1 M1,源操作矩阵基地址
 * @param rd M,目的矩阵基地址
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
int vecvt_bf_x16_m(int16_t *rs1, Bfloat16 *rd, struct ShapeStride *ss)
{
    Map_int16_t rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_Bfloat16 rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    SHAPE_STRIDE_INFO(ss);
    for (int row = 0; row < rs1_matrix.rows(); row ++)
    for (int col = 0; col < rs1_matrix.cols(); col ++) {
        rd_matrix(row, col).x = i16_to_bf16( rs1_matrix(row, col) ).v;
    }

    if (GLOBAL_DBG) {
        cout << "vecvt_bf_x16_m-rs1:" << endl << rs1_matrix << endl;
        cout << "vecvt_bf_x16_m-rd:" << endl << rd_matrix << endl;
    }

    return 0;
}

/**
 * vecvt_x16_bf_m() vecvt.x16.bf.m
 * 
 * 将矩阵中的元素由 bfp16 格式转换为 int16
 * @param rs1 M1,源操作矩阵基地址
 * @param rd M,目的矩阵基地址
 * @param ss 矩阵形状描述
 * @param rounding_mode 圆整模式
 * @return 执行结果
 */
int vecvt_x16_bf_m(Bfloat16 *rs1, int16_t *rd, struct ShapeStride *ss, uint32_t rounding_mode)
{
    Map_Bfloat16 rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_int16_t rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    SHAPE_STRIDE_INFO(ss);
    bfloat16_t rs1_bf16;
    for (int row = 0; row < rs1_matrix.rows(); row ++)
    for (int col = 0; col < rs1_matrix.cols(); col ++) {
        rs1_bf16.v = rs1_matrix(row, col).x;
        rd_matrix(row, col) = bf16_to_i16( rs1_bf16, rounding_mode, true );
    }

    if (GLOBAL_DBG) {
        cout << "vecvt_x16_bf_m-rs1:" << endl << rs1_matrix << endl;
        cout << "vecvt_x16_bf_m-rd:" << endl << rd_matrix << endl;
    }

    return 0;
}

/**
 * vecvt_f32_bf_m() vecvt.f32.bf.m
 * 
 * 将矩阵中的元素由 bfp16 格式转换为 fp32
 * @param rs1 M1,源操作矩阵基地址
 * @param rd M,目的矩阵基地址
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
int vecvt_f32_bf_m(Bfloat16 *rs1, Float32 *rd, struct ShapeStride *ss)
{
    Map_Bfloat16 rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_Float32 rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    SHAPE_STRIDE_INFO(ss);

    bfloat16_t rs1_bf16;
    for (int row = 0; row < rs1_matrix.rows(); row ++)
    for (int col = 0; col < rs1_matrix.cols(); col ++) {
        rs1_bf16.v = rs1_matrix(row, col).x;
        rd_matrix(row, col).x = bf16_to_f32( rs1_bf16 ).v;
    }

    if (GLOBAL_DBG) {
        cout << "vecvt_f32_bf_m-rs1:" << endl << rs1_matrix << endl;
        cout << "vecvt_f32_bf_m-rd:" << endl << rd_matrix << endl;
    }

    return 0;
}

/**
 * vecvt_bf_f32_m() vecvt.bf.f32.m
 * 
 * 将矩阵中的元素由 fp32 格式转换为 bfp16
 * @param rs1 M1,源操作矩阵基地址
 * @param rd M,目的矩阵基地址
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
int vecvt_bf_f32_m(Float32 *rs1, Bfloat16 *rd, struct ShapeStride *ss)
{
    Map_Float32 rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_Bfloat16 rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    SHAPE_STRIDE_INFO(ss);
    float32_t rs1_f32;
    for (int row = 0; row < rs1_matrix.rows(); row ++)
    for (int col = 0; col < rs1_matrix.cols(); col ++) {
        rs1_f32.v = rs1_matrix(row, col).x;
        rd_matrix(row, col).x = f32_to_bf16( rs1_f32 ).v;
    }

    if (GLOBAL_DBG) {
        cout << "vecvt_bf_f32_m-rs1:" << endl << rs1_matrix << endl;
        cout << "vecvt_bf_f32_m-rd:" << endl << rd_matrix << endl;
    }

    return 0;
}

/**
 * vecvt_bf_hf_m() vecvt.bf.hf.m
 * 
 * 将矩阵中的元素由 fp16 格式转换为 bfp16
 * @param rs1 M1,源操作矩阵基地址
 * @param rd M,目的矩阵基地址
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
int vecvt_bf_hf_m(half *rs1, Bfloat16 *rd, struct ShapeStride *ss)
{
    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_Bfloat16 rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    SHAPE_STRIDE_INFO(ss);

    float16_t rs1_f16;
    for (int row = 0; row < rs1_matrix.rows(); row ++)
    for (int col = 0; col < rs1_matrix.cols(); col ++) {
        rs1_f16.v = rs1_matrix(row, col).x;
        rd_matrix(row, col).x = f16_to_bf16( rs1_f16 ).v;
    }

    if (GLOBAL_DBG) {
        cout << "vecvt_bf_hf_m-rs1:" << endl << rs1_matrix << endl;
        cout << "vecvt_bf_hf_m-rd:" << endl << rd_matrix << endl;
    }

    return 0;
}

/**
 * vecvt_hf_bf_m() vecvt.hf.bf.m
 * 
 * 将矩阵中的元素由 bfp16 格式转换为 fp16
 * @param rs1 M1,源操作矩阵基地址
 * @param rd M,目的矩阵基地址
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
int vecvt_hf_bf_m(Bfloat16 *rs1, half *rd, struct ShapeStride *ss)
{
    Map_Bfloat16 rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    SHAPE_STRIDE_INFO(ss);

    bfloat16_t rs1_bf16;
    for (int row = 0; row < rs1_matrix.rows(); row ++)
    for (int col = 0; col < rs1_matrix.cols(); col ++) {
        rs1_bf16.v = rs1_matrix(row, col).x;
        rd_matrix(row, col).x = bf16_to_f16( rs1_bf16 ).v;
    }

    if (GLOBAL_DBG) {
        cout << "vecvt_hf_bf_m-rs1:" << endl << rs1_matrix << endl;
        cout << "vecvt_hf_bf_m-rd:" << endl << rd_matrix << endl;
    }

    return 0;
}

/**
 * vecvt_f32_x32_m() vecvt.f32.x32.m
 * 
 * 将矩阵中的元素由 int32 格式转换为 fp32
 * @param rs1 M1,源操作矩阵基地址
 * @param rd M,目的矩阵基地址
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
int vecvt_f32_x32_m(int32_t *rs1, Float32 *rd, struct ShapeStride *ss)
{
    Map_int32_t rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_Float32 rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    SHAPE_STRIDE_INFO(ss);
    for (int row = 0; row < rs1_matrix.rows(); row ++)
    for (int col = 0; col < rs1_matrix.cols(); col ++) {
        rd_matrix(row, col).x = i32_to_f32( rs1_matrix(row, col) ).v;
    }

    if (GLOBAL_DBG) {
        cout << "vecvt_f32_x32_m-rs1:" << endl << rs1_matrix << endl;
        cout << "vecvt_f32_x32_m-rd:" << endl << rd_matrix << endl;
    }

    return 0;
}

/**
 * vecvt_x32_f32_m() vecvt.x32.f32.m
 * 
 * 将矩阵中的元素由 fp32 格式转换为 int32
 * @param rs1 M1,源操作矩阵基地址
 * @param rd M,目的矩阵基地址
 * @param ss 矩阵形状描述
 * @param rounding_mode 圆整模式
 * @return 执行结果
 */
int vecvt_x32_f32_m(Float32 *rs1, int32_t *rd, struct ShapeStride *ss, uint32_t rounding_mode)
{
    Map_Float32 rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_int32_t rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    SHAPE_STRIDE_INFO(ss);
    float32_t rs1_f32;
    for (int row = 0; row < rs1_matrix.rows(); row ++)
    for (int col = 0; col < rs1_matrix.cols(); col ++) {
        rs1_f32.v = rs1_matrix(row, col).x;
        rd_matrix(row, col) = f32_to_i32( rs1_f32, rounding_mode, true );
    }

    if (GLOBAL_DBG) {
        cout << "vecvt_x32_f32_m-rs1:" << endl << rs1_matrix << endl;
        cout << "vecvt_x32_f32_m-rd:" << endl << rd_matrix << endl;
    }

    return 0;
}

uint64_t dmae_src_len(uint32_t data_type, struct DmaeShapeStride *dmae_ss)
{
    //src shape
    uint16_t shape_x = dmae_ss->shape_x;
    uint16_t shape_y = dmae_ss->shape_y;
    uint16_t shape_z = dmae_ss->shape_z;

    uint64_t copy_stride_s_x = 0;
    uint64_t copy_stride_s_y = 0;
    uint64_t len = 0;
    uint8_t e_size = 2;

    switch (data_type) {
        case 0x0: // half
        case 0x101: //Bfloat16
        case 0x2: //fp16->fp32
        case 0x102: //bfp16->fp32
            e_size = 2;
            break;
        case 0x202: //Float32
        case 0x201: //fp32->bfp16
        case 0x200: //fp32->fp16
            e_size = 4;
            break;
        case 0x303: //int8_t
            e_size = 1;
            break;
        default:
        break;
    }

    copy_stride_s_x = (dmae_ss->stride_s_x ? dmae_ss->stride_s_x : shape_x) * e_size;
    copy_stride_s_y = dmae_ss->stride_s_y ? dmae_ss->stride_s_y * e_size : shape_y * copy_stride_s_x;

    len = shape_y * copy_stride_s_x + shape_z * copy_stride_s_y;
    return len;
}

uint64_t dmae_dst_len(uint32_t data_type, struct DmaeShapeStride *dmae_ss)
{
    //src shape
    uint16_t shape_x = dmae_ss->shape_x;
    uint16_t shape_y = dmae_ss->shape_y;
    uint16_t shape_z = dmae_ss->shape_z;

    uint64_t copy_stride_d_x = 0;
    uint64_t copy_stride_d_y = 0;
    uint64_t len = 0;
    uint8_t e_size = 2;

    switch (data_type) {
        case 0x0: // half
        case 0x101: //Bfloat16
        case 0x2: //fp16->fp32
        case 0x102: //bfp16->fp32
            e_size = 2;
            break;
        case 0x202: //Float32
        case 0x201: //fp32->bfp16
        case 0x200: //fp32->fp16
            e_size = 4;
            break;
        case 0x303: //int8_t
            e_size = 1;
            break;
        default:
        break;
    }

    copy_stride_d_x = (dmae_ss->stride_d_x ? dmae_ss->stride_d_x : shape_x) * e_size;
    copy_stride_d_y = dmae_ss->stride_d_y ? dmae_ss->stride_d_y * e_size : shape_y * copy_stride_d_x;

    len = shape_y * copy_stride_d_x + shape_z * copy_stride_d_y;
    return len;
}

void dmae_mov(uint8_t* src, uint8_t *dst, uint32_t data_type, struct DmaeShapeStride *dmae_ss)
{
    //src shape
    uint16_t shape_x = dmae_ss->shape_x;
    uint16_t shape_y = dmae_ss->shape_y;
    uint16_t shape_z = dmae_ss->shape_z;

    uint64_t copy_stride_s_x = 0;
    uint64_t copy_stride_s_y = 0;
    uint64_t copy_stride_d_x = 0;
    uint64_t copy_stride_d_y = 0;
    uint64_t copy_s_xy_size = 0;
    uint64_t copy_d_xy_size = 0;
    uint8_t e_size = 2;
    if (GLOBAL_DBG) {
        cout << "data type=" << data_type << endl;
        cout << "shape_x=" << shape_x << endl;
        cout << "shape_y=" << shape_y << endl;
        cout << "shape_z=" << shape_z << endl;
    }

    if (data_type == 0x0 || data_type == 0x101 ||
        data_type == 0x202 || data_type == 0x303) {
        switch (data_type) {
            case 0x0: // half
            case 0x101: //Bfloat16
                e_size = 2;
                break;
            case 0x202: //Float32
                e_size = 4;
                break;
            case 0x303: //int8_t
                e_size = 1;
                break;
            default:
            break;
        }

        copy_stride_s_x = (dmae_ss->stride_s_x ? dmae_ss->stride_s_x : shape_x) * e_size;
        copy_stride_s_y = dmae_ss->stride_s_y ? dmae_ss->stride_s_y * e_size : shape_y * copy_stride_s_x;
        copy_stride_d_x = (dmae_ss->stride_d_x ? dmae_ss->stride_d_x : shape_x) * e_size;
        copy_stride_d_y = dmae_ss->stride_d_y ? dmae_ss->stride_d_y * e_size : shape_y * copy_stride_d_x;

        if ((dmae_ss->stride_s_x | dmae_ss->stride_s_y | dmae_ss->stride_d_x | dmae_ss->stride_d_y) == 0) {
            memcpy(dst, src, shape_x * shape_y * shape_z * e_size);
        }
        else {
            for (int i = 0; i < shape_z; i++) { //z
                for (int j = 0; j < shape_y; j++) //y
                    memcpy(dst + j * copy_stride_d_x + i * copy_stride_d_y, src + j * copy_stride_s_x + i * copy_stride_s_y, shape_x * e_size);
            }
        }
    } else {
        switch (data_type) {
            case 0x2: { //half->float32_t
                copy_stride_s_x = dmae_ss->stride_s_x ? dmae_ss->stride_s_x : shape_x;
                copy_stride_s_y = dmae_ss->stride_s_y ? dmae_ss->stride_s_y : shape_y * copy_stride_s_x;
                copy_stride_d_x = dmae_ss->stride_d_x ? dmae_ss->stride_d_x : shape_x;
                copy_stride_d_y = dmae_ss->stride_d_y ? dmae_ss->stride_d_y : shape_y * copy_stride_d_x;

                float16_t *src_fp16 = (float16_t*)src;
                float32_t *dst_fp32 = (float32_t*)dst;

                for (int i = 0; i < shape_z; i++) {
                    for (int j = 0; j < shape_y; j++ ) {
                        for (int k = 0; k < shape_x; k++)
                            dst_fp32[i * copy_stride_d_y + j * copy_stride_d_x + k] = f16_to_f32(src_fp16[i * copy_stride_s_y + j * copy_stride_s_x + k]);
                    }
                }
            }
            break;
            case 0x102: { //Bfloat16->Float32
                copy_stride_s_x = dmae_ss->stride_s_x ? dmae_ss->stride_s_x : shape_x;
                copy_stride_s_y = dmae_ss->stride_s_y ? dmae_ss->stride_s_y : shape_y * copy_stride_s_x;
                copy_stride_d_x = dmae_ss->stride_d_x ? dmae_ss->stride_d_x : shape_x;
                copy_stride_d_y = dmae_ss->stride_d_y ? dmae_ss->stride_d_y : shape_y * copy_stride_d_x;
                bfloat16_t *src_bf16 = (bfloat16_t*)src;
                float32_t *dst_fp32 = (float32_t*)dst;

                for (int i = 0; i < shape_z; i++) {
                    for (int j = 0; j < shape_y; j++ ) {
                        for (int k = 0; k < shape_x; k++)
                            dst_fp32[i * copy_stride_d_y + j * copy_stride_d_x + k] = bf16_to_f32(src_bf16[i * copy_stride_s_y + j * copy_stride_s_x + k]);
                    }
                }
            }
            break;
            case 0x201: { //Float32->Bfloat16
                copy_stride_s_x = dmae_ss->stride_s_x ? dmae_ss->stride_s_x : shape_x;
                copy_stride_s_y = dmae_ss->stride_s_y ? dmae_ss->stride_s_y : shape_y * copy_stride_s_x;
                copy_stride_d_x = dmae_ss->stride_d_x ? dmae_ss->stride_d_x : shape_x;
                copy_stride_d_y = dmae_ss->stride_d_y ? dmae_ss->stride_d_y : shape_y * copy_stride_d_x;
                float32_t *src_f32 = (float32_t*)src;
                bfloat16_t *dst_bf16 = (bfloat16_t*)dst;

                for (int i = 0; i < shape_z; i++) {
                    for (int j = 0; j < shape_y; j++ ) {
                        for (int k = 0; k < shape_x; k++)
                            dst_bf16[i * copy_stride_d_y + j * copy_stride_d_x + k] = f32_to_bf16(src_f32[i * copy_stride_s_y + j * copy_stride_s_x + k]);
                    }
                }
            }
            break;
            case 0x200: { //Float32->half
                copy_stride_s_x = dmae_ss->stride_s_x ? dmae_ss->stride_s_x : shape_x;
                copy_stride_s_y = dmae_ss->stride_s_y ? dmae_ss->stride_s_y : shape_y * copy_stride_s_x;
                copy_stride_d_x = dmae_ss->stride_d_x ? dmae_ss->stride_d_x : shape_x;
                copy_stride_d_y = dmae_ss->stride_d_y ? dmae_ss->stride_d_y : shape_y * copy_stride_d_x;
                float32_t *src_f32 = (float32_t*)src;
                float16_t *dst_fp16 = (float16_t*)dst;

                for (int i = 0; i < shape_z; i++) {
                    for (int j = 0; j < shape_y; j++ ) {
                        for (int k = 0; k < shape_x; k++)
                            dst_fp16[i * copy_stride_d_y + j * copy_stride_d_x + k] = f32_to_f16(src_f32[i * copy_stride_s_y + j * copy_stride_s_x + k]);
                    }
                }
            }
            break;
            default:
            break;
        }
    }
}