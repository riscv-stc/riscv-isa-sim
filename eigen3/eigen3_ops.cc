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
    printf("conv_s_kernel(0x%x): stride = %d\n", ss->conv_s_kernel, ss->conv_s_kernel & 0xffff);
    printf("conv_kernel(0x%x): kw = %d kh = %d dilation = %d sk = %d\n", ss->conv_kernel,(ss->conv_kernel >> 24) & 0xff,(ss->conv_kernel >> 16) & 0xff,(ss->conv_kernel >> 8) & 0xff, (ss->conv_kernel >> 0) & 0xff);
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
    kw = (ss->conv_kernel >> 24) & 0xff;
    kh = (ss->conv_kernel >> 16) & 0xff;
    dilation = (ss->conv_kernel >> 8) & 0xff;
    sk = (ss->conv_kernel) & 0xff;
    assert(sk > 0 && kw > 0 && kh > 0 && dilation > 0);
    k_stride = ss->conv_s_kernel & 0xffff;
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
 * meconv_x8_mm_base() meconv.x8.mm
 *
 * 标量和矩阵元素广播乘 M = M1 * f
 * @param rs1 M1,源操作矩阵基地址
 * @param rd M,目的矩阵基地址
 * @param rs2 f,源标量操作数
 * @param ss 矩阵形状描述
 * @param outputfp16, 1 output as fp16, 0 output as int32
 * @return 执行结果
 */
int CustomInsns::meconv_x8_mm_base(int8_t *rs1, void *rd, int8_t *rs2, struct ConvShapeStride *ss, int outfp16)
{
    int pad_top, pad_bottom, pad_left, pad_right;
    int kw, kh, okw, okh, k_stride, sk;
    int in_w, in_h, in_c, in_stride;
    int out_w, out_h, out_c, out_stride;
    int w, h, c;
    int dilation;
    int i, j, k, ii, jj, kk;
    int row, col;
    int8_t *rs1_start;
    int8_t *left_val;
    int8_t *start;
    int8_t val;

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
    in_stride = in_stride > 0 ? in_stride : in_c;

    //get the output shape
    out_w = (ss->conv_fm_out >> 16) & 0xffff;
    out_h = (ss->conv_fm_out) & 0xffff;
    out_c = (ss->conv_depth_out) & 0xffff;
    assert(out_w > 0 && out_h > 0 && out_c > 0);
    out_stride = (ss->conv_depth_out >> 16) & 0xffff;
    if (outfp16 == 0) { /*Int 32*/
        assert(out_stride % 4 == 0);
        out_stride = out_stride > 0 ? out_stride >> 2 : out_c;
    }  else {
        assert(out_stride % 2 == 0);
        out_stride = out_stride > 0 ? out_stride >> 1 : out_c;
    }
    

    //get the kernel shape
    kw = (ss->conv_kernel >> 24) & 0xff;
    kh = (ss->conv_kernel >> 16) & 0xff;
    dilation = (ss->conv_kernel >> 8) & 0xff;
    sk = (ss->conv_kernel) & 0xff;
    assert(sk > 0 && kw > 0 && kh > 0 && dilation > 0);
    k_stride = ss->conv_s_kernel & 0xffff ;
    k_stride = k_stride > 0 ? k_stride : out_c;

    /*calculate the kernel shape*/
    Map_int8_t rs2_matrix(rs2, kh * kw * in_c, out_c, DynStride(k_stride, 1)); // the depth is same as in_c

    h = dilation > 1 ? dilation * (kh - 1) + 1 : kh;
    w = dilation > 1 ? dilation * (kw - 1) + 1 : kw;
    okh = kh;
    okw = kw;
    kh = h;
    kw = w;

    /*calculate the input shape*/
    Map_int8_t rs1_matrix(rs1, in_h, in_w * in_c, DynStride(in_stride * in_w, 1));
    h = in_h + (pad_top + pad_bottom);
    w = in_w + (pad_right + pad_left);

    if (debug) {
        MECONV_INFO(ss);
        cout << "rs1:" << endl << rs1_matrix.cast<int32_t>() << endl;
        cout << "rs2:" << endl << rs2_matrix.cast<int32_t>() << endl;
    }


    /*calculate the output shape*/
    h = (in_h + pad_top + pad_bottom - kh + 1 + sk - 1) / sk;
    w = (in_w + pad_left + pad_right - kw + 1 + sk - 1) / sk;
    left_val = (int8_t *)malloc(h * w * okh * okw * in_c * sizeof(int8_t));

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
                                    *(start + start_offset) = 0;
                                    val =  0;
                                }

                                if (debug) {
                                    printf("rd offset= %d val = 0x%x ",start_offset, val);
                                    val = *(start + start_offset);
                                    printf("rd val = 0x%x\n", val);
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
    Map_int8_t left_matrix(left_val, h * w, okh * okw * in_c, DynStride(okh * okw * in_c, 1));
    int32_t *rd_buf;
    int out_stride_int32;
    if (outfp16 == 0) {
        rd_buf =  (int32_t *)rd;
        out_stride_int32 = out_stride;
    } else {
        rd_buf = (int32_t *)malloc(out_h * out_w *out_c * sizeof(int32_t));
        out_stride_int32 = out_c;
    }
    Map_int32_t rd_matrix(rd_buf, out_h * out_w, out_c, DynStride(out_stride_int32, 1));
    rd_matrix = left_matrix.cast<int32_t>() * rs2_matrix.cast<int32_t>();
       
    if (debug)
        cout << "rd:" << endl << rd_matrix << endl;        

    if (outfp16) {
        Map_half rd_fp_matrix((half *)rd, out_h * out_w, out_c, DynStride(out_stride, 1));
        for (int row = 0; row < out_h * out_w; row++)
            for (int col = 0; col < out_c; col++)
                rd_fp_matrix(row, col) = ss->dequant_coeff * rd_matrix(row, col);
        free(rd_buf);
    }

    free(left_val);
    return 0;
}

