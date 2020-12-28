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
/* Matrix_uint8_t     Map_uint8_t */
MY_MATRIX_DEFINE(uint8_t)
MY_MATRIX_DEFINE(int8_t)
/* Matrix_uint16_t     Map_uint16_t */
MY_MATRIX_DEFINE(uint16_t)
MY_MATRIX_DEFINE(int16_t)

MY_MATRIX_DEFINE(uint32_t)
MY_MATRIX_DEFINE(int32_t)

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

  EIGEN_DEVICE_FUNC Float32::operator half() const {
      float32_t f32t;
      f32t.v = x;
      float16_t f16t = f32_to_f16(f32t);
      half res;
      res.x = f16t.v;
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
    return 0;
}



/**
 * veemul_mf() veemul.mf
 * 
 * 标量和矩阵元素广播乘 M = M1 * f
 * @param rs1 M1,源操作矩阵基地址
 * @param rd M,目的矩阵基地址
 * @param rs2 f,源标量操作数
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
int CustomInsns::veemul_mf(half *rs1, half *rd, half rs2, struct ShapeStride *ss)
{
    /* param check */

    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    if (debug) {
        SHAPE_STRIDE_INFO(ss);
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rs2:" << endl << rs2 << endl;
    }

    rd_matrix = rs1_matrix * rs2;

    if (debug)
        cout << "rd:" << endl << rd_matrix << endl;

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
 * veemul_mm() veemul.mm
 * 
 * 矩阵和矩阵元素乘 M = M1*M2
 * stride 一致的情况下运算还可以原地进行, 即rd = rs1 或 rd = rs2
 * @param rs1 M1,源操作矩阵一基地址
 * @param rs2 M2,源操作矩阵二基地址
 * @param rd M,目的矩阵基地址
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
int CustomInsns::veemul_mm(half *rs1, half *rd, half *rs2, struct ShapeStride *ss)
{
    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    Map_half rs2_matrix(rs2, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs2, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    if (debug) {
        SHAPE_STRIDE_INFO(ss);
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rs2:" << endl << rs2_matrix << endl;
    }

    rd_matrix = rs1_matrix.array() * rs2_matrix.array();

    if (debug)
        cout << "rd:" << endl << rd_matrix << endl;

    return 0;  
}

/**
 * veemul_mv() veemul.mv
 * 
 * 向量和矩阵元素广播乘 M = V*M1
 * @param rs1 M1,源操作矩阵基地址
 * @param rs2 M2,源操作向量基地址
 * @param rd M,目的矩阵基地址
 * @param ss 矩阵形状描述
 * @param dim 方向 dim = 0 v为行向量， dim = 1 v为列向量
 * @return 执行结果
 */
int CustomInsns::veemul_mv(half *rs1, half *rd, half *rs2, struct ShapeStride *ss, int dim)
{
    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));
    Map_half vector_dim1(rs2, ss->shape1_row, 1, DynStride(1, 1));
    Map_half vector_dim0(rs2, 1, ss->shape1_column, DynStride(1, 1));
    
    if (debug) {
        SHAPE_STRIDE_INFO(ss);
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rs2:" << endl << vector_dim0 << endl;
    }

    switch (dim) {
    case 0:
        for (int row = 0; row < rs1_matrix.rows(); row++)
            rd_matrix.row(row) = rs1_matrix.row(row).array() * vector_dim0.array();
        if (debug)
            cout << "rd:" << endl << rd_matrix << endl;
        break;
    case 1:
        for (int col = 0; col < rs1_matrix.cols(); col++)
            rd_matrix.col(col) = rs1_matrix.col(col).array() * vector_dim1.array();
        if (debug)
            cout << "rd:" << endl << rd_matrix << endl;
        break;
    default:
        cout << __FUNCTION__ << "error dim" << endl;
        return -BR_EPARAM;
    }
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
 * vemax_m() vemax.m
 * 
 * 矩阵列元素(行向量)求最大值s=max(M1i)， 矩阵行元素(列向量)求最大值s=max(M1j)
 * @param rs1 M1,源操作矩阵基地址
 * @param rd V,目的向量基地址
 * @param ss 矩阵形状描述
 * @param dim 方向 dim = 0 列向量求最大值， dim = 1 行向量求最大值
 * @return 执行结果
 */
