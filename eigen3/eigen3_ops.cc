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
      float32_t f32t;
      f32t = bf16_mul32(bf16a, bf16b);
      Float32 f32;
      f32.x = f32t.v;
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
    printf("conv_depth_in(0x%x): scin = %d depth = %d\n", ss->conv_depth_in, (ss->conv_depth_in >> 16) & 0xffff, ss->conv_depth_in & 0xffff);
    printf("conv_fm_out(0x%x): w = %d h = %d\n", ss->conv_fm_out, (ss->conv_fm_out >> 16) & 0xffff, ss->conv_fm_out & 0xffff);
    printf("conv_depth_out(0x%x): scout = %d depth = %d\n", ss->conv_depth_out, (ss->conv_depth_out >> 16) & 0xffff, ss->conv_depth_out & 0xffff);
    printf("conv_s_kernel(0x%x): stride = %d\n", ss->conv_kernel_params1, ss->conv_kernel_params1 & 0xffff);
    printf("conv_kernel(0x%x): kw = %d kh = %d dilation = %d sk = %d\n", ss->conv_kernel_params2,(ss->conv_kernel_params2 >> 24) & 0xff,(ss->conv_kernel_params2 >> 16) & 0xff,(ss->conv_kernel_params2 >> 8) & 0xff, (ss->conv_kernel_params2 >> 0) & 0xff);
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
    int kw, kh, okw, okh, k_stride, sk;
    int in_w, in_h, in_c, in_stride;
    int out_w, out_h, out_c, out_stride;
    int w, h, c;
    int dilation;
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
    in_c = (ss->conv_depth_in) & 0xffff;
    assert(in_w > 0 && in_h > 0 && in_c > 0);
    in_stride = (ss->conv_depth_in >> 16) & 0xffff;
    assert((in_stride % 2) == 0); //half
    in_stride = in_stride > 0 ? in_stride >> 1 : in_c;

    //get the output shape
    out_w = (ss->conv_fm_out >> 16) & 0xffff;
    out_h = (ss->conv_fm_out) & 0xffff;
    out_c = (ss->conv_depth_out) & 0xffff;
    assert(out_w > 0 && out_h > 0 && out_c > 0);
    out_stride = (ss->conv_depth_out >> 16) & 0xffff;
    assert(out_stride % 2 == 0);
    out_stride = out_stride > 0 ? out_stride >> 1 : out_c;

    //get the kernel shape
    kw = (ss->conv_kernel_params2 >> 24) & 0xff;
    kh = (ss->conv_kernel_params2 >> 16) & 0xff;
    dilation = (ss->conv_kernel_params2 >> 8) & 0xff;
    sk = (ss->conv_kernel_params2) & 0xff;
    assert(sk > 0 && kw > 0 && kh > 0 && dilation > 0);
    k_stride = ss->conv_kernel_params1 & 0xffff;
    assert(k_stride % 2 == 0);
    k_stride = k_stride > 0 ? k_stride >> 1 : out_c;

    /*calculate the kernel shape*/
    Map_half rs2_matrix(rs2, kh * kw * in_c, out_c, DynStride(k_stride, 1)); // the depth is same as in_c

    h = dilation > 1 ? dilation * (kh - 1) + 1 : kh;
    w = dilation > 1 ? dilation * (kw - 1) + 1 : kw;
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
    h = (in_h + pad_top + pad_bottom - kh + 1 + sk - 1) / sk;
    w = (in_w + pad_left + pad_right - kw + 1 + sk - 1) / sk;
    left_val = (half *)malloc(h * w * okh * okw * in_c * sizeof(half));

    for (i = 0; i < h; i++) {
        for (j = 0; j < w; j++) {
            start = left_val + i * w * okh * okw * in_c + j * okh * okw * in_c;
            rs1_start = rs1;

            for (ii = 0; ii < kh; ii++) {
                for (jj = 0; jj < kw; jj++) {
                    for (kk = 0; kk < in_c; kk++) {
                        row = i * sk + ii;
                        col = j * sk + jj;

                        if (ii % dilation)
                            continue;
                        else {
                            if (jj % dilation)
                                continue;
                            else {
				int start_offset = ii/dilation * okw * in_c + jj/dilation * in_c + kk;
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
                            }//jj % dilation
                        }//i % dilation
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
            
            if ((out_stride == out_c ) && (out_c <= 32)){
                if ((in_stride == in_c) && (dilation == 1)){
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
                                res_tmp = half_mul_f32(row_matrix(0, offset + index_cin), col_matrix(0, offset +index_cin));
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
    int kw, kh, okw, okh, k_stride, sk;
    int in_w, in_h, in_c, in_stride;
    int out_w, out_h, out_c, out_stride;
    int w, h, c;
    int dilation;
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
    in_c = (ss->conv_depth_in) & 0xffff;
    assert(in_w > 0 && in_h > 0 && in_c > 0);
    in_stride = (ss->conv_depth_in >> 16) & 0xffff;
    assert((in_stride % 2) == 0); //half
    in_stride = in_stride > 0 ? in_stride >> 1 : in_c;

    //get the output shape
    out_w = (ss->conv_fm_out >> 16) & 0xffff;
    out_h = (ss->conv_fm_out) & 0xffff;
    out_c = (ss->conv_depth_out) & 0xffff;
    assert(out_w > 0 && out_h > 0 && out_c > 0);
    out_stride = (ss->conv_depth_out >> 16) & 0xffff;
    assert(out_stride % 2 == 0);
    out_stride = out_stride > 0 ? out_stride >> 1 : out_c;

    //get the kernel shape
    kw = (ss->conv_kernel_params2 >> 24) & 0xff;
    kh = (ss->conv_kernel_params2 >> 16) & 0xff;
    dilation = (ss->conv_kernel_params2 >> 8) & 0xff;
    sk = (ss->conv_kernel_params2) & 0xff;
    assert(sk > 0 && kw > 0 && kh > 0 && dilation > 0);
    k_stride = ss->conv_kernel_params1 & 0xffff;
    // assert(k_stride % 2 == 0);
    k_stride = k_stride > 0 ? k_stride : out_c;

    //get de/quant coeff
    float16_t quant_coeff = f32_to_f16(ss->mme_quant_coeff);
    float16_t dequant_coeff = f32_to_f16(ss->mme_dequant_coeff);

    /*calculate the kernel shape*/
    Map_int8_t rs2_matrix(rs2, kh * kw * in_c, out_c, DynStride(k_stride, 1)); // the depth is same as in_c

    h = dilation > 1 ? dilation * (kh - 1) + 1 : kh;
    w = dilation > 1 ? dilation * (kw - 1) + 1 : kw;
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
    h = (in_h + pad_top + pad_bottom - kh + 1 + sk - 1) / sk;
    w = (in_w + pad_left + pad_right - kw + 1 + sk - 1) / sk;
    left_val = (half *)malloc(h * w * okh * okw * in_c * sizeof(half));

    for (i = 0; i < h; i++) {
        for (j = 0; j < w; j++) {
            start = left_val + i * w * okh * okw * in_c + j * okh * okw * in_c;
            rs1_start = rs1;

            for (ii = 0; ii < kh; ii++) {
                for (jj = 0; jj < kw; jj++) {
                    for (kk = 0; kk < in_c; kk++) {
                        row = i * sk + ii;
                        col = j * sk + jj;

                        if (ii % dilation)
                            continue;
                        else {
                            if (jj % dilation)
                                continue;
                            else {
				int start_offset = ii/dilation * okw * in_c + jj/dilation * in_c + kk;
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
                            }//jj % dilation
                        }//i % dilation
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


int CustomInsns::meconv_sp_mm(half *rs1, half *rs2, uint8_t *sparseidx, half *rd, struct ConvShapeStride *ss)
{
    int pad_top, pad_bottom, pad_left, pad_right;
    int kw, kh, okw, okh, k_stride, sk, stride_idx;
    int in_w, in_h, in_c, in_stride;
    int out_w, out_h, out_c, out_stride;
    int w, h, c;
    int dilation;
    int i, j, k, ii, jj, kk, index_cin, counter;
    int row, col;
    uint32_t sp_index1, sp_index2;
    half *rs1_start;
    half *left_val, *row_val, *col_val;
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
    in_c = (ss->conv_depth_in) & 0xffff;
    assert(in_w > 0 && in_h > 0 && in_c > 0);
    in_stride = (ss->conv_depth_in >> 16) & 0xffff;
    assert((in_stride % 2) == 0); //half
    in_stride = in_stride > 0 ? in_stride >> 1 : in_c;

    //get the index stride
    stride_idx = ss->stride_idx? ss->stride_idx>>1 : in_c>>1;

    //get the output shape
    out_w = (ss->conv_fm_out >> 16) & 0xffff;
    out_h = (ss->conv_fm_out) & 0xffff;
    out_c = (ss->conv_depth_out) & 0xffff;
    assert(out_w > 0 && out_h > 0 && out_c > 0);
    out_stride = (ss->conv_depth_out >> 16) & 0xffff;
    assert(out_stride % 2 == 0);
    out_stride = out_stride > 0 ? out_stride >> 1 : out_c;

    //get the kernel shape
    kw = (ss->conv_kernel_params2 >> 24) & 0xff;
    kh = (ss->conv_kernel_params2 >> 16) & 0xff;
    dilation = (ss->conv_kernel_params2 >> 8) & 0xff;
    sk = (ss->conv_kernel_params2) & 0xff;
    assert(sk > 0 && kw > 0 && kh > 0 && dilation > 0);
    k_stride = ss->conv_kernel_params1 & 0xffff;
    assert(k_stride % 2 == 0);
    k_stride = k_stride > 0 ? k_stride >> 1 : out_c;

    /* split the 8bit index shape into 2bit */
    i = (kw * kh * stride_idx * out_c + 3)/4;
    Map_uint8_t tmp_matrix(sparseidx, 1, i, DynStride(i, 1));
    sp_idx_data = (uint8_t *)malloc(kw * kh * (in_c/2) * out_c *sizeof(uint8_t));
    Map_uint8_t sp_matrix(sp_idx_data, kh * kw * (in_c/2), out_c, DynStride(out_c, 1));
    ii = 0;
    
    for (i = 0; i < kw * kh; i++){
        for (j = 0; j < (in_c/2); j++)
            for (k = 0; k < out_c; k++) {
                sp_matrix(i*(in_c/2)+j, k) = (tmp_matrix(0, ii/4) >> (ii%4 *2)) &3;
                ++ii;
            }
        for (j = 0; j < (stride_idx - in_c/2); j++)
            for (k = 0; k < out_c; k++) 
                ++ii;
    }
    /////////////////////////////////////////////////////////////

    /*calculate the kernel shape*/
    Map_half rs2_matrix(rs2, kh * kw * (in_c/2), out_c, DynStride(k_stride, 1)); // the depth is same as in_c

    h = dilation > 1 ? dilation * (kh - 1) + 1 : kh;
    w = dilation > 1 ? dilation * (kw - 1) + 1 : kw;
    okh = kh;
    okw = kw;
    kh = h;
    kw = w;

    /*calculate the input shape*/
    Map_half rs1_matrix(rs1, in_h * in_w, in_c, DynStride(in_stride, 1));

    /*calculate the output shape*/
    h = (in_h + pad_top + pad_bottom - kh + 1 + sk - 1) / sk;
    w = (in_w + pad_left + pad_right - kw + 1 + sk - 1) / sk;
    left_val = (half *)malloc(h * w * okh * okw * in_c * sizeof(half));

    for (i = 0; i < h; i++) {
        for (j = 0; j < w; j++) {
            start = left_val + i * w * okh * okw * in_c + j * okh * okw * in_c;
            rs1_start = rs1;

            for (ii = 0; ii < kh; ii++) {
                for (jj = 0; jj < kw; jj++) {
                    for (kk = 0; kk < in_c; kk++) {
                        row = i * sk + ii;
                        col = j * sk + jj;

                        if (ii % dilation)
                            continue;
                        else {
                            if (jj % dilation)
                                continue;
                            else {
				int start_offset = ii/dilation * okw * in_c + jj/dilation * in_c + kk;
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
                            }//jj % dilation
                        }//i % dilation
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
    idx_val = (uint8_t *)malloc(okh * okw * in_c * sizeof(uint8_t));
    Map_half row_matrix(row_val, 1, okh * okw * in_c, DynStride(okh * okw * in_c, 1));
    Map_half col_matrix(col_val, 1, okh * okw * (in_c/2), DynStride(okh * okw * in_c/2, 1));
    Map_uint8_t idx_matrix(idx_val, 1, okh * okw * (in_c/2), DynStride(okh * okw * in_c/2, 1));
    float32_t odd, even;
    if (debug) {
        cout << "rs1: " << rs1_matrix << endl;
        cout << "left: " << left_matrix << endl;
        cout << "rs2: " << rs2_matrix << endl;
        cout << "origin idx:\n";
        for (i = 0; i < (kw * kh * (stride_idx) * out_c + 3)/4; i++)
            cout << (int32_t)tmp_matrix(0,i) << endl;
        cout << "trans idx:\n";
        for(i = 0; i < kw * kh * (in_c/2); i++){
            for (j = 0; j < out_c; j++)
                cout << (int32_t)sp_matrix(i,j) << "\t";
            cout << endl;
        }
    }
    //rd_matrix = left_matrix * rs2_matrix;
    for (i = 0; i < out_h * out_w; i++) {
        for (j = 0; j < out_c; j++) {
            row_matrix = left_matrix.row(i);
            col_matrix = rs2_matrix.col(j).transpose();
            idx_matrix = sp_matrix.col(j).transpose();
            odd.v = 0x80000000;
            even.v = 0x80000000;
            counter = 0;
            for (k = 0; k < okh * okw * in_c; k+=4) {
                sp_index1 = (int32_t)idx_matrix(0, k/2);
                sp_index2 = (int32_t)idx_matrix(0, k/2+1);
                even = f32_add(half_mul_f32(row_matrix(0, k+sp_index1), col_matrix(0, k/2)), even);
                odd = f32_add(half_mul_f32(row_matrix(0, k+sp_index2), col_matrix(0, k/2+1)), odd);
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

int CustomInsns::medeconv_mm(half *rs1, half *rs2, half *rd, struct ConvShapeStride *ss)
{
    int pad_top, pad_bottom, pad_left, pad_right;
    int in_pad_top, in_pad_bottom, in_pad_left, in_pad_right;
    int kw, kh, k_stride, sk, stride_idx;
    int in_w, in_h, in_c, in_stride;
    int out_w, out_h, out_c, out_stride;
    int w, h, c;
    int stride_w, stride_h;
    int i, j, k, m, ii, jj, kk;
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
    in_c = (ss->conv_depth_in) & 0xffff;
    assert(in_w > 0 && in_h > 0 && in_c > 0);
    in_stride = (ss->conv_depth_in >> 16) & 0xffff;
    assert((in_stride % 2) == 0); //half
    in_stride = in_stride > 0 ? in_stride >> 1 : in_c;

    //get the output shape
    out_w = (ss->conv_fm_out >> 16) & 0xffff;
    out_h = (ss->conv_fm_out) & 0xffff;
    out_c = (ss->conv_depth_out) & 0xffff;
    assert(out_w > 0 && out_h > 0 && out_c > 0);
    out_stride = (ss->conv_depth_out >> 16) & 0xffff;
    assert(out_stride % 2 == 0);
    out_stride = out_stride > 0 ? out_stride >> 1 : out_c;

    //get the kernel shape
    kw = (ss->conv_kernel_params2 >> 24) & 0xff;
    kh = (ss->conv_kernel_params2 >> 16) & 0xff;
    sk = (ss->conv_kernel_params2) & 0xff;
    stride_h = (ss->conv_kernel_params2) & 0xff;
    stride_w = (ss-> conv_kernel_params1 >> 16) & 0xff;
    assert(sk > 0 && kw > 0 && kh > 0 && stride_h > 0 && stride_w > 0);
    k_stride = ss->conv_kernel_params1 & 0xffff;
    assert(k_stride % 2 == 0);
    k_stride = k_stride > 0 ? k_stride >> 1 : in_c;

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

    /*kernel 上下左右翻转*/
    ker_val = (half *)malloc(kh * kw * out_c * in_c * sizeof(half));
    for (i = 0; i < kh; i++){
        for (j = 0; j < kw; j++){
            for (k = 0; k < out_c; k++){
                for (m = 0; m < in_c; m++){
                    rs2_offset = i * kw * k_stride * out_c + j * k_stride * out_c + k * k_stride + m;
                    ker_offset = (kh-i-1) * kw * in_c * out_c + (kw-j-1) * in_c * out_c + m * out_c + k;
                    *(ker_val + ker_offset) = *(rs2 + rs2_offset);
                }
            }
        }
    }

    Map_half rs2_matrix(ker_val, kh * kw * in_c, out_c, DynStride(out_c, 1));

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
    float32_t odd, even;
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
            for (k = 0; k < kh * kw * in_c; k++){
                if (! (k % 2))
                    even = f32_add(half_mul_f32(row_matrix(0, k), col_matrix(0, k)), even);
                else
                    odd = f32_add(half_mul_f32(row_matrix(0, k), col_matrix(0, k)), odd);
            }
            rd_matrix(i, j) = f32_to_half(f32_add(odd, even));
            if (debug) {
                cout << "left" << row_matrix << endl;
                cout << "kernel" << col_matrix << endl;
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

int CustomInsns::medeconv_sp_mm(half *rs1, half *rs2, uint8_t *sparseidx, half *rd, struct ConvShapeStride *ss)
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
    in_c = (ss->conv_depth_in) & 0xffff;
    assert(in_w > 0 && in_h > 0 && in_c > 0);
    in_stride = (ss->conv_depth_in >> 16) & 0xffff;
    assert((in_stride % 2) == 0); //half
    in_stride = in_stride > 0 ? in_stride >> 1 : in_c;

    //get the index stride
    stride_idx = ss->stride_idx? ss->stride_idx>>1 : in_c>>1;

    //get the output shape
    out_w = (ss->conv_fm_out >> 16) & 0xffff;
    out_h = (ss->conv_fm_out) & 0xffff;
    out_c = (ss->conv_depth_out) & 0xffff;
    assert(out_w > 0 && out_h > 0 && out_c > 0);
    out_stride = (ss->conv_depth_out >> 16) & 0xffff;
    assert(out_stride % 2 == 0);
    out_stride = out_stride > 0 ? out_stride >> 1 : out_c;

    //get the kernel shape
    kw = (ss->conv_kernel_params2 >> 24) & 0xff;
    kh = (ss->conv_kernel_params2 >> 16) & 0xff;
    stride_h = (ss->conv_kernel_params2) & 0xff;
    stride_w = (ss-> conv_kernel_params1 >> 16) & 0xff;
    assert(kw > 0 && kh > 0 && stride_h > 0 && stride_w > 0);
    k_stride = ss->conv_kernel_params1 & 0xffff;
    assert(k_stride % 2 == 0);
    k_stride = k_stride > 0 ? k_stride >> 1 : in_c >> 1;

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
    sp_idx_data = (uint8_t *)malloc(kw * kh * out_c * ker_c * sizeof(uint8_t));
    ii = 0;
    for (i = 0; i < kw * kh * out_c; i++){
        for (j = 0; j < ker_c; j++) {
            *(sp_idx_data + i*ker_c + j)= ((*(sparseidx+ ii/4)) >> (ii%4 *2)) &3;
            ++ii;
        }
        for (j = 0; j < (stride_idx - ker_c); j++)
            ++ii;
    }

    /* 2. kernel&sp_idx: kh, kw方向上下左右翻转， cin，cout方向做转置*/
    ker_val = (half *)malloc(kh * kw * ker_c * out_c * sizeof(half));
    sp_val = (uint8_t *)malloc(kh * kw * ker_c * out_c * sizeof(uint8_t));
    for (i = 0; i < kh; i++){
        for (j = 0; j < kw; j++){
            for (k = 0; k < out_c; k++){
                for (m = 0; m < ker_c; m++){
                    rs2_offset = i * kw * k_stride * out_c + j * k_stride * out_c + k * k_stride + m;
                    idx_offset = i * kw * ker_c * out_c + j * ker_c * out_c + k * ker_c + m;
                    ker_offset = (kh-i-1) * kw * ker_c * out_c + (kw-j-1) * ker_c * out_c + m * out_c + k;
                    *(ker_val + ker_offset) = *(rs2 + rs2_offset);
                    *(sp_val + ker_offset) = *(sp_idx_data + idx_offset);
                }
            }
        }
    }
    Map_half rs2_matrix(ker_val, kh * kw * ker_c, out_c, DynStride(out_c, 1));    
    Map_uint8_t sp_matrix(sp_val, kh * kw * ker_c, out_c, DynStride(out_c, 1));

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
    col_val = (half *)malloc(kh * kw * ker_c * sizeof(half));
    idx_val = (uint8_t *)malloc(kh * kw * ker_c * sizeof(uint8_t));
    Map_half row_matrix(row_val, 1, kh * kw * in_c, DynStride(kh * kw * in_c, 1));
    Map_half col_matrix(col_val, 1, kh * kw * ker_c, DynStride(kh * kw * ker_c, 1));
    Map_uint8_t idx_matrix(idx_val, 1, kh * kw * ker_c, DynStride(kh * kw * ker_c, 1));
    float32_t odd, even;
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
            idx_matrix = sp_matrix.col(j).transpose();
            odd.v = 0;
            even.v = 0;
            for (k = 0; k < kh * kw * in_c; k+=4){
                sp_index1 = (int32_t)idx_matrix(0, k/2);
                sp_index2 = (int32_t)idx_matrix(0, k/2+1);
                even = f32_add(half_mul_f32(row_matrix(0, k+sp_index1), col_matrix(0, k/2)), even);
                odd = f32_add(half_mul_f32(row_matrix(0, k+sp_index2), col_matrix(0, k/2+1)), odd);
            }
            rd_matrix(i, j) = f32_to_half(f32_add(odd, even));
            if (debug) {
                cout << "left" << row_matrix << endl;
                cout << "kernel" << col_matrix << endl;
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
    free(idx_val);
    free(sp_idx_data);
    free(sp_val);
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
 * veemul_x8_hf_mf() veemul.x8_hf_mf
 *
 * 标量和矩阵元素广播乘 M = M1 * f
 * @param rs1 M1,源操作矩阵基地址
 * @param rd M,目的矩阵基地址
 * @param rs2 f,源标量操作数
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
int CustomInsns::veemul_x8_hf_mf(half *rs1, int8_t *rd, half rs2, struct ShapeStride *ss)
{
    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_int8_t rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    half val;

    if (debug)  {
        SHAPE_STRIDE_INFO(ss);
        cout << "Start veemul x8_hf" << rs2 << endl;
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rs2:" << endl << rs2 << endl;
    }

    for (int row = 0; row < rs1_matrix.rows(); row++) {
        for (int col = 0; col < rs1_matrix.cols(); col++) {
            val = rs2 * rs1_matrix(row, col);
            if (val > (half)127)
                rd_matrix(row, col) = 127;
            else if (val < (half)-128)
                rd_matrix(row, col) = -128;
            else
                rd_matrix(row, col) = (int8_t)val;
        }
    }
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
    int i, j, k;
    float32_t odd, even;
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
            even.v = 0x80000000;
            odd.v = 0x80000000;
            for (k = 0; k < ss->shape1_column; k++) {
                if (!(k % 2))
                    even = f32_add(even, half_mul_f32(rs1_matrix(i, k), rs2_matrix(k, j)));
                else
                    odd = f32_add(odd, half_mul_f32(rs1_matrix(i, k), rs2_matrix(k, j)));
            }
            rd_matrix(i, j) = f32_to_half(f32_add(even, odd));        
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
int CustomInsns::memul_mm(half *rs1, int8_t *rs2, half *rd, struct ShapeStride *ss)
{
    int i, j, k;
    float16_t rs1_f16;
    int8_t rs1_i8;
    int32_t res;

    float16_t quant_coeff = f32_to_f16(ss->mme_quant_coeff);
    float16_t dequant_coeff = f32_to_f16(ss->mme_dequant_coeff);
    /* param check */
    if (ss->shape1_column != ss->shape2_row) {
        cout << __FUNCTION__ << ": shape1_column must equal shape2_row" << endl;
        return -BR_EPARAM;
    }

    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    Map_int8_t rs2_matrix(rs2, ss->shape2_row, ss->shape2_column, DynStride(ss->stride_rs2, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape2_column);
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape2_column, DynStride(ss->stride_rd, 1));

    if (debug) {
        SHAPE_STRIDE_INFO(ss);
        cout << "rs1:\n" << rs1_matrix << endl;
        cout << "rs2: \n";
        for (i = 0; i < ss->shape2_row; i++){
            for (j = 0; j < ss->shape2_column; j++){
                cout << (((short)rs2_matrix(i, j))&0xff) << "\t";
            }
            cout << endl;
        }
    }

    /* dot only support vector not support matrix, so we use '*' to do calculation */
    //rd_matrix = rs1_matrix * rs2_matrix;
    for (i = 0; i < ss->shape1_row; i++) {
        for (j = 0; j < ss->shape2_column; j++) {
            res = -0;
            for (k = 0; k < ss->shape1_column; k++) {
                rs1_f16 = f16_mul(half_to_float16_t(rs1_matrix(i, k)), quant_coeff);
                rs1_i8 = f16_to_i8(rs1_f16, softfloat_round_near_maxMag, true);
                res += rs1_i8 * rs2_matrix(k, j);
                if (debug) {
                    cout << i <<": rs1: " << rs1_matrix(i, k).x;
                    cout << " rs1 * coeff: " << rs1_f16.v;
                    cout << " rs1 * coeff to int8: " << (int)rs1_i8;
                    cout << " rs2: " << (((short)rs2_matrix(k, j))&0xff);
                    k == (ss->shape1_column - 1)? cout << " res " << res <<endl : cout << endl;
                }
            }
            rd_matrix(i, j) = int32_mul_f16(res, dequant_coeff);        
        }
    }
    if (debug)
        cout << "rd:\n" << rd_matrix << endl;

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
    i = (ss->shape2_row * (ss->stride_idx/2) + 3)/4;
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
        for (j = 0; j < (ss->stride_idx/2 - ss->shape2_column); j++){
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
                odd = f32_add(half_mul_f32(rs1_matrix(i, k+sp_index1), rs2_matrix(k/2, j)), odd);
                even = f32_add(half_mul_f32(rs1_matrix(i, k+sp_index2), rs2_matrix(k/2+1, j)), even);
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
int CustomInsns::memul_ts_mm(half *rs1, half *rs2, half *rd, struct ShapeStride *ss, int ts)
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

    if (ts == 0) {
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
    Map_half rd_matrix(rd, ss->shape1_row, 1, DynStride(1, 1));

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
    Map_half rd_matrix(rd, ss->shape1_row, 1, DynStride(1, 1));

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
    Map_half rd_matrix(rd, ss->shape1_row, 1, DynStride(1, 1));

    if (debug) {
        SHAPE_STRIDE_INFO(ss);
        cout << "rs1:\n" << rs1_matrix << endl;
    }

    for (i = 0; i < ss->shape1_row; i++) {
        float32_t res= half_to_f32(rs1_matrix(i, 0));
        for(j = 1; j < ss->shape1_column; j++){
            res = f32_add(res, half_to_f32(rs1_matrix(i, j)));
        }
        rd_matrix(i, 0) = f32_to_half(res);
    }
    if (debug)
        cout << "rd:\n" << rd_matrix << endl;
        
    return 0;
}


/**
 * verecip_m() verecip.m
 * 
 * 矩阵元素求倒运算，正常算术运算 M = 1/M1
 * @param rs1 M1,源操作矩阵一基地址
 * @param rd M,目的矩阵基地址
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
int CustomInsns::verecip_m(half *rs1, half *rd, struct ShapeStride *ss)
{

    half one = (half)1.0;
    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    if (debug) {
        SHAPE_STRIDE_INFO(ss);
        cout << "rs1:\n" << rs1_matrix << endl;
    }

    /* dot only support vector not support matrix, so we use '*' to do calculation */
    for (int row = 0; row < rs1_matrix.rows(); row ++)
	   for (int col = 0; col < rs1_matrix.cols(); col ++)
		  rd_matrix(row, col) =  one / rs1_matrix(row, col);
    if (debug)
        cout << "rd:\n" << rd_matrix << endl;

    return 0;
}


/**
 * vesqrt_m() vesqrt.m
 * 
 * 矩阵元素求平方根，正常算术运算 M = sqrt(M2)
 * @param rs1 M1,源操作矩阵一基地址
 * @param rd M,目的矩阵基地址
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
int CustomInsns::vesqrt_m(half *rs1, half *rd, struct ShapeStride *ss)
{

    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    if (debug) {
        SHAPE_STRIDE_INFO(ss);
        cout << "rs1:\n" << rs1_matrix << endl;
    }

    for (int row = 0; row < rs1_matrix.rows(); row ++)
	   for (int col = 0; col < rs1_matrix.cols(); col ++)
		  rd_matrix(row, col) =  sqrt(rs1_matrix(row, col));
    if (debug)
        cout << "rd:\n" << rd_matrix << endl;

    return 0;
}

/**
 * veexp_m() veexp.m
 * 
 * 矩阵元素指数运算，正常算术运算 M = e^Mij
 * @param rs1 M1,源操作矩阵一基地址
 * @param rd M,目的矩阵基地址
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
int CustomInsns::veexp_m(half *rs1, half *rd, struct ShapeStride *ss)
{

    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    if (debug) {
        SHAPE_STRIDE_INFO(ss);
        cout << "rs1:\n" << rs1_matrix << endl;
    }

    for (int row = 0; row < rs1_matrix.rows(); row ++)
	   for (int col = 0; col < rs1_matrix.cols(); col ++) {
		  //cout << "rs1_matrix(" << row << "," << col << ")" << "= " << rs1_matrix(row, col) << endl;
		  rd_matrix(row, col) =  exp(rs1_matrix(row, col));
	   }
    if (debug)
        cout << "rd:\n" << rd_matrix << endl;

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

/**
 * vecvt_hf_xu8_m() vecvt.hf.xu8.m
 * 
 * 将矩阵中的元素由 uint8 格式转换为 fp16
 * @param rs1 M1,源操作矩阵基地址
 * @param rd M,目的矩阵基地址
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
int CustomInsns::vecvt_hf_xu8_m(uint8_t *rs1, half *rd, struct ShapeStride *ss)
{
    Map_uint8_t rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));
    
    SHAPE_STRIDE_INFO(ss);
    rd_matrix = rs1_matrix.cast<half>();

    if (debug) {
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rd:" << endl << rd_matrix << endl;
    }

    return 0;
}

/**
 * vecvt_hf_x8_m() vecvt.hf.x8.m
 * 
 * 将矩阵中的元素由 int8 格式转换为 fp16
 * @param rs1 M1,源操作矩阵基地址
 * @param rd M,目的矩阵基地址
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
int CustomInsns::vecvt_hf_x8_m(int8_t *rs1, half *rd, struct ShapeStride *ss)
{
    Map_int8_t rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));
    
    SHAPE_STRIDE_INFO(ss);
    rd_matrix = rs1_matrix.cast<half>();

    if (debug) {
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rd:" << endl << rd_matrix << endl;
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
int CustomInsns::vecvt_hf_x16_m(int16_t *rs1, half *rd, struct ShapeStride *ss)
{
    Map_int16_t rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));
    
    SHAPE_STRIDE_INFO(ss);
    rd_matrix = rs1_matrix.cast<half>();

    if (debug) {
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rd:" << endl << rd_matrix << endl;
    }

    return 0;
}

/**
 * vecvt_hf_xu16_m() vecvt.hf.xu16.m
 * 
 * 将矩阵中的元素由 uint16 格式转换为 fp16
 * @param rs1 M1,源操作矩阵基地址
 * @param rd M,目的矩阵基地址
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
int CustomInsns::vecvt_hf_xu16_m(uint16_t *rs1, half *rd, struct ShapeStride *ss)
{
    Map_uint16_t rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));
    
    SHAPE_STRIDE_INFO(ss);
    rd_matrix = rs1_matrix.cast<half>();

    if (debug) {
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rd:" << endl << rd_matrix << endl;
    }

    return 0;
}

/**
 * vecvt_x8_hf_m() vecvt.x8.hf.m
 * 
 * 将矩阵中的元素由 int8 格式转换为 fp16
 * @param rs1 M1,源操作矩阵基地址
 * @param rd M,目的矩阵基地址
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
int CustomInsns::vecvt_x8_hf_m(half *rs1, int8_t *rd, struct ShapeStride *ss)
{
    Map_half   rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_int8_t rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));
    
    SHAPE_STRIDE_INFO(ss);
    rd_matrix = rs1_matrix.cast<signed char>();

    if (debug) {
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rd:" << endl << rd_matrix << endl;
    }

    return 0;
}

int vecvt_xu8_hf_m(half *rs1, uint8_t *rd, struct ShapeStride *ss)
{
    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_uint8_t rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    SHAPE_STRIDE_INFO(ss);
    rd_matrix = rs1_matrix.cast<unsigned char>();

    if (GLOBAL_DBG) {
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rd:" << endl << rd_matrix << endl;
    }

    return 0;
}

int vecvt_x16_hf_m(half *rs1, short *rd, struct ShapeStride *ss)
{
    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_int16_t rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    SHAPE_STRIDE_INFO(ss);
    rd_matrix = rs1_matrix.cast<short>();

    if (GLOBAL_DBG) {
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rd:" << endl << rd_matrix << endl;
    }

    return 0;
}

int vecvt_f32_hf_m(half *rs1, Float32 *rd, struct ShapeStride *ss)
{
    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_Float32 rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    SHAPE_STRIDE_INFO(ss);
    rd_matrix = rs1_matrix.cast<Float32>();

    if (GLOBAL_DBG) {
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rd:" << endl << rd_matrix << endl;
    }

    return 0;
}

int vecvt_hf_f32_m(Float32 *rs1, half *rd, struct ShapeStride *ss)
{
    Map_Float32 rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    SHAPE_STRIDE_INFO(ss);
    rd_matrix = rs1_matrix.cast<half>();

    if (GLOBAL_DBG) {
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rd:" << endl << rd_matrix << endl;
    }

    return 0;
}

int vecvt_bf_x8_m(int8_t *rs1, Bfloat16 *rd, struct ShapeStride *ss)
{
    Map_int8_t rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_Bfloat16 rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    SHAPE_STRIDE_INFO(ss);
    rd_matrix = rs1_matrix.cast<Bfloat16>();

    if (GLOBAL_DBG) {
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rd:" << endl << rd_matrix << endl;
    }

    return 0;
}

int vecvt_bf_xu8_m(uint8_t *rs1, Bfloat16 *rd, struct ShapeStride *ss)
{
    Map_uint8_t rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_Bfloat16 rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    SHAPE_STRIDE_INFO(ss);
    rd_matrix = rs1_matrix.cast<Bfloat16>();

    if (GLOBAL_DBG) {
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rd:" << endl << rd_matrix << endl;
    }

    return 0;
}

int vecvt_x8_bf_m(Bfloat16 *rs1, int8_t *rd, struct ShapeStride *ss)
{
    Map_Bfloat16 rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_int8_t rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    SHAPE_STRIDE_INFO(ss);
    rd_matrix = rs1_matrix.cast<signed char>();

    if (GLOBAL_DBG) {
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rd:" << endl << rd_matrix << endl;
    }

    return 0;
}

int vecvt_xu8_bf_m(Bfloat16 *rs1, uint8_t *rd, struct ShapeStride *ss)
{
    Map_Bfloat16 rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_uint8_t rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    SHAPE_STRIDE_INFO(ss);
    rd_matrix = rs1_matrix.cast<unsigned char>();

    if (GLOBAL_DBG) {
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rd:" << endl << rd_matrix << endl;
    }

    return 0;
}

int vecvt_x16_bf_m(Bfloat16 *rs1, int16_t *rd, struct ShapeStride *ss)
{
    Map_Bfloat16 rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_int16_t rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    SHAPE_STRIDE_INFO(ss);
    rd_matrix = rs1_matrix.cast<short>();

    if (GLOBAL_DBG) {
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rd:" << endl << rd_matrix << endl;
    }

    return 0;
}

int vecvt_bf_x16_m(int16_t *rs1, Bfloat16 *rd, struct ShapeStride *ss)
{
    Map_int16_t rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_Bfloat16 rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    SHAPE_STRIDE_INFO(ss);
    rd_matrix = rs1_matrix.cast<Bfloat16>();

    if (GLOBAL_DBG) {
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rd:" << endl << rd_matrix << endl;
    }

    return 0;
}

int vecvt_f32_bf_m(Bfloat16 *rs1, Float32 *rd, struct ShapeStride *ss)
{
    Map_Bfloat16 rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_Float32 rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    SHAPE_STRIDE_INFO(ss);
    rd_matrix = rs1_matrix.cast<Float32>();

    if (GLOBAL_DBG) {
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rd:" << endl << rd_matrix << endl;
    }

    return 0;
}

int vecvt_bf_f32_m(Float32 *rs1, Bfloat16 *rd, struct ShapeStride *ss)
{
    Map_Float32 rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_Bfloat16 rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    SHAPE_STRIDE_INFO(ss);
    rd_matrix = rs1_matrix.cast<Bfloat16>();

    if (GLOBAL_DBG) {
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rd:" << endl << rd_matrix << endl;
    }

    return 0;
}

int vecvt_bf_hf_m(half *rs1, Bfloat16 *rd, struct ShapeStride *ss)
{
    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_Bfloat16 rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    SHAPE_STRIDE_INFO(ss);
    rd_matrix = rs1_matrix.cast<Bfloat16>();

    if (GLOBAL_DBG) {
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rd:" << endl << rd_matrix << endl;
    }

    return 0;
}

int vecvt_hf_bf_m(Bfloat16 *rs1, half *rd, struct ShapeStride *ss)
{
    Map_Bfloat16 rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    SHAPE_STRIDE_INFO(ss);
    rd_matrix = rs1_matrix.cast<half>();

    if (GLOBAL_DBG) {
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rd:" << endl << rd_matrix << endl;
    }

    return 0;
}

int vecvt_f32_x32_m(int32_t *rs1, Float32 *rd, struct ShapeStride *ss)
{
    Map_int32_t rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_Float32 rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    SHAPE_STRIDE_INFO(ss);
    rd_matrix = rs1_matrix.cast<Float32>();

    if (GLOBAL_DBG) {
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rd:" << endl << rd_matrix << endl;
    }

    return 0;
}

int vecvt_x32_f32_m(Float32 *rs1, int32_t *rd, struct ShapeStride *ss)
{
    Map_Float32 rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_int32_t rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    SHAPE_STRIDE_INFO(ss);
    rd_matrix = rs1_matrix.cast<int>();

    if (GLOBAL_DBG) {
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rd:" << endl << rd_matrix << endl;
    }

    return 0;
}