/**
 * meconv_x8_mm() meconv.x8.mm
 *
 * 标量和矩阵元素广播乘 M = M1 * f
 * @param rs1 M1,源操作矩阵基地址
 * @param rd M,目的矩阵基地址
 * @param rs2 f,源标量操作数
 * @param ss 矩阵形状描述
 * @return 执行结果
 */

int CustomInsns::meconv_x8_mm(int8_t *rs1, int32_t *rd, int8_t *rs2, struct ConvShapeStride *ss)
{
    return meconv_x8_mm_base(rs1, rd, rs2, ss, 0);
}

/**
 * meconv_x8_mm() meconv.x8.mm
 *
 * 标量和矩阵元素广播乘 M = M1 * f
 * @param rs1 M1,源操作矩阵基地址
 * @param rd M,目的矩阵基地址
 * @param rs2 f,源标量操作数
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
int CustomInsns::meconv_hf_x8_mm(int8_t *rs1, half *rd, int8_t *rs2, struct ConvShapeStride *ss)
{
    return meconv_x8_mm_base(rs1, rd, rs2, ss, 1);
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
 * veemacc_mm() veemacc.mm
 * 
 * 矩阵和矩阵元素乘，再所有元素求和 M = sum(M1ij*M2ij)
 * @param rs1 M1,源操作矩阵一基地址
 * @param rs2 M2,源操作矩阵二基地址
 * @param rd s,目的数存放地址
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
int CustomInsns::veemacc_mm(half *rs1, half *rd, half *rs2, struct ShapeStride *ss)
{
    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    Map_half rs2_matrix(rs2, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs2, 1));

    if (debug) {
        SHAPE_STRIDE_INFO(ss);
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rs2:" << endl << rs2_matrix << endl;
    }
    
    //first get the sum of col elment, then get the sum of row.
    rd[0] = ((rs1_matrix.array() * rs2_matrix.array()).colwise().sum()).sum();

    if (debug)
        cout << "rd:" << endl << rd[0] << endl;

    return 0;
}

/**
 * veemacc_mm() veemacc.mm dim = ?
 * 
 * 矩阵和矩阵元素乘，再按照某个方向元素求和
 * @param rs1 M1,源操作矩阵一基地址
 * @param rs2 M2,源操作矩阵二基地址
 * @param rd s,目的向量存放地址
 * @param ss 矩阵形状描述
 * @param dim 方向 dim = 0 v为行向量， dim = 1 v为列向量
 * @return 执行结果
 */
int CustomInsns::veemacc_mm(half *rs1, half *rd, half *rs2, struct ShapeStride *ss, int dim)
{
    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    Map_half rs2_matrix(rs2, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs2, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, 1);
    Map_half vector_dim1(rd, ss->shape1_row, 1, DynStride(ss->stride_rd, 1));
    Map_half vector_dim0(rd, 1, ss->shape1_column, DynStride(1, 1));

    if (debug) {
        SHAPE_STRIDE_INFO(ss);
        cout << "dim: " << dim << endl;
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rs2:" << endl << rs2_matrix << endl;
    }

    switch (dim) {
    case 0:
        vector_dim0 = (rs1_matrix.array() * rs2_matrix.array()).colwise().sum();
        if (debug)
           cout << "rd:" << endl << vector_dim0 << endl;
        break;
    case 1:
        vector_dim1 = (rs1_matrix.array() * rs2_matrix.array()).rowwise().sum();
        if (debug)
           cout << "rd:" << endl << vector_dim1 << endl;
        break;
    default:
        cout << __FUNCTION__ << "error dim" << endl;
        return -BR_EPARAM;
    }
    
    return 0;  
}