int CustomInsns::vemax_m(half *rs1, half *rd, struct ShapeStride *ss, int dim)
{
    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    Map_half rd_col_max(rd, 1, ss->shape1_column, DynStride(1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, 1);
    Map_half rd_row_max(rd, ss->shape1_row, 1, DynStride(ss->stride_rd, 1));
    
    if (debug) {
        SHAPE_STRIDE_INFO(ss);
        cout << "rs1:" << endl << rs1_matrix << endl;
    }

    switch (dim) {
    case 0:
        rd_col_max = rs1_matrix.colwise().maxCoeff();
        if (debug)
            cout << "rd:" << endl << rd_col_max << endl;
        break;
    case 1:
        rd_row_max = rs1_matrix.rowwise().maxCoeff();
        if (debug)
            cout << "rd:" << endl << rd_row_max << endl;
        break;
    default:
        cout << __FUNCTION__ << "error dim" << endl;
        return -BR_EPARAM;
    }
    return 0;
}

/**
 * vemax_m() vemax.m
 * 
 * 矩阵所有元素求最大值
 * @param rs1 M1,源操作矩阵基地址
 * @param rd V,目的数基地址
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
int CustomInsns::vemax_m(half *rs1, half *rd, struct ShapeStride *ss)
{
    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    
    if (debug) {
        SHAPE_STRIDE_INFO(ss);
        cout << "rs1:" << endl << rs1_matrix << endl;
    }

    *rd = rs1_matrix.maxCoeff();
    return 0;
}

/**
 * vemax_mm() vemax.mm
 * 
 * 两个矩阵对应元素间求最大值 M = max(M1, M2)
 * stride 一致的情况下运算还可以原地进行, 即rd = rs1 或 rd = rs2
 * @param rs1 M1,源操作矩阵一基地址
 * @param rs2 M2,源操作矩阵二基地址
 * @param rd M,目的矩阵基地址
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
int CustomInsns::vemax_mm(half *rs1, half *rd, half *rs2, struct ShapeStride *ss)
{
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);

    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    Map_half rs2_matrix(rs2, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs2, 1));
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    if (debug) {
        SHAPE_STRIDE_INFO(ss);
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rs2:" << endl << rs2_matrix << endl;
    }

    rd_matrix = (rs1_matrix.array() > rs2_matrix.array()).select(rs1_matrix, rs2_matrix);

    if (debug)
        cout << "rd:" << endl << rd_matrix << endl;

    return 0;   
}

/**
 * vemax_mf() vemax.mf
 * 
 * 矩阵元素与标量比较求最大值 M = max(M1ij, f)
 * 如果要在原地进行，则rd的stride必须和rs1的stride相同
 * @param rs1 M1,源操作矩阵基地址
 * @param rd M,目的矩阵基地址
 * @param rs2 f,源标量浮点操作数
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
int CustomInsns::vemax_mf(half *rs1, half *rd, half rs2, struct ShapeStride *ss)
{

    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    if (debug) {
        SHAPE_STRIDE_INFO(ss);
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rs2:" << endl << rs2 << endl;
    }

    rd_matrix = (rs1_matrix.array() > rs2).select(rs1_matrix, rs2);

    if (debug)
        cout << "rd:" << endl << rd_matrix << endl;

    return 0;
}

/**
 * vemax_mv() vemax.mv
 * 
 * 当dim=0时，矩阵列元素(行向量)与行向量元素比较求最大值 M=max(M1i, V);
 * 当dim=1时，矩阵行元素(列向量)与列向量元素比较求最大值 M=max(M1j, V)
 * @param rs1 M1,源操作矩阵基地址
 * @param rs2 M2,源操作向量基地址
 * @param rd M,目的矩阵基地址
 * @param ss 矩阵形状描述
 * @param dim 方向 dim = 0 v为行向量， dim = 1 v为列向量
 * @return 执行结果
 */
int CustomInsns::vemax_mv(half *rs1, half *rd, half *rs2, struct ShapeStride *ss, int dim)
{
    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));
    Map_half vector_dim1(rs2, ss->shape1_row, 1, DynStride(1, 1));
    Map_half vector_dim0(rs2, 1, ss->shape1_column, DynStride(1, 1));
    
    switch (dim) {
    case 0:
        if (debug) {
            SHAPE_STRIDE_INFO(ss);
            cout << "rs1:" << endl << rs1_matrix << endl;
            cout << "rs2:" << endl << vector_dim0 << endl;
        }

        for (int row = 0; row < rs1_matrix.rows(); row++)
            rd_matrix.row(row) = (rs1_matrix.row(row).array() > vector_dim0.array()).select(
                rs1_matrix.row(row), vector_dim0);
        
        if (debug)
            cout << "rd:" << endl << rd_matrix << endl;
        break;
    case 1:
        if (debug) {
            SHAPE_STRIDE_INFO(ss);
            cout << "rs1:" << endl << rs1_matrix << endl;
            cout << "rs2:" << endl << vector_dim1 << endl;
        }

        for (int col = 0; col < rs1_matrix.cols(); col++)
            rd_matrix.col(col) = (rs1_matrix.col(col).array() > vector_dim1.array()).select(
                rs1_matrix.col(col), vector_dim1);
        
        if (debug)
            cout << "rd:" << endl << rd_matrix << endl;

        break;
    default:
        cout << __FUNCTION__ << "error dim" << endl;
        return -BR_EPARAM;
    }
    return 0;
}

