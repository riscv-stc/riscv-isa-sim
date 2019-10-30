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

#define SHAPE_STRIDE_INFO(ss) do {\
        cout << endl << __FUNCTION__ << endl;\
        shapestride_dbg(ss);\
    } while(0)

#define MECONV_INFO(ss) do {\
        if (debug) {\
            cout << endl << __FUNCTION__ << endl;\
            meconv_dbg(ss);\
        }\
    } while(0)
/**
 * CustomInsns() 构造函数
 * 
 * 默认不开启debug
 */
CustomInsns::CustomInsns(): debug(GLOBAL_DBG)
{
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
 * shapestride_dbg() 打印ShapeStride信息
 * 
 * 用于调试
 * @param ss shapeStride 参数指针
 * @return 无返回值
 */
void CustomInsns::shapestride_dbg(struct ShapeStride *ss)
{
    printf("\nShapeStride:\n");
    printf("shape1: (%d:%d)\n", ss->shape1_row, ss->shape1_column);
    printf("shape2: (%d:%d)\n", ss->shape2_row, ss->shape2_column);
    printf("stride rs1: %d\n", ss->stride_rs1);
    printf("stride rs2: %d\n", ss->stride_rs2);
    printf("stride rd : %d\n\n", ss->stride_rd);
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
    int i, j, k, ii, jj, kk;
    int row, col;
    half *rs1_start;
    half *left_val;
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
    Map_half rs1_matrix(rs1, in_h, in_w * in_c, DynStride(in_stride * in_w, 1));

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
    rd_matrix = left_matrix * rs2_matrix;

    if (debug)
        cout << "rd:" << endl << rd_matrix << endl;

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
int CustomInsns::meconv_x8_mm(const int8_t *rs1, int32_t *rd, const int8_t *rs2, struct ConvShapeStride *ss)
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
    assert(out_stride % 4 == 0);
    out_stride = out_stride > 0 ? out_stride >> 2 : out_c;

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
    Map_int32_t rd_matrix(rd, out_h * out_w, out_c, DynStride(out_stride, 1));
    rd_matrix = left_matrix.cast<int32_t>() * rs2_matrix.cast<int32_t>();

    if (debug)
        cout << "rd:" << endl << rd_matrix << endl;

    return 0;
}

#define STRIDE_DEFAULT
#ifdef STRIDE_DEFAULT
#define SET_DEFAULT_STRIDE(stride, value) do { \
	if (!stride)        \
	    stride = value; \
} while (0)
#else 
#define SET_DEFAULT_STRIDE(stride, value)
#endif

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
    if ((rs1 == rd) && (ss->stride_rs1 != ss->stride_rd)) {
        cout << __FUNCTION__ << ": when rs1 equal rs2, stride_rs1 must equal stride_rd" << endl;
        return -BR_EPARAM;
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

    ss->stride_rs1 = ss->stride_rs1 >> 1;
    Map_int32_t rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    if (debug) {
        SHAPE_STRIDE_INFO(ss);
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rs2:" << endl << rs2 << endl;
    }
    /* param check */
    if ((rs1 == rd) && (ss->stride_rs1 != ss->stride_rd)) {
        cout << __FUNCTION__ << ": when rs1 equal rs2, stride_rs1 must equal stride_rd" << endl;
        return -BR_EPARAM;
    }

    for (int row = 0; row < rs1_matrix.rows(); row++)
        for (int col = 0; col < rs1_matrix.cols(); col++)
            rd_matrix(row, col) = rs2 * rs1_matrix(row, col);
 
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
    /* param check */
    if ((rs1 == rd) && (ss->stride_rs1 != ss->stride_rd)) {
        cout << __FUNCTION__ << ": when rd = rs1, stride_rs1 must equal stride_rd" << endl;
        return -BR_EPARAM;
    }
    if ((rs2 == rd) && (ss->stride_rs2 != ss->stride_rd)) {
        cout << __FUNCTION__ << ": when rd = rs2, stride_rs2 must equal stride_rd" << endl;
        return -BR_EPARAM;
    }

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
    ss->stride_rs1 = ss->stride_rs1 >> 1;
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

    rd[0] = (rs1_matrix.array() * rs2_matrix.array()).sum();

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
 * veadd_mf() veadd.mf
 * 
 * 标量和矩阵元素广播加 M = M1 + f
 * 如果要在原地进行，则rd的stride必须和rs1的stride相同
 * @param rs1 M1,源操作矩阵基地址
 * @param rd M,目的矩阵基地址
 * @param rs2 f,源标量操作数
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
int CustomInsns::veadd_mf(half *rs1, half *rd, half rs2, struct ShapeStride *ss)
{

    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    if (debug) {
        SHAPE_STRIDE_INFO(ss);
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rs2:" << endl << rs2 << endl;
    }
    /* param check */
    if ((rs1 == rd) && (ss->stride_rs1 != ss->stride_rd)) {
        cout << __FUNCTION__ << ": when rs1 equal rs2, stride_rs1 must equal stride_rd" << endl;
        return -BR_EPARAM;
    }

    /* eigen not support matrix + scalar, so we creat a matrix init to const f, to
     * convert this operation to matrix + matrix
     */
    Matrix_half const_matrix(ss->shape1_row, ss->shape1_column);
    const_matrix = const_matrix.Constant(ss->shape1_row, ss->shape1_column, rs2);
    rd_matrix = rs1_matrix + const_matrix;

    if (debug)
        cout << "rd:" << endl << rd_matrix << endl;

    return 0;
}

/**
 * veadd_mm() veadd.mm
 * 
 * 同维度矩阵和矩阵元素加 M = M1 + M2
 * stride 一致的情况下运算还可以原地进行, 即rd = rs1 或 rd = rs2
 * @param rs1 M1,源操作矩阵一基地址
 * @param rs2 M2,源操作矩阵二基地址
 * @param rd M,目的矩阵基地址
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
int CustomInsns::veadd_mm(half *rs1, half *rd, half *rs2, struct ShapeStride *ss)
{
    /* param check */
    if ((rs1 == rd) && (ss->stride_rs1 != ss->stride_rd)) {
        cout << __FUNCTION__ << ": when rd = rs1, stride_rs1 must equal stride_rd" << endl;
        return -BR_EPARAM;
    }
    if ((rs2 == rd) && (ss->stride_rs2 != ss->stride_rd)) {
        cout << __FUNCTION__ << ": when rd = rs2, stride_rs2 must equal stride_rd" << endl;
        return -BR_EPARAM;
    }

    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    Map_half rs2_matrix(rs2, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs2, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    if (debug) {
        SHAPE_STRIDE_INFO(ss);
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rs2:" << endl << rs2_matrix << endl;
    }

    rd_matrix = rs1_matrix + rs2_matrix;

    if (debug)
        cout << "rd:" << endl << rd_matrix << endl;

    return 0;   
}

/**
 * veadd_mv() veadd.mv
 * 
 * 同维度矩阵和矩阵元素加 M = M1 + v
 * @param rs1 M1,源操作矩阵基地址
 * @param rs2 M2,源操作向量基地址
 * @param rd M,目的矩阵基地址
 * @param ss 矩阵形状描述
 * @param dim 方向 dim = 0 v为行向量， dim = 1 v为列向量
 * @return 执行结果
 */
int CustomInsns::veadd_mv(half *rs1, half *rd, half *rs2, struct ShapeStride *ss, int dim)
{
    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, 1);
    Map_half vector_dim1(rs2, ss->shape1_row, 1, DynStride(ss->stride_rd, 1));
    Map_half vector_dim0(rs2, 1, ss->shape1_column, DynStride(1, 1));
    
    switch (dim) {
    case 0:
        if (debug) {
            SHAPE_STRIDE_INFO(ss);
            cout << "rs1:" << endl << rs1_matrix << endl;
            cout << "rs2:" << endl << vector_dim0 << endl;
        }

        for (int row = 0; row < rs1_matrix.rows(); row++)
            rd_matrix.row(row) = rs1_matrix.row(row).array() + vector_dim0.array();
        
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
            rd_matrix.col(col) = rs1_matrix.col(col).array() + vector_dim1.array();
        
        if (debug)
            cout << "rd:" << endl << rd_matrix << endl;
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
    Map_half rd_row_max(rd, ss->shape1_row, 1, DynStride(1, 1));
    
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
    /* param check */
    if ((rs1 == rd) && (ss->stride_rs1 != ss->stride_rd)) {
        cout << __FUNCTION__ << ": when rd = rs1, stride_rs1 must equal stride_rd" << endl;
        return -BR_EPARAM;
    }
    if ((rs2 == rd) && (ss->stride_rs2 != ss->stride_rd)) {
        cout << __FUNCTION__ << ": when rd = rs2, stride_rs2 must equal stride_rd" << endl;
        return -BR_EPARAM;
    }

    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    Map_half rs2_matrix(rs2, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs2, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
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
    /* param check */
    if ((rs1 == rd) && (ss->stride_rs1 != ss->stride_rd)) {
        cout << __FUNCTION__ << ": when rs1 equal rs2, stride_rs1 must equal stride_rd" << endl;
        return -BR_EPARAM;
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
    Map_half rd_row_max(rd, ss->shape1_row, 1, DynStride(1, 1));
    
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
    /* param check */
    if ((rs1 == rd) && (ss->stride_rs1 != ss->stride_rd)) {
        cout << __FUNCTION__ << ": when rd = rs1, stride_rs1 must equal stride_rd" << endl;
        return -BR_EPARAM;
    }
    if ((rs2 == rd) && (ss->stride_rs2 != ss->stride_rd)) {
        cout << __FUNCTION__ << ": when rd = rs2, stride_rs2 must equal stride_rd" << endl;
        return -BR_EPARAM;
    }

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
    /* param check */
    if ((rs1 == rd) && (ss->stride_rs1 != ss->stride_rd)) {
        cout << __FUNCTION__ << ": when rs1 equal rs2, stride_rs1 must equal stride_rd" << endl;
        return -BR_EPARAM;
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
 * vesub_mm() vesub.mm
 * 
 * 同维度矩阵和矩阵元素减 M = M1 - M2
 * stride 一致的情况下运算还可以原地进行, 即rd = rs1 或 rd = rs2
 * @param rs1 M1,源操作矩阵一基地址
 * @param rs2 M2,源操作矩阵二基地址
 * @param rd M,目的矩阵基地址
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
int CustomInsns::vesub_mm(half *rs1, half *rd, half *rs2, struct ShapeStride *ss)
{
    /* param check */
    if ((rs1 == rd) && (ss->stride_rs1 != ss->stride_rd)) {
        cout << __FUNCTION__ << ": when rd = rs1, stride_rs1 must equal stride_rd" << endl;
        return -BR_EPARAM;
    }
    if ((rs2 == rd) && (ss->stride_rs2 != ss->stride_rd)) {
        cout << __FUNCTION__ << ": when rd = rs2, stride_rs2 must equal stride_rd" << endl;
        return -BR_EPARAM;
    }

    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    Map_half rs2_matrix(rs2, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs2, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    if (debug) {
        SHAPE_STRIDE_INFO(ss);
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rs2:" << endl << rs2_matrix << endl;
    }

    /* our half not support operator - (const half& a, const half& b),
     * but can support operator - (const half& a),
     *  so we use (a + -b) to instead
     */
    rd_matrix = rs1_matrix + -rs2_matrix;

    if (debug)
        cout << "rd:" << endl << rd_matrix << endl;

    return 0;   
}

/**
 * vesub_mv() vesub.mv
 * 
 * 同维度矩阵和矩阵元素减 M = M1 - v
 * @param rs1 M1,源操作矩阵基地址
 * @param rs2 M2,源操作向量基地址
 * @param rd M,目的矩阵基地址
 * @param ss 矩阵形状描述
 * @param dim 方向 dim = 0 v为行向量， dim = 1 v为列向量
 * @return 执行结果
 */
int CustomInsns::vesub_mv(half *rs1, half *rd, half *rs2, struct ShapeStride *ss, int dim)
{
    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));
    Map_half vector_dim1(rs2, ss->shape1_row, 1, DynStride(1, 1));
    Map_half vector_dim0(rs2, 1, ss->shape1_column, DynStride(1, 1));

    /* our half not support operator - (const half& a, const half& b),
     * but can support operator - (const half& a),
     *  so we use (a + -b) to instead
     */
    switch (dim) {
    case 0:
        if (debug) {
            SHAPE_STRIDE_INFO(ss);
            cout << "rs1:" << endl << rs1_matrix << endl;
            cout << "rs2:" << endl << vector_dim0 << endl;
        }

        for (int row = 0; row < rs1_matrix.rows(); row++)
            rd_matrix.row(row) = rs1_matrix.row(row).array() + -vector_dim0.array();
        break;
    case 1:
        if (debug) {
            SHAPE_STRIDE_INFO(ss);
            cout << "rs1:" << endl << rs1_matrix << endl;
            cout << "rs2:" << endl << vector_dim1 << endl;
        }

        for (int col = 0; col < rs1_matrix.cols(); col++)
            rd_matrix.col(col) = rs1_matrix.col(col).array() + -vector_dim1.array();
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

    ss->stride_rd = ss->stride_rd >> 1;
    ss->stride_rs1 = ss->stride_rs1 << 1;
    ss->stride_rs2 = ss->stride_rs2 << 1;
    Map_int8_t rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    Map_int8_t rs2_matrix(rs2, ss->shape2_row, ss->shape2_column, DynStride(ss->stride_rs2, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape2_column);
    Map_int32_t rd_matrix(rd, ss->shape1_row, ss->shape2_column, DynStride(ss->stride_rd, 1));

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
int CustomInsns::memul_mm(half *rs1, half *rs2, half *rd, struct ShapeStride *ss, int ts)
{
    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    Map_half rs2_matrix(rs2, ss->shape2_row, ss->shape2_column, DynStride(ss->stride_rs2, 1));

    if (debug) {
        SHAPE_STRIDE_INFO(ss);
        cout << "rs1:\n" << rs1_matrix << endl;
        cout << "rs2:\n" << rs2_matrix << endl;
    }

    if (ts == 1) {
        /* param check */
        if (ss->shape1_row != ss->shape2_row) {
            cout << __FUNCTION__ << ": shape1_column must equal shape2_row" << endl;
            return -BR_EPARAM;
        }

        SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape2_column);
        Map_half rd_matrix(rd, ss->shape1_column, ss->shape2_column, DynStride(ss->stride_rd, 1));
        rd_matrix = rs1_matrix.transpose() * rs2_matrix;

        if (debug)
            cout << "rd:\n" << rd_matrix << endl;
    }

    return 0;
}

/**
 * vesub_mf() vesub.mf
 * 
 * 标量和矩阵元素广播减 M = M1 - f
 * 如果要在原地进行，则rd的stride必须和rs1的stride相同
 * @param rs1 M1,源操作矩阵基地址
 * @param rd M,目的矩阵基地址
 * @param rs2 f,源标量操作数
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
int CustomInsns::vesub_mf(half *rs1, half *rd, half rs2, struct ShapeStride *ss)
{

    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    if (debug) {
        SHAPE_STRIDE_INFO(ss);
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rs2:" << endl << rs2 << endl;
    }
    /* param check */
    if ((rs1 == rd) && (ss->stride_rs1 != ss->stride_rd)) {
        cout << __FUNCTION__ << ": when rs1 equal rs2, stride_rs1 must equal stride_rd" << endl;
        return -BR_EPARAM;
    }

    /* eigen not support matrix + scalar, so we creat a matrix init to const f, to
     * convert this operation to matrix + matrix
     * Our half not support operator - (const half& a, const half& b),
     * but can support operator - (const half& a),
     *  so we use (a + -b) to instead
     */
    Matrix_half const_matrix(ss->shape1_row, ss->shape1_column);
    const_matrix = const_matrix.Constant(ss->shape1_row, ss->shape1_column, -rs2);
    rd_matrix = rs1_matrix + const_matrix;

    if (debug)
        cout << "rd:" << endl << rd_matrix << endl;

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
 * veacc_m() veacc.m
 * 
 * 矩阵列元素(行向量)求和s=sum(M1i)， 矩阵行元素(列向量)求和 s=sum(M1j)
 * @param rs1 M1,源操作矩阵基地址
 * @param rd V,目的向量基地址
 * @param ss 矩阵形状描述
 * @param dim 方向 dim = 0 列向量求和， dim = 1 行向量求和
 * @return 执行结果
 */
int CustomInsns::veacc_m(half *rs1, half *rd, struct ShapeStride *ss, int dim)
{
    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    Map_half rd_col_sum(rd, 1, ss->shape1_column, DynStride(1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, 1);
    Map_half rd_row_sum(rd, ss->shape1_row, 1, DynStride(ss->stride_rd, 1));

    if (debug) {
        SHAPE_STRIDE_INFO(ss);
        cout << "rs1:\n" << rs1_matrix << endl;
        cout << "dim: " << dim << endl;
    }

    switch (dim) {
    case 0:
        rd_col_sum = rs1_matrix.colwise().sum();
        if (debug)
            cout << "rd:\n" << rd_col_sum << endl;
        break;
    case 1:
        rd_row_sum = rs1_matrix.rowwise().sum();
        if (debug)
            cout << "rd:\n" << rd_row_sum << endl;
        break;
    default:
        cout << __FUNCTION__ << "error dim" << endl;
        return -BR_EPARAM;
    }

    return 0;
}

/**
 * veacc_m() veacc.m
 * 
 * 矩阵所有元素求和
 * @param rs1 M1,源操作矩阵基地址
 * @param rd V,目的向量基地址
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
int CustomInsns::veacc_m(half *rs1, half *rd, struct ShapeStride *ss)
{
    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    Map_half rd_col_sum(rd, 1, ss->shape1_column, DynStride(1, 1));

    if (debug) {
        SHAPE_STRIDE_INFO(ss);
        cout << "rs1:\n" << rs1_matrix << endl;
    }

    rd_col_sum = rs1_matrix.colwise().sum();
    *rd = rd_col_sum.sum();

    if (debug)
        cout << "rd:\n" << *rd << endl;

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

    rd_matrix = (rs1_matrix.array() > (half)0).select(rs1_matrix, rs1_matrix * rs2);
    
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
            rd_matrix.row(row) = (rs1_matrix.row(row).array() > (half)0).select(
                rs1_matrix.row(row), 
                rs1_matrix.row(row).array() * vector_dim0.array());
        
        if (debug) {
            cout << "rs2:" << endl << vector_dim0 << endl;
            cout << "rd:" << endl << rd_matrix << endl;
        }
        break;
    case 1:
        for (int col = 0; col < rs1_matrix.cols(); col++)
            rd_matrix.col(col) = (rs1_matrix.col(col).array() > (half)0).select(
                rs1_matrix.col(col),
                rs1_matrix.col(col).array() * vector_dim1.array());
        
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
 * mov_m() mov.m
 * 
 * 将矩阵从一个地方搬移到另一个地方
 * @param rs1 M1,源操作矩阵基地址
 * @param rd V,目的矩阵基地址
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
int CustomInsns::mov_m(half *rs1, half *rd, struct ShapeStride *ss)
{
    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    if (debug) {
        SHAPE_STRIDE_INFO(ss);
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rd:" << endl << rd_matrix << endl;
    }
    
    rd_matrix = rs1_matrix;

    if (debug)
        cout << "rd:" << endl << rd_matrix << endl;

    return 0;
}

/**
 * mov_v() mov.v
 * 
 * 将矩阵从一个地方搬移到另一个地方
 * @param rs1 M1,源操作矩阵基地址
 * @param rd V,目的矩阵基地址
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
int CustomInsns::mov_v(half *rs1, half *rd, struct ShapeStride *ss, int dim)
{
    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape2_column);
    Map_half rd_matrix(rd, ss->shape2_row, ss->shape2_column, DynStride(ss->stride_rd, 1));
    
    if (debug) {
        SHAPE_STRIDE_INFO(ss);
        cout << "rs1:" << endl << rs1_matrix << endl;
    }

    switch (dim) {
        case 0:
            for (int row = 0; row < rd_matrix.rows(); row++)
                rd_matrix.row(row) = rs1_matrix;
            break;
        case 1:
            for (int col = 0; col < rd_matrix.cols(); col++)
                rd_matrix.col(col) = rs1_matrix;
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
 * mov_f() mov.f
 *
 * 将浮点标量寄存器单值复制扩展成一个矩阵
 * @param rs1 标 量 操 作 数
 * @param rd V,目的矩阵基地址
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
int CustomInsns::mov_f(half rs1, half *rd, struct ShapeStride *ss)
{
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    if (debug) {
        SHAPE_STRIDE_INFO(ss);
        cout << "rs1:" << endl << rs1 << endl;
    }

    rd_matrix = rd_matrix.Constant(ss->shape1_row, ss->shape1_column, rs1);

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