/**
 * veemacc_mv() veemacc.mv
 * 
 * 当dim=0时，列向量和矩阵元素广播乘，再列元素(行向量)求和；
 * 当dim=1时，行向量和矩阵元素广播乘，再行元素(列向量)求和
 * @param rs1 M1,源操作矩阵基地址
 * @param rs2 M2,源操作向量基地址
 * @param rd M,目的向量基地址
 * @param ss 矩阵形状描述
 * @param dim 方向 dim = 0 v为行向量， dim = 1 v为列向量
 * @return 执行结果
 */
int CustomInsns::veemacc_mv(half *rs1, half *rd, half *rs2, struct ShapeStride *ss, int dim)
{
    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    Map_half vec_rs2_dim1(rs2, 1, ss->shape1_column, DynStride(1, 1));
    Map_half vec_rs2_dim0(rs2, ss->shape1_row, 1, DynStride(1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, 1);
    Map_half vec_rd_dim1(rd, ss->shape1_row, 1, DynStride(ss->stride_rd, 1));
    Map_half vec_rd_dim0(rd, 1, ss->shape1_column, DynStride(1, 1));
    Matrix_half rd_matrix(ss->shape1_row, ss->shape1_column);

    switch (dim) {
    case 0:
        if (debug) {
            SHAPE_STRIDE_INFO(ss);
            cout << "rs1:" << endl << rs1_matrix << endl;
            cout << "rs2:" << endl << vec_rs2_dim0 << endl;
        }

        for (int col = 0; col < rs1_matrix.cols(); col++)
            rd_matrix.col(col) = rs1_matrix.col(col).array() * vec_rs2_dim0.array();
        vec_rd_dim0 = rd_matrix.colwise().sum();

        if (debug) {
            cout << "mul:" << endl << rd_matrix << endl;
            cout << "rd:" << endl << vec_rd_dim0 << endl;
        }
        break;
    case 1:
        if (debug) {
            SHAPE_STRIDE_INFO(ss);
            cout << "rs1:" << endl << rs1_matrix << endl;
            cout << "rs2:" << endl << vec_rs2_dim1 << endl;
        }

        for (int row = 0; row < rs1_matrix.rows(); row++)
            rd_matrix.row(row) = rs1_matrix.row(row).array() * vec_rs2_dim1.array();
        vec_rd_dim1 = rd_matrix.rowwise().sum();
        
        if (debug) {
            cout << "mul:" << endl << rd_matrix << endl;
            cout << "rd:" << endl << vec_rd_dim1 << endl;
        }
        break;
    default:
        cout << __FUNCTION__ << " error dim" << endl;
        return -BR_EPARAM;
    }
    return 0;
}

/**
 * veemacc_mf() veemacc.mf
 * 
 * 当dim=0时，浮点标量和矩阵元素广播乘，再列元素(行向量)求和；
 * 当dim=1时，浮点标量和矩阵元素广播乘，再行元素(列向量)求和
 * @param rs1 M1,源操作矩阵基地址
 * @param rs2 M2,源操作向量基地址
 * @param rd M,目的向量基地址
 * @param ss 矩阵形状描述
 * @param dim 方向 dim = 0 v为行向量， dim = 1 v为列向量
 * @return 执行结果
 */
int CustomInsns::veemacc_mf(half *rs1, half *rd, half rs2, struct ShapeStride *ss, int dim)
{
    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, 1);
    Map_half vec_rd_dim1(rd, ss->shape1_row, 1, DynStride(ss->stride_rd, 1));
    Map_half vec_rd_dim0(rd, 1, ss->shape1_column, DynStride(1, 1));

    if (debug) {
        SHAPE_STRIDE_INFO(ss);
        cout << "dim: " << dim << endl;
        cout << "rs1:\n" << rs1_matrix << endl;
        cout << "rs2:\n" << rs2 << endl;
    }

    switch (dim) {
    case 0:
        vec_rd_dim0 = (rs1_matrix.array() * rs2).colwise().sum();
        if (debug)
            cout << "rd:\n" << vec_rd_dim0 << endl;
        break;
    case 1:
        vec_rd_dim1 = (rs1_matrix.array() * rs2).rowwise().sum();
        if (debug)
            cout << "rd:\n" << vec_rd_dim1 << endl;
        break;
    default:
        cout << __FUNCTION__ << " error dim" << endl;
        return -BR_EPARAM;
    }
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
            even.v = 0;
            odd.v = 0;
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
 * memul_x8_mm() memul.x8.mm
 * 
 * 矩阵和矩阵算术乘，正常算术运算 M = M1.M2
 * 源操作矩阵一的列值必须和源操作矩阵二的行值相等，如果不等则直接返回错误
 * @param rs1 M1,源操作矩阵一基地址
 * @param rd M,目的矩阵基地址
 * @param rs2 M2,源操作矩阵二基地址
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
int CustomInsns::memul_x8_mm(char *rs1, char *rs2, int *rd, struct ShapeStride *ss)
{
    /* param check */
    if (ss->shape1_column != ss->shape2_row) {
        cout << __FUNCTION__ << ": shape1_column must equal shape2_row" << endl;
        return -BR_EPARAM;
    }

    Map_int8_t rs1_matrix((int8_t *)rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    Map_int8_t rs2_matrix((int8_t *)rs2, ss->shape2_row, ss->shape2_column, DynStride(ss->stride_rs2, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape2_column);
    Map_int32_t rd_matrix((int32_t *)rd, ss->shape1_row, ss->shape2_column, DynStride(ss->stride_rd, 1));

    if (debug) {
        SHAPE_STRIDE_INFO(ss);
        cout << "rs1:\n" << rs1_matrix << endl;
        cout << "rs2:\n" << rs2_matrix << endl;
    }

    /* dot only support vector not support matrix, so we use '*' to do calculation */
    rd_matrix = rs1_matrix.cast<int32_t>() * rs2_matrix.cast<int32_t>();
    if (debug)
        cout << "rd:\n" << rd_matrix << endl;

    return 0;
}

/**
 * memul_hf_x8_mm() memul.x8.mm
 *
 * 矩阵和矩阵算术乘，正常算术运算 M = M1.M2
 * 源操作矩阵一的列值必须和源操作矩阵二的行值相等，如果不等则直接返回错误
 * @param rs1 M1,源操作矩阵一基地址
 * @param rd M,目的矩阵基地址
 * @param rs2 M2,源操作矩阵二基地址
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
int CustomInsns::memul_hf_x8_mm(char *rs1, char *rs2, half *rd, struct ShapeStride *ss, half dequant_coeff)
{
    /* param check */
    if (ss->shape1_column != ss->shape2_row) {
        cout << __FUNCTION__ << ": shape1_column must equal shape2_row" << endl;
        return -BR_EPARAM;
    }

    Map_int8_t rs1_matrix((int8_t *)rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    Map_int8_t rs2_matrix((int8_t *)rs2, ss->shape2_row, ss->shape2_column, DynStride(ss->stride_rs2, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape2_column);

    int32_t *rd_buf = (int32_t *)malloc(ss->shape2_column * ss->shape1_row * sizeof(int32_t));
    Map_int32_t rd_matrix(rd_buf, ss->shape1_row, ss->shape2_column, DynStride(ss->shape2_column , 1));

    if (debug) {
        SHAPE_STRIDE_INFO(ss);
        cout << "rs1:\n" << rs1_matrix << endl;
        cout << "rs2:\n" << rs2_matrix << endl;
    }

    /* dot only support vector not support matrix, so we use '*' to do calculation */
    rd_matrix = rs1_matrix.cast<int32_t>() * rs2_matrix.cast<int32_t>();

    Map_half rd_fp_matrix(rd, ss->shape1_row, ss->shape2_column, DynStride(ss->stride_rd , 1));
    for (int row = 0; row < ss->shape1_row; row++)
        for (int col = 0; col < ss->shape2_column; col++)
            rd_fp_matrix(row, col) = dequant_coeff * rd_matrix(row, col);
    free(rd_buf);

    if (debug)
        cout << "rd:\n" << rd_fp_matrix << endl;

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
    for (int row = 0; row < ss->shape1_row; row++)
    {
        for (int col = 0; col < ss->shape1_column; col++)
	{
	    if (rs1_matrix(row, col) > (half)127)
		rd_matrix(row, col) = 127;
	    else if (rs1_matrix(row, col) < (half)-128)
		rd_matrix(row, col) = -128;
	    else
		rd_matrix(row, col) = (int8_t)rs1_matrix(row, col);
	}
    }

    if (debug) {
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rd:" << endl << rd_matrix << endl;
    }

    return 0;
}