/**
 * vemin_m() vemin.m
 * 
 * 矩阵列元素(行向量)求最小值s=min(M1i)， 矩阵行元素(列向量)求最小值s=min(M1j)
 * @param rs1 M1,源操作矩阵基地址
 * @param rd V,目的向量基地址
 * @param ss 矩阵形状描述
 * @param dim 方向 dim = 0 列向量求最小值， dim = 1 行向量求最小值
 * @return 执行结果
 */
int CustomInsns::vemin_m(half *rs1, half *rd, struct ShapeStride *ss, int dim)
{
    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    Map_half rd_col_max(rd, 1, ss->shape1_column, DynStride(1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, 1);
    Map_half rd_row_max(rd, ss->shape1_row, 1, DynStride(ss->stride_rd, 1));
    
    if (debug) {
        SHAPE_STRIDE_INFO(ss);
        cout << "rs1:" << endl << rs1_matrix << endl;
    }

    switch (dim) {
    case 0:
        rd_col_max = rs1_matrix.colwise().minCoeff();
        if (debug)
            cout << "rd:" << endl << rd_col_max << endl;
        break;
    case 1:
        rd_row_max = rs1_matrix.rowwise().minCoeff();
        if (debug)
            cout << "rd:" << endl << rd_row_max << endl;
        break;
    default:
        cout << __FUNCTION__ << "error dim" << endl;
        return -BR_EPARAM;
    }
    return 0;
}

/**
 * vemin_m() vemin.m
 * 
 * 矩阵所有元素求最小值
 * @param rs1 M1,源操作矩阵基地址
 * @param rd V,目的数基地址
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
int CustomInsns::vemin_m(half *rs1, half *rd, struct ShapeStride *ss)
{
    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    
    if (debug) {
        SHAPE_STRIDE_INFO(ss);
        cout << "rs1:" << endl << rs1_matrix << endl;
    }

    *rd = rs1_matrix.minCoeff();
    return 0;
}

/**
 * vemin_mm() vemin.mm
 * 
 * 两个矩阵对应元素间求最小值 M = min(M1, M2)
 * stride 一致的情况下运算还可以原地进行, 即rd = rs1 或 rd = rs2
 * @param rs1 M1,源操作矩阵一基地址
 * @param rs2 M2,源操作矩阵二基地址
 * @param rd M,目的矩阵基地址
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
int CustomInsns::vemin_mm(half *rs1, half *rd, half *rs2, struct ShapeStride *ss)
{
    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    Map_half rs2_matrix(rs2, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs2, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    if (debug) {
        SHAPE_STRIDE_INFO(ss);
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rs2:" << endl << rs2_matrix << endl;
        cout << "rd:" << endl << rd_matrix << endl;
    }

    rd_matrix = (rs1_matrix.array() < rs2_matrix.array()).select(rs1_matrix, rs2_matrix);

    if (debug)
        cout << "rd:" << endl << rd_matrix << endl;

    return 0;   
}

/**
 * vemin_mf() vemin.mf
 * 
 * 矩阵元素与标量比较求最小值 M = min(M1ij, f)
 * 如果要在原地进行，则rd的stride必须和rs1的stride相同
 * @param rs1 M1,源操作矩阵基地址
 * @param rd M,目的矩阵基地址
 * @param rs2 f,源标量浮点操作数
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
int CustomInsns::vemin_mf(half *rs1, half *rd, half rs2, struct ShapeStride *ss)
{

    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    if (debug) {
        SHAPE_STRIDE_INFO(ss);
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rs2:" << endl << rs2 << endl;
    }

    rd_matrix = (rs1_matrix.array() < rs2).select(rs1_matrix, rs2);

    if (debug)
        cout << "rd:" << endl << rd_matrix << endl;

    return 0;
}

/**
 * vemin_mv() vemin.mv
 * 
 * 当dim=0时，矩阵列元素(行向量)与行向量元素比较求最小值 M=min(M1i, V);
 * 当dim=1时，矩阵行元素(列向量)与列向量元素比较求最小值 M=min(M1j, V)
 * @param rs1 M1,源操作矩阵基地址
 * @param rs2 M2,源操作向量基地址
 * @param rd M,目的矩阵基地址
 * @param ss 矩阵形状描述
 * @param dim 方向 dim = 0 v为行向量， dim = 1 v为列向量
 * @return 执行结果
 */
int CustomInsns::vemin_mv(half *rs1, half *rd, half *rs2, struct ShapeStride *ss, int dim)
{
    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));
    Map_half vector_dim1(rs2, ss->shape1_row, 1, DynStride(1, 1));
    Map_half vector_dim0(rs2, 1, ss->shape1_column, DynStride(1, 1));
    
    switch (dim) {
    case 0:
        if (debug) {
            SHAPE_STRIDE_INFO(ss);
            cout << "rs1:" << endl << rs1_matrix << endl;
            cout << "rs2:" << endl << vector_dim0 << endl;
        }

        for (int row = 0; row < rs1_matrix.rows(); row++)
            rd_matrix.row(row) = (rs1_matrix.row(row).array() < vector_dim0.array()).select(
                rs1_matrix.row(row), vector_dim0);
        break;
    case 1:
        if (debug) {
            SHAPE_STRIDE_INFO(ss);
            cout << "rs1:" << endl << rs1_matrix << endl;
            cout << "rs2:" << endl << vector_dim1 << endl;
        }

        for (int col = 0; col < rs1_matrix.cols(); col++)
            rd_matrix.col(col) = (rs1_matrix.col(col).array() < vector_dim1.array()).select(
                rs1_matrix.col(col), vector_dim1);
        break;
    default:
        cout << __FUNCTION__ << "error dim" << endl;
        return -BR_EPARAM;
    }
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
 * vemul_mm() vemul.mm
 * 
 * 矩阵和矩阵算术乘，正常算术运算 M = M1.M2
 * 源操作矩阵一的列值必须和源操作矩阵二的行值相等，如果不等则直接返回错误
 * @param rs1 M1,源操作矩阵一基地址
 * @param rd M,目的矩阵基地址
 * @param rs2 M2,源操作矩阵二基地址
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
int CustomInsns::vemul_mm(half *rs1, half *rs2, half *rd, struct ShapeStride *ss)
{
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
    rd_matrix = rs1_matrix * rs2_matrix;
    if (debug)
        cout << "rd:\n" << rd_matrix << endl;

    return 0;
}

/**
 * vemul_mv() vemul.mv
 * 
 * 向量和矩阵算数乘 V = V1.M1
 * @param rs1 M1,源操作矩阵基地址
 * @param rd V,目的向量基地址
 * @param rs2 V1,源操作向量基地址
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
int CustomInsns::vemul_mv(half *rs1, half *rs2, half *rd, struct ShapeStride *ss)
{
    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    Map_half rs2_vector(rs2, 1, ss->shape1_row, DynStride(1, 1));
    Map_half rd_vector(rd, 1, ss->shape1_column, DynStride(1, 1));

    if (debug) {
        SHAPE_STRIDE_INFO(ss);
        cout << "rs1:\n" << rs1_matrix << endl;
        cout << "rs2:\n" << rs2_vector << endl;
    }

    /* dot only support vector not support matrix, so we use '*' to do calculation */
    rd_vector = rs2_vector * rs1_matrix;
    if (debug)
        cout << "rd:\n" << rd_vector << endl;

    return 0;
}

/**
 * velkrelu_mf() velkrelu.mf
 * 
 * 矩阵元素与0比较，小于标量则乘一常数系数
 * @param rs1 M1,源操作矩阵基地址
 * @param rs2 k, 源标量浮点操作数
 * @param rd V,目的矩阵基地址
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
int CustomInsns::velkrelu_mf(half *rs1, half rs2, half *rd, struct ShapeStride *ss)
{
    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    if (debug) {
        SHAPE_STRIDE_INFO(ss);
        cout << "rs1:\n" << rs1_matrix << endl;
    }

    rd_matrix = (rs1_matrix.array() <= (half)0).select(rs1_matrix * rs2, rs1_matrix);
    
    if (debug)
        cout << "rd:\n" << rd_matrix << endl;

    return 0;
}

/**
 * velkrelu_mv() velkrelu.mv
 * 
 * 当dim = 0 时，矩阵行元素与0比较，小于标量则乘以一常数行向量；
 * 当dim = 1时，矩阵列元素与0比较，小于标量则乘以一常数列向量
 * @param rs1 M1,源操作矩阵基地址
 * @param rs2 M2,源操作向量基地址
 * @param rd M,目的矩阵基地址
 * @param ss 矩阵形状描述
 * @param dim 方向 dim = 0 v为行向量， dim = 1 v为列向量
 * @return 执行结果
 */
int CustomInsns::velkrelu_mv(half *rs1, half *rd, half *rs2, struct ShapeStride *ss, int dim)
{
    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));
    Map_half vector_dim1(rs2, ss->shape1_row, 1, DynStride(1, 1));
    Map_half vector_dim0(rs2, 1, ss->shape1_column, DynStride(1, 1));

    if (debug) {
        SHAPE_STRIDE_INFO(ss);
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "dim: " << dim << endl;
    }

    switch (dim) {
    case 0:
        for (int row = 0; row < rs1_matrix.rows(); row++)
            rd_matrix.row(row) = (rs1_matrix.row(row).array() <= (half)0).select(
                rs1_matrix.row(row).array() * vector_dim0.array(),
                rs1_matrix.row(row));
        
        if (debug) {
            cout << "rs2:" << endl << vector_dim0 << endl;
            cout << "rd:" << endl << rd_matrix << endl;
        }
        break;
    case 1:
        for (int col = 0; col < rs1_matrix.cols(); col++)
            rd_matrix.col(col) = (rs1_matrix.col(col).array() <= (half)0).select(
                rs1_matrix.col(col).array() * vector_dim1.array(),
                rs1_matrix.col(col));
        
        if (debug) {
            cout << "rs2:" << endl << vector_dim1 << endl;
            cout << "rd:" << endl << rd_matrix << endl;
        }
        break;
    default:
        cout << __FUNCTION__ << "error dim" << endl;
        return -BR_EPARAM;
    }
    return 0;
}

/**
 * velut_m() velut.m
 * 
 * 以矩阵中所有元素为索引(二进制),查找表中对应的值 M=LUT(M1ij)
 * @param rs1 M1,源操作矩阵基地址
 * @param rs2 A_base, LUT基地址
 * @param rd V,目的矩阵基地址
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
int CustomInsns::velut_m(uint16_t *rs1, unsigned long rs2, half *rd, struct ShapeStride *ss)
{
    Map_uint16_t rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    if (debug) {
        SHAPE_STRIDE_INFO(ss);
        cout << "rs1:" << endl << rs1_matrix << endl;
    }

    for (int i = 0; i < ss->shape1_row; i++)
        for (int j = 0; j < ss->shape1_column; j++)
            rd_matrix(i, j) = *((half *)rs2 + rs1_matrix(i, j));
    
    if (debug)
        cout << "rd:" << endl << rd_matrix << endl;

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

int veavgpool_m(half *rs1, half *rd, struct VmeShapeStride *vss)
{
    DEFINE_MAP_DTYPE(half)
    DEFINE_MAP_DTYPE(Float32)

    //param check
    assert((vss->kw * vss->kh <= 64));
    assert((vss->kw > 0) && (vss->kh) > 0);
    assert((vss->row > 0) && (vss->column > 0) && (vss->cin > 0));
    assert((vss->wout > 0) && (vss->hout > 0));

    VME_SHAPE_STRIDE_INFO(vss);

    // rs1 source
    int row_2d, column_2d;
    row_2d = vss->row * vss->column;
    column_2d = vss->cin;
    Map_half rs1_2d(rs1, row_2d, column_2d, DynStride(vss->ifm_c_stride, 1));

    // rs1 with padding
    int row_2d_padded, column_2d_padded, row_3d_padded, column_3d_padded;
    row_3d_padded = vss->row + vss->n_pad_l + vss->n_pad_r;
    column_3d_padded = vss->column + vss->n_pad_u + vss->n_pad_d;
    row_2d_padded = row_3d_padded * column_3d_padded;
    column_2d_padded = vss->cin;
    half *padded_buf = (half *)malloc(row_2d_padded * column_2d_padded * sizeof(half));
    Map_half rs1_2d_padded(padded_buf, row_2d_padded, column_2d_padded, DynStride(vss->cin, 1));

    // zero padding
    half *padding_buf = (half *)malloc(vss->cin * sizeof(half));
    Map_half padding(padding_buf, 1, vss->cin, DynStride(1, 1));
    padding = padding.Constant(1, vss->cin, half(0));
    PADDING_3D_HW_C(rs1_2d, rs1_2d_padded, padding, row_2d_padded, row_3d_padded, column_3d_padded,
        vss->n_pad_u, vss->n_pad_d, vss->n_pad_l, vss->n_pad_r);

    // to save fetched block
    int row_2d_fetch, column_2d_fetch, stride_fetch;
    row_2d_fetch = vss->kh;
    column_2d_fetch = vss->cin * vss->kw;
    stride_fetch = vss->cin * row_3d_padded;
    half *fb_buf = (half *)malloc(row_2d_fetch * column_2d_fetch * sizeof(half));
    Map_half fetch_block(fb_buf, row_2d_fetch, column_2d_fetch, DynStride(column_2d_fetch, 1));

    // reshape fetched block by using same buf
    int row_block_reshaped, col_block_reshaped;
    row_block_reshaped = vss->kw * vss->kh;
    col_block_reshaped = vss->cin;
    Map_half block_reshaped(fb_buf, row_block_reshaped, col_block_reshaped, DynStride(col_block_reshaped, 1));

    // save (block * 1/n), inner dtype
    Float32 *block_indtype_buf = (Float32 *)malloc(row_block_reshaped * col_block_reshaped * sizeof(Float32));
    Map_Float32 block_avged(block_indtype_buf, row_block_reshaped, col_block_reshaped, DynStride(col_block_reshaped, 1));

    // recip vector 1/n
    half *recip_buf = (half *)malloc(row_block_reshaped * sizeof(half));
    Map_half vec_rescip(recip_buf, row_block_reshaped, 1, DynStride(1, 1));

    // output, flatten to 2d
    int row_2d_out, column_2d_out;
    row_2d_out = vss->hout * vss->wout;
    column_2d_out = vss->cin;
    Map_half rd_2d(rd, row_2d_out, column_2d_out, DynStride(vss->ofm_c_stride, 1));

    // internal result, one row, get rd_row_intype then convert to rd_row
    half *rd_row_buf = (half *)malloc(column_2d_out * sizeof(half));
    Map_half rd_row(rd_row_buf, 1, column_2d_out, DynStride(1, 1));
    Float32 *rd_row_intype_buf = (Float32 *)malloc(column_2d_out * sizeof(Float32));
    Map_Float32 rd_row_intype(rd_row_intype_buf, 1, column_2d_out, DynStride(1, 1));

    int n, row_valid, col_valid;
    int start_row = 0;
    int start_col = 0;
    int rd_row_idx = 0;
    half *recip_tab = (half *)recip_table_half;
    half *fetch_base = padded_buf;
    while (1) {
        // calculate n and creat a 1/n vector
        LINE_WINDOWS_COMMON_LENGTH(start_row, vss->kh, vss->n_pad_u, vss->row, row_valid);
        LINE_WINDOWS_COMMON_LENGTH(start_col, vss->kw, vss->n_pad_l, vss->column, col_valid);
        n = row_valid * col_valid;
        vec_rescip = vec_rescip.Constant(row_block_reshaped, 1, recip_tab[n]);

        // fetch next block and reshape
        Map_half fetch(fetch_base, row_2d_fetch, column_2d_fetch, DynStride(stride_fetch, 1));
        fetch_block = fetch;

        // mul 1/n
        MATRIX_MUL_VEC_V_CONVERT(block_reshaped, vec_rescip, block_avged, row_block_reshaped, col_block_reshaped, Float32);

        // acc and convert back to half, only support odd&even mode
        MATRIX_ACC_DIMH_PARITY(block_avged, rd_row_intype, Float32, row_block_reshaped, col_block_reshaped);
        MATRIX_CAST(rd_row_intype, rd_row, half, 1, column_2d_out);

        // get a row in output
        rd_2d.row(rd_row_idx) = rd_row;

        if (GLOBAL_DBG) {
            std::cout << "n = " << n << std::endl;
            std::cout << "output row index = " << rd_row_idx << std::endl;
            std::cout << "current start point(" << start_row << "," << start_col << ")" << std::endl;
        }

        // move point
        rd_row_idx++;
        start_col += vss->sw;
        if (start_col + vss->kw > row_3d_padded) {
            start_col = 0;
            start_row += vss->sh;
            if (start_row + vss->kh > column_3d_padded)
                break;
        }
        fetch_base = padded_buf + start_col * vss->cin + start_row * row_3d_padded * vss->cin;
    }

    free(rd_row_intype_buf);
    free(rd_row_buf);
    free(recip_buf);
    free(block_indtype_buf);
    free(fb_buf);
    free(padding_buf);
    free(padded_buf);

    return 0;
}

int vemaxpool_m(half *rs1, half *rd, struct VmeShapeStride *vss)
{
    DEFINE_MAP_DTYPE(half)
    DEFINE_MAP_DTYPE(Float32)

    //param check
    assert((vss->kw * vss->kh <= 64));
    assert((vss->kw > 0) && (vss->kh) > 0);
    assert((vss->row > 0) && (vss->column > 0) && (vss->cin > 0));
    assert((vss->wout > 0) && (vss->hout > 0));

    VME_SHAPE_STRIDE_INFO(vss);

    // rs1 source
    int row_2d, column_2d;
    row_2d = vss->row * vss->column;
    column_2d = vss->cin;
    Map_half rs1_2d(rs1, row_2d, column_2d, DynStride(vss->ifm_c_stride, 1));

    // rs1 with padding
    int row_2d_padded, column_2d_padded, row_3d_padded, column_3d_padded;
    row_3d_padded = vss->row + vss->n_pad_l + vss->n_pad_r;
    column_3d_padded = vss->column + vss->n_pad_u + vss->n_pad_d;
    row_2d_padded = row_3d_padded * column_3d_padded;
    column_2d_padded = vss->cin;
    half *padded_buf = (half *)malloc(row_2d_padded * column_2d_padded * sizeof(half));
    Map_half rs1_2d_padded(padded_buf, row_2d_padded, column_2d_padded, DynStride(vss->cin, 1));

    // zero padding
    half *padding_buf = (half *)malloc(vss->cin * sizeof(half));
    Map_half padding(padding_buf, 1, vss->cin, DynStride(1, 1));
    padding = padding.Constant(1, vss->cin, half(0));
    PADDING_3D_HW_C(rs1_2d, rs1_2d_padded, padding, row_2d_padded, row_3d_padded, column_3d_padded,
        vss->n_pad_u, vss->n_pad_d, vss->n_pad_l, vss->n_pad_r);

    // to save fetched block
    int row_2d_fetch, column_2d_fetch, stride_fetch;
    row_2d_fetch = vss->kh;
    column_2d_fetch = vss->cin * vss->kw;
    stride_fetch = vss->cin * row_3d_padded;
    half *fb_buf = (half *)malloc(row_2d_fetch * column_2d_fetch * sizeof(half));
    Map_half fetch_block(fb_buf, row_2d_fetch, column_2d_fetch, DynStride(column_2d_fetch, 1));

    // reshape fetched block by using same buf
    int row_block_reshaped, col_block_reshaped;
    row_block_reshaped = vss->kw * vss->kh;
    col_block_reshaped = vss->cin;
    Map_half block_reshaped(fb_buf, row_block_reshaped, col_block_reshaped, DynStride(col_block_reshaped, 1));

    // output, flatten to 2d
    int row_2d_out, column_2d_out;
    row_2d_out = vss->hout * vss->wout;
    column_2d_out = vss->cin;
    Map_half rd_2d(rd, row_2d_out, column_2d_out, DynStride(vss->ofm_c_stride, 1));

    // internal result, one row, get rd_row_intype then convert to rd_row
    half *rd_row_buf = (half *)malloc(column_2d_out * sizeof(half));
    Map_half rd_row(rd_row_buf, 1, column_2d_out, DynStride(1, 1));

    int start_row = 0;
    int start_col = 0;
    int rd_row_idx = 0;
    half *recip_tab = (half *)recip_table_half;
    half *fetch_base = padded_buf;
    while (1) {
        // fetch next block and reshape
        Map_half fetch(fetch_base, row_2d_fetch, column_2d_fetch, DynStride(stride_fetch, 1));
        fetch_block = fetch;

        // get a row in output
        rd_2d.row(rd_row_idx) = block_reshaped.colwise().maxCoeff();

        if (GLOBAL_DBG) {
            std::cout << "output row index = " << rd_row_idx << std::endl;
            std::cout << "current start point(" << start_row << "," << start_col << ")" << std::endl;
        }

        // move point
        rd_row_idx++;
        start_col += vss->sw;
        if (start_col + vss->kw > row_3d_padded) {
            start_col = 0;
            start_row += vss->sh;
            if (start_row + vss->kh > column_3d_padded)
                break;
        }
        fetch_base = padded_buf + start_col * vss->cin + start_row * row_3d_padded * vss->cin;
    }

    free(rd_row_buf);
    free(fb_buf);
    free(padding_buf);
    free(padded_buf);

    return 0;
}

int vedwconv_mm(half *rs1, half *rs2, half *rd, struct VmeShapeStride *vss)
{
    DEFINE_MAP_DTYPE(half)
    DEFINE_MAP_DTYPE(Float32)

    //param check
    assert((vss->kw * vss->kh <= 64));
    assert((vss->kw > 0) && (vss->kh) > 0);
    assert((vss->row > 0) && (vss->column > 0) && (vss->cin > 0));
    assert((vss->wout > 0) && (vss->hout > 0));

    VME_SHAPE_STRIDE_INFO(vss);

    // rs1 source
    int row_2d, column_2d;
    row_2d = vss->row * vss->column;
    column_2d = vss->cin;
    Map_half rs1_2d(rs1, row_2d, column_2d, DynStride(vss->ifm_c_stride, 1));

    // kernel matrix
    int row_k_2d, col_k_2d;
    row_k_2d = vss->kw * vss->kh;
    col_k_2d = vss->cin;
    Map_half kernel_2d(rs2, row_k_2d, col_k_2d, DynStride(vss->k_c_stride, 1));

    // rs1 with padding
    int row_2d_padded, column_2d_padded, row_3d_padded, column_3d_padded;
    row_3d_padded = vss->row + vss->n_pad_l + vss->n_pad_r;
    column_3d_padded = vss->column + vss->n_pad_u + vss->n_pad_d;
    row_2d_padded = row_3d_padded * column_3d_padded;
    column_2d_padded = vss->cin;
    half *padded_buf = (half *)malloc(row_2d_padded * column_2d_padded * sizeof(half));
    Map_half rs1_2d_padded(padded_buf, row_2d_padded, column_2d_padded, DynStride(vss->cin, 1));

    // zero padding
    half *padding_buf = (half *)malloc(vss->cin * sizeof(half));
    Map_half padding(padding_buf, 1, vss->cin, DynStride(1, 1));
    padding = padding.Constant(1, vss->cin, half(0));
    PADDING_3D_HW_C(rs1_2d, rs1_2d_padded, padding, row_2d_padded, row_3d_padded, column_3d_padded,
        vss->n_pad_u, vss->n_pad_d, vss->n_pad_l, vss->n_pad_r);

    // to save fetched block
    int row_2d_fetch, column_2d_fetch, stride_fetch;
    row_2d_fetch = vss->kh;
    column_2d_fetch = vss->cin * vss->kw;
    stride_fetch = vss->cin * row_3d_padded;
    half *fb_buf = (half *)malloc(row_2d_fetch * column_2d_fetch * sizeof(half));
    Map_half fetch_block(fb_buf, row_2d_fetch, column_2d_fetch, DynStride(column_2d_fetch, 1));

    // reshape fetched block by using same buf
    int row_block_reshaped, col_block_reshaped;
    row_block_reshaped = vss->kw * vss->kh;
    col_block_reshaped = vss->cin;
    Map_half block_reshaped(fb_buf, row_block_reshaped, col_block_reshaped, DynStride(col_block_reshaped, 1));

    // save (block * kernel), inner dtype
    Float32 *block_indtype_buf = (Float32 *)malloc(row_block_reshaped * col_block_reshaped * sizeof(Float32));
    Map_Float32 block_muled(block_indtype_buf, row_block_reshaped, col_block_reshaped, DynStride(col_block_reshaped, 1));

    // output, flatten to 2d
    int row_2d_out, column_2d_out;
    row_2d_out = vss->hout * vss->wout;
    column_2d_out = vss->cin;
    Map_half rd_2d(rd, row_2d_out, column_2d_out, DynStride(vss->ofm_c_stride, 1));

    // internal result, one row, get rd_row_intype then convert to rd_row
    half *rd_row_buf = (half *)malloc(column_2d_out * sizeof(half));
    Map_half rd_row(rd_row_buf, 1, column_2d_out, DynStride(1, 1));
    Float32 *rd_row_intype_buf = (Float32 *)malloc(column_2d_out * sizeof(Float32));
    Map_Float32 rd_row_intype(rd_row_intype_buf, 1, column_2d_out, DynStride(1, 1));

    int row_valid, col_valid;
    int start_row = 0;
    int start_col = 0;
    int rd_row_idx = 0;
    half *fetch_base = padded_buf;
    while (1) {
        // fetch next block and reshape
        Map_half fetch(fetch_base, row_2d_fetch, column_2d_fetch, DynStride(stride_fetch, 1));
        fetch_block = fetch;

        // block * kernel
        MATRIX_MUL_CONVERT(block_reshaped, kernel_2d, block_muled, row_block_reshaped, col_block_reshaped, Float32);

        // acc and convert back to half
        MATRIX_ACC_DIMH_PARITY(block_muled, rd_row_intype, Float32, row_block_reshaped, col_block_reshaped);
        MATRIX_CAST(rd_row_intype, rd_row, half, 1, column_2d_out);

        // get a row in output
        rd_2d.row(rd_row_idx) = rd_row;

        if (GLOBAL_DBG) {
            std::cout << "output row index = " << rd_row_idx << std::endl;
            std::cout << "current start point(" << start_row << "," << start_col << ")" << std::endl;
        }

        // move point
        rd_row_idx++;
        start_col += vss->sw;
        if (start_col + vss->kw > row_3d_padded) {
            start_col = 0;
            start_row += vss->sh;
            if (start_row + vss->kh > column_3d_padded)
                break;
        }
        fetch_base = padded_buf + start_col * vss->cin + start_row * row_3d_padded * vss->cin;
    }

    free(rd_row_intype_buf);
    free(rd_row_buf);
    free(block_indtype_buf);
    free(fb_buf);
    free(padding_buf);
    free(padded_buf);

    return 0;
}
