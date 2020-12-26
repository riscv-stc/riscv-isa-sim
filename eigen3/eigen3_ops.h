/**
 * @file eigen3_ops.h
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

#ifndef __EIGEN3_OPS_H__
#define __EIGEN3_OPS_H__

#include <Eigen/Dense>
#include <Eigen/Geometry>
#include <iostream>
#include <bitset>

using namespace Eigen;
using namespace std;



#define GLOBAL_DBG      0
#define DBG_VECTOR_VVM    do {                   \
    if (debug) {                                \
        cout << __FUNCTION__ << endl;           \
        cout << "vs2:\n" << vector_vs2 << endl; \
        cout << "vs1:\n" << vector_vs1 << endl;        \
        cout << "vm:\n" << vm << endl;          \
        cout << "v0:\n" << vector_v0 << endl;   \
        cout << "vd:\n" << vector_vd << endl;   \
    }                                           \
} while(0)

#define DBG_VECTOR_VF    do {                   \
    if (debug) {                                \
        cout << __FUNCTION__ << endl;           \
        cout << "vs2:\n" << vector_vs2 << endl; \
        cout << "rs1:\n" << rs1 << endl;        \
        cout << "vm:\n" << vm << endl;          \
        cout << "v0:\n" << vector_v0 << endl;   \
        cout << "vd:\n" << vector_vd << endl;   \
    }                                           \
} while(0)

#define DBG_VECTOR_VV    do {                   \
    if (debug) {                                \
        cout << __FUNCTION__ << endl;           \
        cout << "vs2:\n" << vector_vs2 << endl; \
        cout << "vs1:\n" << vector_vs1 << endl; \
        cout << "vm:\n" << vm << endl;          \
        cout << "v0:\n" << vector_v0 << endl;   \
        cout << "vd:\n" << vector_vd << endl;   \
    }                                           \
} while(0)

#define DBG_INFO4(flag, arg1, arg2, arg3, arg4) do {  \
    if (flag) {                                       \
        cout << __FUNCTION__ << endl;                 \
        cout << #arg1":\n" << arg1 << endl;           \
        cout << #arg2":\n" << arg2 << endl;           \
        cout << #arg3":\n" << arg3 << endl;           \
        cout << #arg4":\n" << arg4 << endl;           \
    }                                                 \
} while(0)

#define DBG_INFO5(flag, arg1, arg2, arg3, arg4, arg5) do {  \
    if (flag) {                                             \
        cout << __FUNCTION__ << endl;                       \
        cout << #arg1":\n" << arg1 << endl;                 \
        cout << #arg2":\n" << arg2 << endl;                 \
        cout << #arg3":\n" << arg3 << endl;                 \
        cout << #arg4":\n" << arg4 << endl;                 \
        cout << #arg5":\n" << arg5 << endl;                 \
    }                                                       \
} while(0)

#define SET_BIT(number, bit) do {  \
        number |= (0x1 << bit);    \
} while(0)

#define SHAPESTRIDE_DBG(ss) do { \
    printf("\nShapeStride:\n"); \
    printf("shape1: (%d:%d)\n", ss->shape1_row, ss->shape1_column); \
    printf("shape2: (%d:%d)\n", ss->shape2_row, ss->shape2_column); \
    printf("stride rs1: %d\n", ss->stride_rs1); \
    printf("stride rs2: %d\n", ss->stride_rs2); \
    printf("stride rd : %d\n\n", ss->stride_rd); \
} while(0)

#define SHAPE_STRIDE_INFO(ss) do {\
        if (GLOBAL_DBG) {\
           cout << endl << __FUNCTION__ << endl;\
           SHAPESTRIDE_DBG(ss);\
        } \
    } while(0)

#define VME_SHAPE_STRIDE_INFO(vss) do { \
    if (GLOBAL_DBG) { \
        printf("\nVME ShapeStride:\n"); \
        printf("input : (%d:%d:%d) stride(%d)\n", vss->row, vss->column, vss->cin, vss->ifm_c_stride); \
        printf("output: (%d:%d) stride(%d)\n", vss->hout, vss->wout, vss->ofm_c_stride); \
        printf("kernel: (%d:%d) step(%d:%d)\n", vss->kw, vss->kh, vss->sw, vss->sh); \
        printf("padding: (u%d:d%d:l%d:r%d)\n", vss->n_pad_u, vss->n_pad_d, vss->n_pad_l, vss->n_pad_r); \
        printf("k_c_stride: %d\n", vss->k_c_stride); \
    } \
} while(0);

#define STRIDE_DEFAULT
#ifdef STRIDE_DEFAULT
#define SET_DEFAULT_STRIDE(stride, value) do { \
	if (!stride)        \
	    stride = value; \
} while (0)
#else
#define SET_DEFAULT_STRIDE(stride, value)
#endif

typedef Stride<Dynamic, Dynamic> DynStride;

#define DEFINE_MAP_DTYPE(dtype) \
    typedef Matrix<dtype, Dynamic, Dynamic, RowMajor> Matrix_##dtype; \
    typedef Map<Matrix_##dtype, Unaligned, Stride<Dynamic, Dynamic> > Map_##dtype;

#define MATRIX_CAST(src, dest, destDtype, row, column) \
    for (int _row = 0; _row < row; _row++) { \
        for (int _col = 0; _col < column; _col++) { \
            destDtype tmp = destDtype(src(_row, _col)); \
		    dest(_row, _col) = tmp; \
	    } \
    }

#define MATRIX_MUL_CONVERT(src1, src2, dest, row, column, dtype) do { \
    for (int _row = 0; _row < row; _row++) { \
        for (int _col = 0; _col < column; _col++) { \
            dest(_row, _col) = dtype::mulConvert(src1(_row, _col), src2(_row, _col)); \
        } \
    } \
} while(0);

#define MATRIX_MUL_VEC_H_CONVERT(matrix, vec, dest, row, column, dtype) do { \
    for (int _row = 0; _row < row; _row++) { \
        for (int _col = 0; _col < column; _col++) { \
            dest(_row, _col) = dtype::mulConvert(matrix(_row, _col), vec(0, _col)); \
        } \
    } \
} while(0);

#define MATRIX_MUL_VEC_V_CONVERT(matrix, vec, dest, row, column, dtype) do { \
    for (int _row = 0; _row < row; _row++) { \
        for (int _col = 0; _col < column; _col++) { \
            dest(_row, _col) = dtype::mulConvert(matrix(_row, _col), vec(_row, 0)); \
        } \
    } \
} while(0);

#define MATRIX_MUL_SCALA_CONVERT(matrix, f, dest, row, column, dtype) do { \
    for (int _row = 0; _row < row; _row++) { \
        for (int _col = 0; _col < column; _col++) { \
            dest(_row, _col) = dtype::mulConvert(matrix(_row, _col), f); \
        } \
    } \
} while(0);

#define MATRIX_ACC_DIMH_PARITY(src, dest, dtype, row, column) do { \
    for (int _col = 0; _col < column; _col++) { \
        dtype odd_acc = dtype(0); \
        dtype even_acc = dtype(0); \
        for (int _row = 0; _row < row; _row++) { \
            if (_row % 2) \
                even_acc += src(_row, _col); \
            else \
                odd_acc += src(_row, _col); \
        } \
        dest(0, _col) = odd_acc + even_acc; \
    } \
} while(0);

#define MATRIX_ACC_DIMH_4PART(src, dest, dtype, row, column) do { \
    for (int _col = 0; _col < column; _col++) { \
        dtype acc0 = dtype(0); \
        dtype acc1 = dtype(0); \
        dtype acc2 = dtype(0); \
        dtype acc3 = dtype(0); \
        for (int _row = 0; _row < row; _row++) { \
            if ((_row % 4) == 0) \
                acc0 += src(_row, _col); \
            if ((_row % 4) == 1) \
                acc1 += src(_row, _col); \
            if ((_row % 4) == 2) \
                acc2 += src(_row, _col); \
            if ((_row % 4) == 3) \
                acc3 += src(_row, _col); \
        } \
        acc0 += acc2; \
        acc1 += acc3; \
        dest(0, _col) = acc0 + acc1; \
    } \
} while(0);

#define MATRIX_ACC_DIMW_PAIR(src, dest, dtype, row, column) do { \
    dtype *pbuf = new dtype[column]; \
    for (int _row = 0; _row < row; _row++) { \
        for (int _col = 0; _col < column; _col++) { \
            pbuf[_col] = src(_row, _col); \
        } \
        int get_pos = 0; \
        int put_pos = 0; \
        int loop = column; \
        while (loop > 1) { \
            if (get_pos == (loop - 1)) { \
                pbuf[put_pos] = pbuf[get_pos]; \
            } else { \
                pbuf[put_pos] = pbuf[get_pos] + pbuf[get_pos + 1]; \
            } \
            if (get_pos >= (loop - 2)) { \
                loop = loop / 2 + loop % 2; \
                get_pos = 0; \
                put_pos = 0; \
            } else { \
                get_pos += 2; \
                put_pos++; \
            } \
        } \
        dest(_row, 0) = pbuf[0]; \
    } \
    delete[] pbuf; \
} while(0);

#define PADDING_3D_HW_C(src, dest, padding, dest_row_2d, dest_row_3d, dest_column_3d, u, d, l, r) do { \
    unsigned int src_row = 0; \
    for (int _row = 0; _row < dest_row_2d; _row++) { \
        if ((_row < (dest_column_3d * u)) || \
                ((_row % dest_column_3d) < l) || \
                (_row % dest_column_3d >= (dest_column_3d - r)) || \
                (_row > (dest_column_3d * (dest_row_3d - d)))) { \
            dest.row(_row) = padding.row(0); \
        } else { \
            dest.row(_row) = src.row(src_row); \
            src_row++; \
        } \
    } \
} while(0)

#define LINE_WINDOWS_COMMON_LENGTH(pos, line, pre, window, common) do { \
    if ((pos + line) <= pre) \
        common = 0; \
    else if (pos < pre) \
        common = pos + line - pre; \
    else if ((pos + line) < (pre + window)) \
        common = line; \
    else if (pos < (pre + window)) \
        common = window + pre - pos; \
    else \
        common = 0; \
} while(0);

const uint16_t recip_table_half[65] = {
    0x0000,0x3c00,0x3800,0x3555,0x3400,0x3266,0x3155,0x3092,
    0x3000,0x2f1c,0x2e66,0x2dd1,0x2d55,0x2cec,0x2c92,0x2c44,
    0x2c00,0x2b87,0x2b1c,0x2abc,0x2a66,0x2a18,0x29d1,0x2990,
    0x2955,0x291e,0x28ec,0x28bd,0x2892,0x2869,0x2844,0x2821,
    0x2800,0x27c1,0x2787,0x2750,0x271c,0x26eb,0x26bc,0x2690,
    0x2666,0x263e,0x2618,0x25f4,0x25d1,0x25b0,0x2590,0x2572,
    0x2555,0x2539,0x251e,0x2505,0x24ec,0x24d4,0x24bd,0x24a7,
    0x2492,0x247d,0x2469,0x2456,0x2444,0x2432,0x2421,0x2410,
    0x2400
};

/**
 * @brief 矩阵形状描述结构
 *
 * 按照 CSR shape1, shape2, stride1, stride2 进行设计
 * 用于提供输入矩阵和输出矩阵的形状和存储方式的描述
 */
struct ShapeStride
{
    /* CSR shape1 */
    unsigned short shape1_column;
    unsigned short shape1_row;

    /* CSR shape2 */
    unsigned short shape2_column;
    unsigned short shape2_row;

    /* CSR stride1 */
    unsigned short stride_rd;

    /* CSR stride2 */
    unsigned short stride_rs2;
    unsigned short stride_rs1;

    /* CSR sparse index stride*/
    unsigned short stride_idx;

    /* quant_coeff */
    float32_t mme_quant_coeff;
    float32_t mme_dequant_coeff;
};

struct VmeShapeStride
{
    int row, column;
    int ifm_c_stride, cin;
    int wout, hout;
    int ofm_c_stride, k_c_stride;
    int kw, kh, sw, sh;
    int n_pad_u, n_pad_d, n_pad_l, n_pad_r;
};

/**
 * @brief 矩阵形状描述结构
 *
 * 按照 CSR进行设计
 * 用于提供输入矩阵和输出矩阵的形状和存储方式的描述
 */
struct ConvShapeStride
{
    /* CSR FM in */
    unsigned int conv_fm_in;
    unsigned int conv_depth_in;

    /* CSR  FM out*/
    unsigned int conv_fm_out;
    unsigned int conv_depth_out;

    /* CSR kernel */
    unsigned int conv_kernel_params1;
    unsigned int conv_kernel_params2;

    /* CSR padding */
    unsigned int conv_padding;

    /* CSR sparse index stride*/
    unsigned short stride_idx;

    /* quant_coeff */
    float32_t mme_quant_coeff;
    float32_t mme_dequant_coeff;
};

/**
 * @brief 返回值枚举
 */
enum {
    BR_OK = 0,
    BR_EPARAM
};


template <typename DType>
int veadd_mm(DType* rs1, DType* rd, DType* rs2, struct ShapeStride *ss)
{
    DEFINE_MAP_DTYPE(DType)

    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);

    Map_DType rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    Map_DType rs2_matrix(rs2, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs2, 1));
    Map_DType rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    if (GLOBAL_DBG) {
        SHAPE_STRIDE_INFO(ss);
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rs2:" << endl << rs2_matrix << endl;
    }

    rd_matrix = rs1_matrix + rs2_matrix;

    if (GLOBAL_DBG)
        cout << "rd:" << endl << rd_matrix << endl;

    return 0;
}

template <typename DType>
int veadd_mv(DType *rs1, DType *rd, DType *rs2, struct ShapeStride *ss, int dim)
{
    DEFINE_MAP_DTYPE(DType)

    Map_DType rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_DType rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));
    Map_DType vector_dim1(rs2, ss->shape1_row, 1, DynStride(1, 1));
    Map_DType vector_dim0(rs2, 1, ss->shape1_column, DynStride(1, 1));

    switch (dim) {
    case 0:
        if (GLOBAL_DBG) {
            SHAPE_STRIDE_INFO(ss);
            cout << "rs1:" << endl << rs1_matrix << endl;
            cout << "rs2:" << endl << vector_dim0 << endl;
        }

        for (int row = 0; row < rs1_matrix.rows(); row++)
            rd_matrix.row(row) = rs1_matrix.row(row).array() + vector_dim0.array();

        if (GLOBAL_DBG)
            cout << "rd:" << endl << rd_matrix << endl;
        break;
    case 1:
        if (GLOBAL_DBG) {
            SHAPE_STRIDE_INFO(ss);
            cout << "rs1:" << endl << rs1_matrix << endl;
            cout << "rs2:" << endl << vector_dim1 << endl;
        }

        for (int col = 0; col < rs1_matrix.cols(); col++)
            rd_matrix.col(col) = rs1_matrix.col(col).array() + vector_dim1.array();

        if (GLOBAL_DBG)
            cout << "rd:" << endl << rd_matrix << endl;
        break;
    default:
        cout << __FUNCTION__ << " error dim" << endl;
        return -BR_EPARAM;
    }
    return 0;
}

template <typename DType>
int veadd_mf(DType *rs1, DType *rd, DType rs2, struct ShapeStride *ss)
{
    DEFINE_MAP_DTYPE(DType)

    Map_DType rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_DType rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    if (GLOBAL_DBG) {
        SHAPE_STRIDE_INFO(ss);
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rs2:" << endl << rs2 << endl;
    }

    /* eigen not support matrix + scalar, so we creat a matrix init to const f, to
     * convert this operation to matrix + matrix
     */
    Matrix_DType const_matrix(ss->shape1_row, ss->shape1_column);
    const_matrix = const_matrix.Constant(ss->shape1_row, ss->shape1_column, rs2);
    rd_matrix = rs1_matrix + const_matrix;

    if (GLOBAL_DBG)
        cout << "rd:" << endl << rd_matrix << endl;

    return 0;
}

template <typename DType>
int vesub_mm(DType *rs1, DType *rd, DType *rs2, struct ShapeStride *ss)
{
    DEFINE_MAP_DTYPE(DType)

    Map_DType rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    Map_DType rs2_matrix(rs2, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs2, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_DType rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    if (GLOBAL_DBG) {
        SHAPE_STRIDE_INFO(ss);
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rs2:" << endl << rs2_matrix << endl;
    }

    /* our half not support operator - (const half& a, const half& b),
     * but can support operator - (const half& a),
     *  so we use (a + -b) to instead
     */
    rd_matrix = rs1_matrix + -rs2_matrix;

    if (GLOBAL_DBG)
        cout << "rd:" << endl << rd_matrix << endl;

    return 0;
}

template <typename DType>
int vesub_mv(DType *rs1, DType *rd, DType *rs2, struct ShapeStride *ss, int dim)
{
    DEFINE_MAP_DTYPE(DType)

    Map_DType rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_DType rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));
    Map_DType vector_dim1(rs2, ss->shape1_row, 1, DynStride(1, 1));
    Map_DType vector_dim0(rs2, 1, ss->shape1_column, DynStride(1, 1));

    /* our half not support operator - (const half& a, const half& b),
     * but can support operator - (const half& a),
     *  so we use (a + -b) to instead
     */
    switch (dim) {
    case 0:
        if (GLOBAL_DBG) {
            SHAPE_STRIDE_INFO(ss);
            cout << "rs1:" << endl << rs1_matrix << endl;
            cout << "rs2:" << endl << vector_dim0 << endl;
        }

        for (int row = 0; row < rs1_matrix.rows(); row++)
            rd_matrix.row(row) = rs1_matrix.row(row).array() + -vector_dim0.array();
        break;
    case 1:
        if (GLOBAL_DBG) {
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
    if (GLOBAL_DBG)
        cout << "rd:" << endl << rd_matrix << endl;
    return 0;
}

template <typename DType>
int vesub_mf(DType *rs1, DType *rd, DType rs2, struct ShapeStride *ss)
{
    DEFINE_MAP_DTYPE(DType)

    Map_DType rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_DType rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    if (GLOBAL_DBG) {
        SHAPE_STRIDE_INFO(ss);
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rs2:" << endl << rs2 << endl;
    }

    /* eigen not support matrix + scalar, so we creat a matrix init to const f, to
     * convert this operation to matrix + matrix
     * Our half not support operator - (const half& a, const half& b),
     * but can support operator - (const half& a),
     *  so we use (a + -b) to instead
     */
    Matrix_DType const_matrix(ss->shape1_row, ss->shape1_column);
    const_matrix = const_matrix.Constant(ss->shape1_row, ss->shape1_column, -rs2);
    rd_matrix = rs1_matrix + const_matrix;

    if (GLOBAL_DBG)
        cout << "rd:" << endl << rd_matrix << endl;

    return 0;
}

template <typename OutDType, typename InDType>
int veacc_m(OutDType *rs1, OutDType *rd, struct ShapeStride *ss, int dim)
{
    DEFINE_MAP_DTYPE(OutDType)
    DEFINE_MAP_DTYPE(InDType)

    Map_OutDType rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    InDType *rs1_buf = (InDType *)malloc(ss->shape1_row * ss->shape1_column * sizeof(InDType));
    Map_InDType rs1_matrix_inner(rs1_buf, ss->shape1_row, ss->shape1_column, DynStride(ss->shape1_column, 1));

    MATRIX_CAST(rs1_matrix, rs1_matrix_inner, InDType, ss->shape1_row, ss->shape1_column);
    //rs1_matrix_inner = rs1_matrix.cast<_InDType>();
    SET_DEFAULT_STRIDE(ss->stride_rd, 1);

    if (GLOBAL_DBG) {
        SHAPE_STRIDE_INFO(ss);
        cout << "rs1:\n" << rs1_matrix << endl;
        cout << "rsinner:\n" << rs1_matrix_inner << endl;
        cout << "dim: " << dim << endl;
    }

    if (dim == 0) {
        Map_OutDType rd_col_sum(rd, 1, ss->shape1_column, DynStride(1, 1));
        InDType *rd_col_buf = (InDType *)malloc(ss->shape1_column * sizeof(InDType));
        Map_InDType rd_col_sum_inner(rd_col_buf, 1, ss->shape1_column, DynStride(1, 1));
        //rd_col_sum_inner = rs1_matrix_inner.colwise().sum();
        if (ss->shape1_column <= 64 && ss->stride_rs1 == ss->shape1_column) {
            MATRIX_ACC_DIMH_4PART(rs1_matrix_inner, rd_col_sum_inner, InDType, ss->shape1_row, ss->shape1_column);
        } else {
            MATRIX_ACC_DIMH_PARITY(rs1_matrix_inner, rd_col_sum_inner, InDType, ss->shape1_row, ss->shape1_column);
        }

        if (GLOBAL_DBG)
            cout << "rdinner:\n" << rd_col_sum_inner << endl;
        //rd_col_sum = rd_col_sum_inner.cast<OutDType>();
        MATRIX_CAST(rd_col_sum_inner, rd_col_sum, OutDType, 1, ss->shape1_column);
        free(rd_col_buf);
        if (GLOBAL_DBG)
            cout << "rd:\n" << rd_col_sum << endl;
    } else {
        Map_OutDType rd_row_sum(rd, ss->shape1_row, 1, DynStride(ss->stride_rd, 1));
        InDType *rd_row_buf = (InDType *)malloc(ss->shape1_row * sizeof(InDType));
        Map_InDType rd_row_sum_inner(rd_row_buf, ss->shape1_row, 1, DynStride(1, 1));
        //rd_row_sum_inner = rs1_matrix_inner.rowwise().sum();
        MATRIX_ACC_DIMW_PAIR(rs1_matrix_inner, rd_row_sum_inner, InDType, ss->shape1_row, ss->shape1_column);
        //rd_row_sum = rd_row_sum_inner.cast<OutDType>();
        MATRIX_CAST(rd_row_sum_inner, rd_row_sum, OutDType, ss->shape1_row, 1);
        free(rd_row_buf);
        if (GLOBAL_DBG)
            cout << "rd:\n" << rd_row_sum << endl;
    }

    free(rs1_buf);

    return 0;
}

template <typename OutDType, typename InDType>
int veacc_m(OutDType *rs1, OutDType *rd, struct ShapeStride *ss)
{
    DEFINE_MAP_DTYPE(OutDType)
    DEFINE_MAP_DTYPE(InDType)

    Map_OutDType rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    if (GLOBAL_DBG) {
        SHAPE_STRIDE_INFO(ss);
        cout << "rs1:\n" << rs1_matrix << endl;
    }

    InDType *rs1_buf = (InDType *)malloc(ss->shape1_row * ss->shape1_column * sizeof(InDType));
    Map_InDType rs1_matrix_inner(rs1_buf, ss->shape1_row, ss->shape1_column, DynStride(ss->shape1_column, 1));
    MATRIX_CAST(rs1_matrix, rs1_matrix_inner, InDType, ss->shape1_row, ss->shape1_column);
    //rs1_matrix_inner = rs1_matrix.cast<InDType>();

    InDType *pcol_sum = (InDType *)malloc(ss->shape1_column * sizeof(InDType));
    Map_InDType rd_col_sum(pcol_sum, 1, ss->shape1_column, DynStride(1, 1));

    //rd_col_sum = rs1_matrix_inner.colwise().sum();
    if (ss->shape1_column <= 64 && ss->stride_rs1 == ss->shape1_column) {
        MATRIX_ACC_DIMH_4PART(rs1_matrix_inner, rd_col_sum, InDType, ss->shape1_row, ss->shape1_column);
    } else {
        MATRIX_ACC_DIMH_PARITY(rs1_matrix_inner, rd_col_sum, InDType, ss->shape1_row, ss->shape1_column);
    }

    //InDType rd_tmp = rd_col_sum.sum();
    Matrix_InDType rd_acc(1, 1);
    MATRIX_ACC_DIMW_PAIR(rd_col_sum, rd_acc, InDType, 1, ss->shape1_column);
    //*rd = OutDType(rd_tmp);
    *rd = OutDType(rd_acc(0, 0));

    if (GLOBAL_DBG)
        cout << "rd:\n" << *rd << endl;

    free(pcol_sum);
    free(rs1_buf);

    return 0;
}

template <typename OutDType, typename InDType>
int veemacc_mm(OutDType *rs1, OutDType *rd, OutDType *rs2, struct ShapeStride *ss, int dim)
{
    DEFINE_MAP_DTYPE(OutDType)
    DEFINE_MAP_DTYPE(InDType)

    Map_OutDType rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    Map_OutDType rs2_matrix(rs2, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs2, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, 1);

    if (GLOBAL_DBG) {
        SHAPE_STRIDE_INFO(ss);
        cout << "dim: " << dim << endl;
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rs2:" << endl << rs2_matrix << endl;
    }

    InDType *mul_buf = (InDType *)malloc(ss->shape1_row * ss->shape1_column * sizeof(InDType));
    Map_InDType mul_result(mul_buf, ss->shape1_row, ss->shape1_column, DynStride(ss->shape1_column, 1));
    MATRIX_MUL_CONVERT(rs1_matrix, rs2_matrix, mul_result, ss->shape1_row, ss->shape1_column, InDType);

    if (dim == 0) {
        Map_OutDType rd_col_sum(rd, 1, ss->shape1_column, DynStride(1, 1));
        InDType *rd_col_buf = (InDType *)malloc(ss->shape1_column * sizeof(InDType));
        Map_InDType rd_col_sum_inner(rd_col_buf, 1, ss->shape1_column, DynStride(1, 1));

        if (ss->shape1_column <= 64 && ss->stride_rs1 == ss->shape1_column) {
            MATRIX_ACC_DIMH_4PART(mul_result, rd_col_sum_inner, InDType, ss->shape1_row, ss->shape1_column);
        } else {
            MATRIX_ACC_DIMH_PARITY(mul_result, rd_col_sum_inner, InDType, ss->shape1_row, ss->shape1_column);
        }

        if (GLOBAL_DBG)
            cout << "rdinner:\n" << rd_col_sum_inner << endl;

        MATRIX_CAST(rd_col_sum_inner, rd_col_sum, OutDType, 1, ss->shape1_column);
        free(rd_col_buf);
        if (GLOBAL_DBG)
            cout << "rd:\n" << rd_col_sum << endl;
    } else {
        Map_OutDType rd_row_sum(rd, ss->shape1_row, 1, DynStride(ss->stride_rd, 1));
        InDType *rd_row_buf = (InDType *)malloc(ss->shape1_row * sizeof(InDType));
        Map_InDType rd_row_sum_inner(rd_row_buf, ss->shape1_row, 1, DynStride(1, 1));

        MATRIX_ACC_DIMW_PAIR(mul_result, rd_row_sum_inner, InDType, ss->shape1_row, ss->shape1_column);

        MATRIX_CAST(rd_row_sum_inner, rd_row_sum, OutDType, ss->shape1_row, 1);
        free(rd_row_buf);
        if (GLOBAL_DBG)
            cout << "rd:\n" << rd_row_sum << endl;
    }

    free(mul_buf);

    return 0;
}

template <typename OutDType, typename InDType>
int veemacc_mm(OutDType *rs1, OutDType *rd, OutDType *rs2, struct ShapeStride *ss)
{
    DEFINE_MAP_DTYPE(OutDType)
    DEFINE_MAP_DTYPE(InDType)

    Map_OutDType rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    Map_OutDType rs2_matrix(rs2, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs2, 1));

    if (GLOBAL_DBG) {
        SHAPE_STRIDE_INFO(ss);
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rs2:" << endl << rs2_matrix << endl;
    }

    InDType *mul_buf = (InDType *)malloc(ss->shape1_row * ss->shape1_column * sizeof(InDType));
    Map_InDType mul_result(mul_buf, ss->shape1_row, ss->shape1_column, DynStride(ss->shape1_column, 1));
    MATRIX_MUL_CONVERT(rs1_matrix, rs2_matrix, mul_result, ss->shape1_row, ss->shape1_column, InDType);

    InDType *pcol_sum = (InDType *)malloc(ss->shape1_column * sizeof(InDType));
    Map_InDType rd_col_sum(pcol_sum, 1, ss->shape1_column, DynStride(1, 1));

    if (ss->shape1_column <= 64 && ss->stride_rs1 == ss->shape1_column) {
        MATRIX_ACC_DIMH_4PART(mul_result, rd_col_sum, InDType, ss->shape1_row, ss->shape1_column);
    } else {
        MATRIX_ACC_DIMH_PARITY(mul_result, rd_col_sum, InDType, ss->shape1_row, ss->shape1_column);
    }

    Matrix_InDType rd_acc(1, 1);
    MATRIX_ACC_DIMW_PAIR(rd_col_sum, rd_acc, InDType, 1, ss->shape1_column);
    *rd = OutDType(rd_acc(0, 0));

    if (GLOBAL_DBG)
        cout << "rd:\n" << *rd << endl;

    free(pcol_sum);
    free(mul_buf);

    return 0;
}

template <typename OutDType, typename InDType>
int veemacc_mv(OutDType *rs1, OutDType *rd, OutDType *rs2, struct ShapeStride *ss, int dim)
{
    DEFINE_MAP_DTYPE(OutDType)
    DEFINE_MAP_DTYPE(InDType)

    Map_OutDType rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, 1);
    if (GLOBAL_DBG) {
        SHAPE_STRIDE_INFO(ss);
        cout << "rs1:" << endl << rs1_matrix << endl;
    }

    InDType *mul_buf = (InDType *)malloc(ss->shape1_row * ss->shape1_column * sizeof(InDType));
    Map_InDType mul_result(mul_buf, ss->shape1_row, ss->shape1_column, DynStride(ss->shape1_column, 1));

    if (0 == dim) {
        Map_OutDType vec_rs2_dim0(rs2, ss->shape1_row, 1, DynStride(1, 1));
        if (GLOBAL_DBG) {
            cout << "rs2:" << endl << vec_rs2_dim0 << endl;
        }

        MATRIX_MUL_VEC_V_CONVERT(rs1_matrix, vec_rs2_dim0, mul_result, ss->shape1_row, ss->shape1_column, InDType);

        InDType *rd_col_buf = (InDType *)malloc(ss->shape1_column * sizeof(InDType));
        Map_InDType rd_col_sum_inner(rd_col_buf, 1, ss->shape1_column, DynStride(1, 1));
        if (ss->shape1_column <= 64 && ss->stride_rs1 == ss->shape1_column) {
            MATRIX_ACC_DIMH_4PART(mul_result, rd_col_sum_inner, InDType, ss->shape1_row, ss->shape1_column);
        } else {
            MATRIX_ACC_DIMH_PARITY(mul_result, rd_col_sum_inner, InDType, ss->shape1_row, ss->shape1_column);
        }


        Map_OutDType vec_rd_dim0(rd, 1, ss->shape1_column, DynStride(1, 1));
        MATRIX_CAST(rd_col_sum_inner, vec_rd_dim0, OutDType, 1, ss->shape1_column);
        free(rd_col_buf);
        if (GLOBAL_DBG)
            cout << "rd:\n" << vec_rd_dim0 << endl;
    } else {
        Map_OutDType vec_rs2_dim1(rs2, 1, ss->shape1_column, DynStride(1, 1));
        if (GLOBAL_DBG) {
            cout << "rs2:" << endl << vec_rs2_dim1 << endl;
        }

        MATRIX_MUL_VEC_H_CONVERT(rs1_matrix, vec_rs2_dim1, mul_result, ss->shape1_row, ss->shape1_column, InDType);

        InDType *rd_row_buf = (InDType *)malloc(ss->shape1_row * sizeof(InDType));
        Map_InDType rd_row_sum_inner(rd_row_buf, ss->shape1_row, 1, DynStride(1, 1));
        MATRIX_ACC_DIMW_PAIR(mul_result, rd_row_sum_inner, InDType, ss->shape1_row, ss->shape1_column);

        Map_OutDType rd_row_sum(rd, ss->shape1_row, 1, DynStride(ss->stride_rd, 1));
        MATRIX_CAST(rd_row_sum_inner, rd_row_sum, OutDType, ss->shape1_row, 1);
        free(rd_row_buf);
        if (GLOBAL_DBG)
            cout << "rd:\n" << rd_row_sum << endl;
    }
    free(mul_buf);

    return 0;
}

template <typename OutDType, typename InDType>
int veemacc_mf(OutDType *rs1, OutDType *rd, OutDType rs2, struct ShapeStride *ss, int dim)
{
    DEFINE_MAP_DTYPE(OutDType)
    DEFINE_MAP_DTYPE(InDType)

    Map_OutDType rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, 1);
    if (GLOBAL_DBG) {
        SHAPE_STRIDE_INFO(ss);
        cout << "dim: " << dim << endl;
        cout << "rs1:\n" << rs1_matrix << endl;
        cout << "rs2:\n" << rs2 << endl;
    }

    InDType *mul_buf = (InDType *)malloc(ss->shape1_row * ss->shape1_column * sizeof(InDType));
    Map_InDType mul_result(mul_buf, ss->shape1_row, ss->shape1_column, DynStride(ss->shape1_column, 1));

    MATRIX_MUL_SCALA_CONVERT(rs1_matrix, rs2, mul_result, ss->shape1_row, ss->shape1_column, InDType);

    if (dim == 0) {
        Map_OutDType rd_col_sum(rd, 1, ss->shape1_column, DynStride(1, 1));
        InDType *rd_col_buf = (InDType *)malloc(ss->shape1_column * sizeof(InDType));
        Map_InDType rd_col_sum_inner(rd_col_buf, 1, ss->shape1_column, DynStride(1, 1));

        if (ss->shape1_column <= 64 && ss->stride_rs1 == ss->shape1_column) {
            MATRIX_ACC_DIMH_4PART(mul_result, rd_col_sum_inner, InDType, ss->shape1_row, ss->shape1_column);
        } else {
            MATRIX_ACC_DIMH_PARITY(mul_result, rd_col_sum_inner, InDType, ss->shape1_row, ss->shape1_column);
        }

        if (GLOBAL_DBG)
            cout << "rdinner:\n" << rd_col_sum_inner << endl;

        MATRIX_CAST(rd_col_sum_inner, rd_col_sum, OutDType, 1, ss->shape1_column);
        free(rd_col_buf);
        if (GLOBAL_DBG)
            cout << "rd:\n" << rd_col_sum << endl;
    } else {
        Map_OutDType rd_row_sum(rd, ss->shape1_row, 1, DynStride(ss->stride_rd, 1));
        InDType *rd_row_buf = (InDType *)malloc(ss->shape1_row * sizeof(InDType));
        Map_InDType rd_row_sum_inner(rd_row_buf, ss->shape1_row, 1, DynStride(1, 1));

        MATRIX_ACC_DIMW_PAIR(mul_result, rd_row_sum_inner, InDType, ss->shape1_row, ss->shape1_column);

        MATRIX_CAST(rd_row_sum_inner, rd_row_sum, OutDType, ss->shape1_row, 1);
        free(rd_row_buf);
        if (GLOBAL_DBG)
            cout << "rd:\n" << rd_row_sum << endl;
    }

    free(mul_buf);
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
template <typename DType>
int mov_m(DType *rs1, DType *rd, struct ShapeStride *ss)
{
    DEFINE_MAP_DTYPE(DType)

    Map_DType rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_DType rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    if (GLOBAL_DBG) {
        SHAPE_STRIDE_INFO(ss);
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rd:" << endl << rd_matrix << endl;
    }
    
    rd_matrix = rs1_matrix;

    if (GLOBAL_DBG)
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
template <typename DType>
int mov_v(DType *rs1, DType *rd, struct ShapeStride *ss, int dim)
{
    DEFINE_MAP_DTYPE(DType)

    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    //mov.v 使用shape1的行数和列数，输入vector使用行数或者列数，列数或者行数为1，输出使用shape1的行数和列数
    Map_DType rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));
    Map_DType rs1_matrix(rs1, dim ? ss->shape1_row : 1, dim ? 1 : ss->shape1_column, DynStride(1, 1));

    if (GLOBAL_DBG) {
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

    if (GLOBAL_DBG)
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
template <typename DType>
int mov_f(DType rs1, DType *rd, struct ShapeStride *ss)
{
    DEFINE_MAP_DTYPE(DType)

    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_DType rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    if (GLOBAL_DBG) {
        SHAPE_STRIDE_INFO(ss);
        cout << "rs1:" << endl << rs1 << endl;
    }

    rd_matrix = rd_matrix.Constant(ss->shape1_row, ss->shape1_column, rs1);

    if (GLOBAL_DBG)
        cout << "rd:" << endl << rd_matrix << endl;

    return 0;
}

extern int veavgpool_m(half *rs1, half *rd, struct VmeShapeStride *vss);
extern int vemaxpool_m(half *rs1, half *rd, struct VmeShapeStride *vss);
extern int vedwconv_mm(half *rs1, half *rs2, half *rd, struct VmeShapeStride *vss);

/**
 * @brief custom扩展指令类
 *
 * 包含了全部的custom矩阵扩展指令
 * 可以通过设置其实例的debug字段值来动态控制debug输出
 */
class CustomInsns
{
private:
    void meconv_dbg(struct ConvShapeStride *ss);
    int meconv_x8_mm_base(int8_t *rs1, void *rd, int8_t *rs2, struct ConvShapeStride *ss, int outfp16);
    float16_t half_to_float16_t(half x);
    float32_t half_mul_f32(half a, half b);
    float32_t half_to_f32(half x);
    half float16_t_to_half(float16_t f16);
    half f32_to_half(float32_t f32);
    half int32_mul_f16(int a, float16_t b);
public:
    int debug;

    CustomInsns();

    int vecvt_hf_x8_m(int8_t *rs1, half *rd, struct ShapeStride *ss);
    int vecvt_hf_xu8_m(uint8_t *rs1, half *rd, struct ShapeStride *ss);
    int vecvt_hf_x16_m(int16_t *rs1, half *rd, struct ShapeStride *ss);
    int vecvt_hf_xu16_m(uint16_t *rs1, half *rd, struct ShapeStride *ss);

    int memul_mm(half *rs1, half *rs2, half *rd, struct ShapeStride *ss);
    int memul_mm(half *rs1, int8_t *rs2, half *rd, struct ShapeStride *ss);
    int memul_sp_mm(half *rs1, half *rs2, uint8_t *sparseidx, half *rd, struct ShapeStride *ss);
    int memul_ts_mm(half *rs1, half *rs2, half *rd, struct ShapeStride *ss, int ts);
    int memin_m(half *rs1, half *rd, struct ShapeStride *ss);
    int memax_m(half *rs1, half *rd, struct ShapeStride *ss);
    int meacc_m(half *rs1, half *rd, struct ShapeStride *ss);

    int vemul_mm(half *rs1, half *rs2, half *rd, struct ShapeStride *ss);
    int vemul_mv(half *rs1, half *rs2, half *rd, struct ShapeStride *ss);

    int veemul_mm(half *rs1, half *rd, half *rs2, struct ShapeStride *ss);
    int veemul_mv(half *rs1, half *rd, half *rs2, struct ShapeStride *ss, int dim);
    int veemul_mf(half *rs1, half *rd, half rs2, struct ShapeStride *ss);
    int veemul_x32_mf(int32_t *rs1, half *rd, half rs2, struct ShapeStride *ss);
    int veemul_x32_mv(int32_t *rs1, half *rd, half *rs2, struct ShapeStride *ss);
    int veemul_x8_hf_mf(half *rs1, int8_t *rd, half rs2, struct ShapeStride *ss);

    int vemax_mm(half *rs1, half *rd, half *rs2, struct ShapeStride *ss);
    int vemax_m(half *rs1, half *rd, struct ShapeStride *ss, int dim);
    int vemax_m(half *rs1, half *rd, struct ShapeStride *ss);
    int vemax_mf(half *rs1, half *rd, half rs2, struct ShapeStride *ss);
    int vemax_mv(half *rs1, half *rd, half *rs2, struct ShapeStride *ss, int dim);

    int vemin_mm(half *rs1, half *rd, half *rs2, struct ShapeStride *ss);
    int vemin_m(half *rs1, half *rd, struct ShapeStride *ss, int dim);
    int vemin_m(half *rs1, half *rd, struct ShapeStride *ss);
    int vemin_mf(half *rs1, half *rd, half rs2, struct ShapeStride *ss);
    int vemin_mv(half *rs1, half *rd, half *rs2, struct ShapeStride *ss, int dim);

    int velkrelu_mf(half *rs1, half rs2, half *rd, struct ShapeStride *ss);
    int velkrelu_mv(half *rs1, half *rd, half *rs2, struct ShapeStride *ss, int dim);

    int velut_m(uint16_t *rs1, unsigned long rs2, half *rd, struct ShapeStride *ss);

    int metr_m(half *rs1, half *rd, struct ShapeStride *ss);
    int vecvt_x8_hf_m(half *rs1, int8_t *rd, struct ShapeStride *ss);
    
    int verecip_m(half *rs1, half *rd, struct ShapeStride *ss);
    int vesqrt_m(half *rs1, half *rd, struct ShapeStride *ss);
    int veexp_m(half *rs1, half *rd, struct ShapeStride *ss);

    int meconv_mm(half *rs1, half *rd, half *rs2, struct ConvShapeStride *ss);
    int meconv_mm(half *rs1, half *rd, int8_t *rs2, struct ConvShapeStride *ss);
    int meconv_sp_mm(half *rs1, half *rs2, uint8_t *sparseidx, half *rd, struct ConvShapeStride *ss);
};

/**
 * @brief 浮点/整数类型转换指令
 *
 * 九章处理器只支持 int16/uint16 到 fp16 的转换指令,即支持 vfcvt.f.xu.v 和 vfcvt.f.x.v。
 * 此外,该执行转换指令时 SEW 必须为 16b,否则将触发非法指令异常。
 */
template <typename MaskType>
class Vfcvt
{
  public:
    int debug;

    Vfcvt(): debug(GLOBAL_DBG)
    {

    }

    typedef Map<Matrix<uint16_t, 1, Dynamic>> VfcvtU16VecMap;
    typedef Map<Matrix<int16_t, 1, Dynamic>> VfcvtI16VecMap;
    typedef Map<Matrix<half, 1, Dynamic>> VfcvtHalfVecMap;
    typedef Map<Matrix<MaskType, 1, Dynamic>> VfcvtMaskVecMap;

    /**
     * vfcvt_f_x_v() vfcvt.f.x.v
     *
     * convert signed integer to fp16 (int16 -> fp16)
     * @param vs2 源操作向量基地址
     * @param vd 目的向量基地址
     * @param vm 不可屏蔽标识， vm=0 可屏蔽， vm=1不可屏蔽
     * @param v0 mask向量基地址
     * @param vl 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vfcvt_f_x_v(int16_t *vs2, half *vd, int vm, MaskType *v0, int vl)
    {
        VfcvtI16VecMap vector_vs2(vs2, vl);
        VfcvtHalfVecMap vector_vd(vd, vl);
        VfcvtMaskVecMap vector_v0(v0, vl);

        if (!vm) {
            for (int i = 0; i < vl; i++) {
                if (vector_v0(i) & 0x1)
                    vector_vd(i) = (half)vector_vs2(i);
            }
        } else
            vector_vd = vector_vs2.cast<half>();

        if (debug) {
            cout << __FUNCTION__ << endl;
            cout << "vs2:\n" << vector_vs2 << endl;
            cout << "vm:\n" << vm << endl;
            cout << "v0:\n" << vector_v0 << endl;
            cout << "vd:\n" << vector_vd << endl;
        }

        return 0;
    }

    /**
     * vfcvt_f_xu_v() vfcvt.f.xu.v
     *
     * convert uinsigned integer to fp16 (uint16 -> fp16)
     * @param vs2 源操作向量基地址
     * @param vd 目的向量基地址
     * @param vm 不可屏蔽标识， vm=0 可屏蔽， vm=1不可屏蔽
     * @param v0 mask向量基地址
     * @param vl 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vfcvt_f_xu_v(uint16_t *vs2, half *vd, int vm, MaskType *v0, int vl)
    {
        VfcvtU16VecMap vector_vs2(vs2, vl);
        VfcvtHalfVecMap vector_vd(vd, vl);
        VfcvtMaskVecMap vector_v0(v0, vl);

        if (!vm) {
            for (int i = 0; i < vl; i++) {
                if (vector_v0(i) & 0x1)
                    vector_vd(i) = (half)vector_vs2(i);
            }
        } else
            vector_vd = vector_vs2.cast<half>();

        if (debug) {
            cout << __FUNCTION__ << endl;
            cout << "vs2:\n" << vector_vs2 << endl;
            cout << "vm:\n" << vm << endl;
            cout << "v0:\n" << vector_v0 << endl;
            cout << "vd:\n" << vector_vd << endl;
        }

        return 0;
    }
};

/**
 * @brief 单宽度向量加法指令
 *
 * 目的元素的宽度和源操作数中的元素宽度保持一致， 可以通过Type指定数据类型
 *
 */
template <typename Type, typename MaskType>
class Vadd
{
  public:
    int debug;
    typedef Map<Matrix<Type, 1, Dynamic>> VaddVecMap;
    typedef Map<Matrix<MaskType, 1, Dynamic>> VaddMaskVecMap;

    Vadd(): debug(GLOBAL_DBG)
    {

    }

    /**
     * vadd_vf() vfadd.vf
     * @param vs2 源操作向量基地址
     * @param rs1 源标量操作数
     * @param vd 目的向量基地址
     * @param vm 不可屏蔽标识， vm=0 可屏蔽， vm=1不可屏蔽
     * @param v0 mask向量基地址
     * @param vl 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vadd_vf(Type *vs2, Type rs1, Type *vd, int vm, MaskType *v0, int vl)
    {
        VaddVecMap vector_vs2(vs2, vl);
        VaddVecMap vector_vd(vd, vl);
        VaddMaskVecMap vector_v0(v0, vl);

        if (!vm) {
            for (int i = 0; i < vl; i++) {
                if (vector_v0(i) & 0x1)
                    vector_vd(i) = vector_vs2(i) + rs1;
            }
        } else
            vector_vd = vector_vs2.array() + rs1;

        DBG_VECTOR_VF;

        return 0;
    }

    /**
     * vadd_vv() vfadd.vv
     * @param vs2 源操作向量二基地址
     * @param vs1 源操作向量一基地址
     * @param vd 目的向量基地址
     * @param vm 不可屏蔽标识， vm=0 可屏蔽， vm=1不可屏蔽
     * @param v0 mask向量基地址
     * @param vl 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vadd_vv(Type *vs2, Type *vs1, Type *vd, int vm, MaskType *v0, int vl)
    {
        VaddVecMap vector_vs2(vs2, vl);
        VaddVecMap vector_vs1(vs1, vl);
        VaddVecMap vector_vd(vd, vl);
        VaddMaskVecMap vector_v0(v0, vl);

        if (!vm) {
            for (int i = 0; i < vl; i++) {
                if (vector_v0(i) & 0x1)
                    vector_vd(i) = vector_vs2(i) + vector_vs1(i);
            }
        } else
            vector_vd = vector_vs2.array() + vector_vs1.array();

        DBG_VECTOR_VV;

        return 0;
    }
};

/**
 * @brief 单宽度向量减法指令
 *
 * 因为芯片本身不支持浮点减法， 所以利用 a + (-b) 来实现 a - b 的操作
 * 目的元素的宽度和源操作数中的元素宽度保持一致， 可以通过Type指定数据类型
 *
 */
template <typename Type, typename MaskType>
class Vsub
{
  public:
    int debug;

    Vsub(): debug(GLOBAL_DBG)
    {
    }

    typedef Map<Matrix<Type, 1, Dynamic>> VsubVecMap;
    typedef Map<Matrix<MaskType, 1, Dynamic>> VsubMaskVecMap;

    /**
     * vsub_vf() vfsub.vf
     * @param vs2 源操作向量基地址
     * @param rs1 源标量操作数
     * @param vd 目的向量基地址
     * @param vm 不可屏蔽标识， vm=0 可屏蔽， vm=1不可屏蔽
     * @param v0 mask向量基地址
     * @param vl 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vsub_vf(Type *vs2, Type rs1, Type *vd, int vm, MaskType *v0, int vl)
    {
        VsubVecMap vector_vs2(vs2, vl);
        VsubVecMap vector_vd(vd, vl);
        VsubMaskVecMap vector_v0(v0, vl);

        if (!vm) {
            for (int i = 0; i < vl; i++) {
                if (vector_v0(i) & 0x1)
                    vector_vd(i) = vector_vs2(i) + -rs1;
            }
        } else
            vector_vd = vector_vs2.array() + -rs1;

        DBG_VECTOR_VF;

        return 0;
    }

    /**
     * vrsub_vf() vfrsub.vf
     * @param vs2 源操作向量基地址
     * @param rs1 源标量操作数
     * @param vd 目的向量基地址
     * @param vm 不可屏蔽标识， vm=0 可屏蔽， vm=1不可屏蔽
     * @param v0 mask向量基地址
     * @param vl 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vrsub_vf(Type *vs2, Type rs1, Type *vd, int vm, MaskType *v0, int vl)
    {
        VsubVecMap vector_vs2(vs2, vl);
        VsubVecMap vector_vd(vd, vl);
        VsubMaskVecMap vector_v0(v0, vl);

        if (!vm) {
            for (int i = 0; i < vl; i++) {
                if (vector_v0(i) & 0x1)
                    vector_vd(i) = rs1 + -vector_vs2(i);
            }
        } else
            vector_vd = rs1 + -vector_vs2.array();

        DBG_VECTOR_VF;

        return 0;
    }

    /**
     * vsub_vv() vfsub.vv
     * @param vs2 源操作向量二基地址
     * @param vs1 源操作向量一基地址
     * @param vd 目的向量基地址
     * @param vm 不可屏蔽标识， vm=0 可屏蔽， vm=1不可屏蔽
     * @param v0 mask向量基地址
     * @param vl 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vsub_vv(Type *vs2, Type *vs1, Type *vd, int vm, MaskType *v0, int vl)
    {
        VsubVecMap vector_vs2(vs2, vl);
        VsubVecMap vector_vs1(vs1, vl);
        VsubVecMap vector_vd(vd, vl);
        VsubMaskVecMap vector_v0(v0, vl);

        if (!vm) {
            for (int i = 0; i < vl; i++) {
                if (vector_v0(i) & 0x1)
                    vector_vd(i) = vector_vs2(i) + -vector_vs1(i);
            }
        } else
            vector_vd = vector_vs2.array() + -vector_vs1.array();

        DBG_VECTOR_VV;

        return 0;
    }
};


/**
 * @brief 单宽度向量乘法指令
 *
 * 目的元素的宽度和源操作数中的元素宽度保持一致， 可以通过Type指定数据类型
 */
template <typename Type, typename MaskType>
class Vmul
{
  public:
    int debug;

    Vmul(): debug(GLOBAL_DBG)
    {

    }

    typedef Map<Matrix<Type, 1, Dynamic>> VmulVecMap;
    typedef Map<Matrix<MaskType, 1, Dynamic>> VmulMaskVecMap;

    /**
     * vmul_vf() vfmul.vf
     * @param vs2 源操作向量基地址
     * @param rs1 源标量操作数
     * @param vd 目的向量基地址
     * @param vm 不可屏蔽标识， vm=0 可屏蔽， vm=1不可屏蔽
     * @param v0 mask向量基地址
     * @param vl 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vmul_vf(Type *vs2, Type rs1, Type *vd, int vm, MaskType *v0, int vl)
    {
        VmulVecMap vector_vs2(vs2, vl);
        VmulVecMap vector_vd(vd, vl);
        VmulMaskVecMap vector_v0(v0, vl);

        if (!vm) {
            for (int i = 0; i < vl; i++) {
                if (vector_v0(i) & 0x1)
                    vector_vd(i) = vector_vs2(i) * rs1;
            }
        } else
            vector_vd = vector_vs2.array() * rs1;

        DBG_VECTOR_VF;

        return 0;
    }

    /**
     * vmul_vv() vfmul.vv
     * @param vs2 源操作向量二基地址
     * @param vs1 源操作向量一基地址
     * @param vd 目的向量基地址
     * @param vm 不可屏蔽标识， vm=0 可屏蔽， vm=1不可屏蔽
     * @param v0 mask向量基地址
     * @param vl 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vmul_vv(Type *vs2, Type *vs1, Type *vd, int vm, MaskType *v0, int vl)
    {
        VmulVecMap vector_vs2(vs2, vl);
        VmulVecMap vector_vs1(vs1, vl);
        VmulVecMap vector_vd(vd, vl);
        VmulMaskVecMap vector_v0(v0, vl);

        if (!vm) {
            for (int i = 0; i < vl; i++) {
                if (vector_v0(i) & 0x1)
                    vector_vd(i) = vector_vs2(i) * vector_vs1(i);
            }
        } else
            vector_vd = vector_vs2.array() * vector_vs1.array();

        DBG_VECTOR_VV;

        return 0;
    }
};

/**
 * @brief 向量浮点合并指令类
 *
 * 虽然目前设计文档中仅有一个操作，但本接口实际支持任意数据类型的merge
 * 当然，从接口格式上限制了输入向量，输出向量，输入标量这三者的数据类型必须一致
 * mask向量数据类型可以独立指定
 *
 * Type 输入向量，输出向量，输入标量的数据类型
 * MaskType mask向量的数据类型
 */
template <typename Type, typename Typef, typename MaskType>
class Vmerge
{
  public:
    int debug;

    Vmerge(): debug(GLOBAL_DBG)
    {

    }

    typedef Map<Matrix<Type, 1, Dynamic>> VmergeDataVecMap;
    typedef Map<Matrix<MaskType, 1, Dynamic>> VmergeMaskVecMap;

    /**
     * vmerge_vf() vfmerge.vf
     * @param vs2 源操作向量基地址
     * @param rs1 源标量操作数
     * @param vd 目的向量基地址
     * @param vm 不可屏蔽标识， vm=0 可屏蔽， vm=1不可屏蔽
     * @param v0 mask向量基地址
     * @param vl 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vmerge_vf(Type *vs2, Typef rs1, Type *vd, int vm, MaskType *v0, int vl)
    {
        VmergeDataVecMap vector_vs2(vs2, vl);
        VmergeDataVecMap vector_vd(vd, vl);
        VmergeMaskVecMap vector_v0(v0, vl);

        /* vm = 1, vd[0...n] = rs1 */
        for (int i = 0; i < vl; i++) {
            if (vm || vector_v0(i) & 0x1)
                vector_vd(i) = rs1;
            else
                vector_vd(i) = vector_vs2(i);
        }

        DBG_VECTOR_VF;

        return 0;
    }

    /**
     * vmerge_vvm() vfmerge.vvm
     * @param vs2 源操作向量基地址
     * @param rs1 源标量操作数
     * @param vd 目的向量基地址
     * @param vm 不可屏蔽标识， vm=0 可屏蔽， vm=1不可屏蔽
     * @param v0 mask向量基地址
     * @param vl 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vmerge_vvm(Type *vs2, Type *vs1, Type *vd, int vm, MaskType *v0, int vl)
    {
        VmergeDataVecMap vector_vs2(vs2, vl);
        VmergeDataVecMap vector_vs1(vs1, vl);
        VmergeDataVecMap vector_vd(vd, vl);
        VmergeMaskVecMap vector_v0(v0, vl);

        /* vm = 1, vd[0...n] = vs1[i] */
        for (int i = 0; i < vl; i++) {
            if (vm || vector_v0(i) & 0x1)
                vector_vd(i) = vector_vs1(i);
            else
                vector_vd(i) = vector_vs2(i);
        }

        DBG_VECTOR_VVM;

        return 0;
    }
};

/**
 * @brief 整数提取指令
 *
 * 选取源向量寄存器中的一个元素
 *
 */
template <typename Type>
class Vext
{
  public:
    int debug;

    Vext(): debug(GLOBAL_DBG)
    {

    }

    typedef Map<Matrix<Type, 1, Dynamic>> VextVecMap;

    /**
     * vext_x_v() vext.x.v   rd = vs2[rs1]
     *
     * 如果索引超出范围则rd会被置为0，不会认为指令错误
     *
     * @param vs2 源操作向量基地址
     * @param rs1 元素索引
     * @param vd 目的数存放地址
     * @param vl 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vext_x_v(Type *vs2, Type *rd, uint16_t rs1, int vl)
    {
        VextVecMap vector_vs2(vs2, vl);

        if (rs1 >= vl)
            *rd = (Type)0;
        else
            *rd = vector_vs2(rs1);

        if (debug) {
            cout << __FUNCTION__ << endl;
            cout << "vs2:\n" << vector_vs2 << endl;
            cout << "rs1:\n" << rs1 << endl;
        }

        return 0;
    }
};

/**
 * @brief 单宽度向量乘加(FMA)指令
 *
 * 包含乘累加(macc), 乘累减(msac), 乘加(madd)， 乘减(msub)
 * 支持任意数据类型
 *
 */
template <typename Type, typename MaskType>
class Vma
{
  public:
    int debug;

    Vma(): debug(GLOBAL_DBG)
    {

    }

    typedef Map<Matrix<Type, 1, Dynamic>> VmaVecMap;
    typedef Map<Matrix<MaskType, 1, Dynamic>> VmaMaskVecMap;

    /**
     * vmacc_vf() vfmacc.vf
     * @param vs2 源操作向量基地址
     * @param rs1 源标量操作数
     * @param vd 目的向量基地址
     * @param vm 不可屏蔽标识， vm=0 可屏蔽， vm=1不可屏蔽
     * @param v0 mask向量基地址
     * @param vl 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vmacc_vf(Type *vs2, Type rs1, Type *vd, int vm, MaskType *v0, int vl)
    {
        VmaVecMap vector_vs2(vs2, vl);
        VmaVecMap vector_vd(vd, vl);
        VmaMaskVecMap vector_v0(v0, vl);

        if (!vm) {
            for (int i = 0; i < vl; i++) {
                if (vector_v0(i) & 0x1)
                    vector_vd(i) = vector_vs2(i) * rs1 + vector_vd(i);
            }
        } else
            vector_vd = vector_vs2.array() * rs1 + vector_vd.array();

        DBG_VECTOR_VF;

        return 0;
    }

    /**
     * vmacc_vv() vfmacc.vv
     * @param vs2 源操作向量二基地址
     * @param vs1 源操作向量一基地址
     * @param vd 目的向量基地址
     * @param vm 不可屏蔽标识， vm=0 可屏蔽， vm=1不可屏蔽
     * @param v0 mask向量基地址
     * @param vl 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vmacc_vv(Type *vs2, Type *vs1, Type *vd, int vm, MaskType *v0, int vl)
    {
        VmaVecMap vector_vs2(vs2, vl);
        VmaVecMap vector_vs1(vs1, vl);
        VmaVecMap vector_vd(vd, vl);
        VmaMaskVecMap vector_v0(v0, vl);

        if (!vm) {
            for (int i = 0; i < vl; i++) {
                if (vector_v0(i) & 0x1)
                    vector_vd(i) = vector_vs2(i) * vector_vs1(i) + vector_vd(i);
            }
        } else
            vector_vd = vector_vs2.array() * vector_vs1.array() + vector_vd.array();

        DBG_VECTOR_VV;

        return 0;
    }

    /**
     * vnmacc_vf() vfnmacc.vf
     *
     * vd[i] = -(f[rs1] * vs2[i]) - vd[i]
     * @param vs2 源操作向量基地址
     * @param rs1 源标量操作数
     * @param vd 目的向量基地址
     * @param vm 不可屏蔽标识， vm=0 可屏蔽， vm=1不可屏蔽
     * @param v0 mask向量基地址
     * @param vl 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vnmacc_vf(Type *vs2, Type rs1, Type *vd, int vm, MaskType *v0, int vl)
    {
        VmaVecMap vector_vs2(vs2, vl);
        VmaVecMap vector_vd(vd, vl);
        VmaMaskVecMap vector_v0(v0, vl);

        if (!vm) {
            for (int i = 0; i < vl; i++) {
                if (vector_v0(i) & 0x1)
                    vector_vd(i) = -(vector_vs2(i) * rs1) + -vector_vd(i);
            }
        } else
            vector_vd = -(vector_vs2.array() * rs1) + -vector_vd.array();

        DBG_VECTOR_VF;

        return 0;
    }

    /**
     * vnmacc_vv() vfnmacc.vv
     *
     * vd[i] = -(vs1[i] * vs2[i]) - vd[i]
     * @param vs2 源操作向量二基地址
     * @param vs1 源操作向量一基地址
     * @param vd 目的向量基地址
     * @param vm 不可屏蔽标识， vm=0 可屏蔽， vm=1不可屏蔽
     * @param v0 mask向量基地址
     * @param vl 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vnmacc_vv(Type *vs2, Type *vs1, Type *vd, int vm, MaskType *v0, int vl)
    {
        VmaVecMap vector_vs2(vs2, vl);
        VmaVecMap vector_vs1(vs1, vl);
        VmaVecMap vector_vd(vd, vl);
        VmaMaskVecMap vector_v0(v0, vl);

        if (!vm) {
            for (int i = 0; i < vl; i++) {
                if (vector_v0(i) & 0x1)
                    vector_vd(i) = -(vector_vs2(i) * vector_vs1(i)) + -vector_vd(i);
            }
        } else
            vector_vd = -(vector_vs2.array() * vector_vs1.array()) + -vector_vd.array();

        DBG_VECTOR_VV;

        return 0;
    }

    /**
     * vmsac_vf() vfmsac.vf
     *
     * vd[i] = +(f[rs1] * vs2[i]) - vd[i]
     * @param vs2 源操作向量基地址
     * @param rs1 源标量操作数
     * @param vd 目的向量基地址
     * @param vm 不可屏蔽标识， vm=0 可屏蔽， vm=1不可屏蔽
     * @param v0 mask向量基地址
     * @param vl 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vmsac_vf(Type *vs2, Type rs1, Type *vd, int vm, MaskType *v0, int vl)
    {
        VmaVecMap vector_vs2(vs2, vl);
        VmaVecMap vector_vd(vd, vl);
        VmaMaskVecMap vector_v0(v0, vl);

        if (!vm) {
            for (int i = 0; i < vl; i++) {
                if (vector_v0(i) & 0x1)
                    vector_vd(i) = vector_vs2(i) * rs1 + -vector_vd(i);
            }
        } else
            vector_vd = vector_vs2.array() * rs1 + -vector_vd.array();

        DBG_VECTOR_VF;

        return 0;
    }

    /**
     * vmsac_vv() vfmsac.vv
     *
     * vd[i] = +(vs1[i] * vs2[i]) - vd[i]
     * @param vs2 源操作向量二基地址
     * @param vs1 源操作向量一基地址
     * @param vd 目的向量基地址
     * @param vm 不可屏蔽标识， vm=0 可屏蔽， vm=1不可屏蔽
     * @param v0 mask向量基地址
     * @param vl 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vmsac_vv(Type *vs2, Type *vs1, Type *vd, int vm, MaskType *v0, int vl)
    {
        VmaVecMap vector_vs2(vs2, vl);
        VmaVecMap vector_vs1(vs1, vl);
        VmaVecMap vector_vd(vd, vl);
        VmaMaskVecMap vector_v0(v0, vl);

        if (!vm) {
            for (int i = 0; i < vl; i++) {
                if (vector_v0(i) & 0x1)
                    vector_vd(i) = vector_vs2(i) * vector_vs1(i) + -vector_vd(i);
            }
        } else
            vector_vd = vector_vs2.array() * vector_vs1.array() + -vector_vd.array();

        DBG_VECTOR_VV;

        return 0;
    }

    /**
     * vnmsac_vf() vfnmsac.vf
     *
     * vd[i] = -(f[rs1] * vs2[i]) + vd[i]
     * @param vs2 源操作向量基地址
     * @param rs1 源标量操作数
     * @param vd 目的向量基地址
     * @param vm 不可屏蔽标识， vm=0 可屏蔽， vm=1不可屏蔽
     * @param v0 mask向量基地址
     * @param vl 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vnmsac_vf(Type *vs2, Type rs1, Type *vd, int vm, MaskType *v0, int vl)
    {
        VmaVecMap vector_vs2(vs2, vl);
        VmaVecMap vector_vd(vd, vl);
        VmaMaskVecMap vector_v0(v0, vl);

        if (!vm) {
            for (int i = 0; i < vl; i++) {
                if (vector_v0(i) & 0x1)
                    vector_vd(i) = -(vector_vs2(i) * rs1) + vector_vd(i);
            }
        } else
            vector_vd = -(vector_vs2.array() * rs1) + vector_vd.array();

        DBG_VECTOR_VF;

        return 0;
    }

    /**
     * vnmsac_vv() vfnmsac.vv
     *
     * d[i] = -(vs1[i] * vs2[i]) + vd[i]
     * @param vs2 源操作向量二基地址
     * @param vs1 源操作向量一基地址
     * @param vd 目的向量基地址
     * @param vm 不可屏蔽标识， vm=0 可屏蔽， vm=1不可屏蔽
     * @param v0 mask向量基地址
     * @param vl 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vnmsac_vv(Type *vs2, Type *vs1, Type *vd, int vm, MaskType *v0, int vl)
    {
        VmaVecMap vector_vs2(vs2, vl);
        VmaVecMap vector_vs1(vs1, vl);
        VmaVecMap vector_vd(vd, vl);
        VmaMaskVecMap vector_v0(v0, vl);

        if (!vm) {
            for (int i = 0; i < vl; i++) {
                if (vector_v0(i) & 0x1)
                    vector_vd(i) = -(vector_vs2(i) * vector_vs1(i)) + vector_vd(i);
            }
        } else
            vector_vd = -(vector_vs2.array() * vector_vs1.array()) + vector_vd.array();

        DBG_VECTOR_VV;

        return 0;
    }

    /**
     * vmadd_vf() vfmadd.vf
     *
     * vfmadd.vf vd, rs1, vs2, vm # vd[i] = +(vd[i] * f[rs1]) + vs2[i]
     * @param vs2 源操作向量基地址
     * @param rs1 源标量操作数
     * @param vd 目的向量基地址
     * @param vm 不可屏蔽标识， vm=0 可屏蔽， vm=1不可屏蔽
     * @param v0 mask向量基地址
     * @param vl 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vmadd_vf(Type *vs2, Type rs1, Type *vd, int vm, MaskType *v0, int vl)
    {
        VmaVecMap vector_vs2(vs2, vl);
        VmaVecMap vector_vd(vd, vl);
        VmaMaskVecMap vector_v0(v0, vl);

        if (!vm) {
            for (int i = 0; i < vl; i++) {
                if (vector_v0(i) & 0x1)
                    vector_vd(i) = vector_vd(i) * rs1 + vector_vs2(i);
            }
        } else
            vector_vd = vector_vd.array() * rs1 + vector_vs2.array();

        DBG_VECTOR_VF;

        return 0;
    }

    /**
     * vmadd_vv() vfmadd.vv
     *
     * vfmadd.vv vd, vs1, vs2, vm # vd[i] = +(vd[i] * vs1[i]) + vs2[i]
     * @param vs2 源操作向量二基地址
     * @param vs1 源操作向量一基地址
     * @param vd 目的向量基地址
     * @param vm 不可屏蔽标识， vm=0 可屏蔽， vm=1不可屏蔽
     * @param v0 mask向量基地址
     * @param vl 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vmadd_vv(Type *vs2, Type *vs1, Type *vd, int vm, MaskType *v0, int vl)
    {
        VmaVecMap vector_vs2(vs2, vl);
        VmaVecMap vector_vs1(vs1, vl);
        VmaVecMap vector_vd(vd, vl);
        VmaMaskVecMap vector_v0(v0, vl);

        if (!vm) {
            for (int i = 0; i < vl; i++) {
                if (vector_v0(i) & 0x1)
                    vector_vd(i) = vector_vd(i) * vector_vs1(i) + vector_vs2(i);
            }
        } else
            vector_vd = vector_vd.array() * vector_vs1.array() + vector_vs2.array();

        DBG_VECTOR_VV;

        return 0;
    }

    /**
     * vnmadd_vf() vfnmadd.vf
     *
     * vfnmadd.vf vd, rs1, vs2, vm # vd[i] = -(vd[i] * f[rs1]) - vs2[i]
     * @param vs2 源操作向量基地址
     * @param rs1 源标量操作数
     * @param vd 目的向量基地址
     * @param vm 不可屏蔽标识， vm=0 可屏蔽， vm=1不可屏蔽
     * @param v0 mask向量基地址
     * @param vl 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vnmadd_vf(Type *vs2, Type rs1, Type *vd, int vm, MaskType *v0, int vl)
    {
        VmaVecMap vector_vs2(vs2, vl);
        VmaVecMap vector_vd(vd, vl);
        VmaMaskVecMap vector_v0(v0, vl);

        if (!vm) {
            for (int i = 0; i < vl; i++) {
                if (vector_v0(i) & 0x1)
                    vector_vd(i) = -(vector_vd(i) * rs1) + -vector_vs2(i);
            }
        } else
            vector_vd = -(vector_vd.array() * rs1) + -vector_vs2.array();

        DBG_VECTOR_VF;

        return 0;
    }

    /**
     * vnmadd_vv() vfnmadd.vv
     *
     * vfnmadd.vv vd, vs1, vs2, vm # vd[i] = -(vd[i] * vs1[i]) - vs2[i]
     * @param vs2 源操作向量二基地址
     * @param vs1 源操作向量一基地址
     * @param vd 目的向量基地址
     * @param vm 不可屏蔽标识， vm=0 可屏蔽， vm=1不可屏蔽
     * @param v0 mask向量基地址
     * @param vl 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vnmadd_vv(Type *vs2, Type *vs1, Type *vd, int vm, MaskType *v0, int vl)
    {
        VmaVecMap vector_vs2(vs2, vl);
        VmaVecMap vector_vs1(vs1, vl);
        VmaVecMap vector_vd(vd, vl);
        VmaMaskVecMap vector_v0(v0, vl);

        if (!vm) {
            for (int i = 0; i < vl; i++) {
                if (vector_v0(i) & 0x1)
                    vector_vd(i) = -(vector_vd(i) * vector_vs1(i)) + -vector_vs2(i);
            }
        } else
            vector_vd = -(vector_vd.array() * vector_vs1.array()) + -vector_vs2.array();

        DBG_VECTOR_VV;

        return 0;
    }

    /**
     * vmsub_vf() vfmsub.vf
     *
     * vfmsub.vf vd, rs1, vs2, vm # vd[i] = +(vd[i] * f[rs1]) - vs2[i]
     * @param vs2 源操作向量基地址
     * @param rs1 源标量操作数
     * @param vd 目的向量基地址
     * @param vm 不可屏蔽标识， vm=0 可屏蔽， vm=1不可屏蔽
     * @param v0 mask向量基地址
     * @param vl 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vmsub_vf(Type *vs2, Type rs1, Type *vd, int vm, MaskType *v0, int vl)
    {
        VmaVecMap vector_vs2(vs2, vl);
        VmaVecMap vector_vd(vd, vl);
        VmaMaskVecMap vector_v0(v0, vl);

        if (!vm) {
            for (int i = 0; i < vl; i++) {
                if (vector_v0(i) & 0x1)
                    vector_vd(i) = vector_vd(i) * rs1 + -vector_vs2(i);
            }
        } else
            vector_vd = vector_vd.array() * rs1 + -vector_vs2.array();

        DBG_VECTOR_VF;

        return 0;
    }

    /**
     * vmsub_vv() vfmsub.vv
     *
     * vfmsub.vv vd, vs1, vs2, vm # vd[i] = +(vd[i] * vs1[i]) - vs2[i]
     * @param vs2 源操作向量二基地址
     * @param vs1 源操作向量一基地址
     * @param vd 目的向量基地址
     * @param vm 不可屏蔽标识， vm=0 可屏蔽， vm=1不可屏蔽
     * @param v0 mask向量基地址
     * @param vl 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vmsub_vv(Type *vs2, Type *vs1, Type *vd, int vm, MaskType *v0, int vl)
    {
        VmaVecMap vector_vs2(vs2, vl);
        VmaVecMap vector_vs1(vs1, vl);
        VmaVecMap vector_vd(vd, vl);
        VmaMaskVecMap vector_v0(v0, vl);

        if (!vm) {
            for (int i = 0; i < vl; i++) {
                if (vector_v0(i) & 0x1)
                    vector_vd(i) = vector_vd(i) * vector_vs1(i) + -vector_vs2(i);
            }
        } else
            vector_vd = vector_vd.array() * vector_vs1.array() + -vector_vs2.array();

        DBG_VECTOR_VV;

        return 0;
    }

    /**
     * vnmsub_vf() vfnmsub.vf
     *
     * vfnmsub.vf vd, rs1, vs2, vm # vd[i] = -(vd[i] * f[rs1]) + vs2[i]
     * @param vs2 源操作向量基地址
     * @param rs1 源标量操作数
     * @param vd 目的向量基地址
     * @param vm 不可屏蔽标识， vm=0 可屏蔽， vm=1不可屏蔽
     * @param v0 mask向量基地址
     * @param vl 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vnmsub_vf(Type *vs2, Type rs1, Type *vd, int vm, MaskType *v0, int vl)
    {
        VmaVecMap vector_vs2(vs2, vl);
        VmaVecMap vector_vd(vd, vl);
        VmaMaskVecMap vector_v0(v0, vl);

        if (!vm) {
            for (int i = 0; i < vl; i++) {
                if (vector_v0(i) & 0x1)
                    vector_vd(i) = -(vector_vd(i) * rs1) + vector_vs2(i);
            }
        } else
            vector_vd = -(vector_vd.array() * rs1) + vector_vs2.array();

        DBG_VECTOR_VF;

        return 0;
    }

    /**
     * vnmsub_vv() vfnmsub.vv
     *
     * vfnmsub.vv vd, vs1, vs2, vm # vd[i] = -(vd[i] * vs1[i]) + vs2[i]
     * @param vs2 源操作向量二基地址
     * @param vs1 源操作向量一基地址
     * @param vd 目的向量基地址
     * @param vm 不可屏蔽标识， vm=0 可屏蔽， vm=1不可屏蔽
     * @param v0 mask向量基地址
     * @param vl 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vnmsub_vv(Type *vs2, Type *vs1, Type *vd, int vm, MaskType *v0, int vl)
    {
        VmaVecMap vector_vs2(vs2, vl);
        VmaVecMap vector_vs1(vs1, vl);
        VmaVecMap vector_vd(vd, vl);
        VmaMaskVecMap vector_v0(v0, vl);

        if (!vm) {
            for (int i = 0; i < vl; i++) {
                if (vector_v0(i) & 0x1)
                    vector_vd(i) = -(vector_vd(i) * vector_vs1(i)) + vector_vs2(i);
            }
        } else
            vector_vd = -(vector_vd.array() * vector_vs1.array()) + vector_vs2.array();

        DBG_VECTOR_VV;

        return 0;
    }

    /**
     * vmax_vf() vfmax.vf
     * @param vs2 源操作向量基地址
     * @param rs1 源标量操作数
     * @param vd 目的向量基地址
     * @param vm 不可屏蔽标识， vm=0 可屏蔽， vm=1不可屏蔽
     * @param v0 mask向量基地址
     * @param vl 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vmax_vf(Type *vs2, Type rs1, Type *vd, int vm, MaskType *v0, int vl)
    {
        VmaVecMap vector_vs2(vs2, vl);
        VmaVecMap vector_vd(vd, vl);
        VmaMaskVecMap vector_v0(v0, vl);

        if (!vm) {
            for (int i = 0; i < vl; i++) {
                if (vector_v0(i) & 0x1) {
                    vector_vd(i) = max(vector_vs2(i), rs1);
                }
            }
        } else {
            for (int i = 0; i < vl; i++) {
                vector_vd(i) = max(vector_vs2(i), rs1);
            }
        }

        DBG_VECTOR_VF;

        return 0;
    }

    /**
     * vmax_vv() vfmax.vv
     * @param vs2 源操作向量二基地址
     * @param vs1 源操作向量一基地址
     * @param vd 目的向量基地址
     * @param vm 不可屏蔽标识， vm=0 可屏蔽， vm=1不可屏蔽
     * @param v0 mask向量基地址
     * @param vl 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vmax_vv(Type *vs2, Type *vs1, Type *vd, int vm, MaskType *v0, int vl)
    {
        VmaVecMap vector_vs2(vs2, vl);
        VmaVecMap vector_vs1(vs1, vl);
        VmaVecMap vector_vd(vd, vl);
        VmaMaskVecMap vector_v0(v0, vl);

        if (!vm) {
            for (int i = 0; i < vl; i++) {
                if (vector_v0(i) & 0x1) {
                    vector_vd(i) = max(vector_vs2(i), vector_vs1(i));
                }
            }
        } else {
            for (int i = 0; i < vl; i++) {
                vector_vd(i) = max(vector_vs2(i), vector_vs1(i));
            }
        }
        DBG_VECTOR_VV;

        return 0;
    }

    /**
     * vmin_vf() vfmin.vf
     * @param vs2 源操作向量基地址
     * @param rs1 源标量操作数
     * @param vd 目的向量基地址test_vsub
     * @param vm 不可屏蔽标识， vm=0 可屏蔽， vm=1不可屏蔽
     * @param v0 mask向量基地址
     * @param vl 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vmin_vf(Type *vs2, Type rs1, Type *vd, int vm, MaskType *v0, int vl)
    {
        VmaVecMap vector_vs2(vs2, vl);
        VmaVecMap vector_vd(vd, vl);
        VmaMaskVecMap vector_v0(v0, vl);

        if (!vm) {
            for (int i = 0; i < vl; i++) {
                if (vector_v0(i) & 0x1) {
                    vector_vd(i) = min(vector_vs2(i), rs1);
                }
            }
        } else {
            for (int i = 0; i < vl; i++) {
                vector_vd(i) = min(vector_vs2(i), rs1);
            }
        }

        DBG_VECTOR_VF;

        return 0;
    }

    /**
     * vmin_vv() vfmin.vv
     * @param vs2 源操作向量二基地址
     * @param vs1 源操作向量一基地址
     * @param vd 目的向量基地址
     * @param vm 不可屏蔽标识， vm=0 可屏蔽， vm=1不可屏蔽
     * @param v0 mask向量基地址
     * @param vl 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vmin_vv(Type *vs2, Type *vs1, Type *vd, int vm, MaskType *v0, int vl)
    {
        VmaVecMap vector_vs2(vs2, vl);
        VmaVecMap vector_vs1(vs1, vl);
        VmaVecMap vector_vd(vd, vl);
        VmaMaskVecMap vector_v0(v0, vl);

        if (!vm) {
            for (int i = 0; i < vl; i++) {
                if (vector_v0(i) & 0x1) {
                    vector_vd(i) = min(vector_vs2(i), vector_vs1(i));
                }
            }
        } else {
            for (int i = 0; i < vl; i++) {
                vector_vd(i) = min(vector_vs2(i), vector_vs1(i));
            }
        }

        DBG_VECTOR_VV;

        return 0;
    }
};

/**
 * @brief 向量浮点符号注入指令
 *
 * 向量浮点符号注入(Sign-Injection)指令的运算结果的指数和尾数由第一个源操作数 vs2 提供
 */
template <typename Type, typename MaskType>
class Vsgnj
{
  private:
    Type xxabs(Type val)
    {
        if (val < (Type)0)
            return -val;
        return val;
    }

public:
    int debug;

    Vsgnj(): debug(GLOBAL_DBG)
    {

    }

    typedef Map<Matrix<Type, 1, Dynamic>> VsgnjVecMap;
    typedef Map<Matrix<MaskType, 1, Dynamic>> VsgnjMaskVecMap;

    /**
     * vsgnj_vv() vfsgnj.vv
     * @param vs2 源操作向量二基地址
     * @param vs1 源操作向量一基地址
     * @param vd 目的向量基地址
     * @param vm 不可屏蔽标识， vm=0 可屏蔽， vm=1不可屏蔽
     * @param v0 mask向量基地址
     * @param vl 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vsgnj_vv(Type *vs2, Type *vs1, Type *vd, int vm, MaskType *v0, int vl)
    {
        VsgnjVecMap vector_vs2(vs2, vl);
        VsgnjVecMap vector_vs1(vs1, vl);
        VsgnjVecMap vector_vd(vd, vl);
        VsgnjMaskVecMap vector_v0(v0, vl);

        if (!vm) {
            for (int i = 0; i < vl; i++) {
                if (vector_v0(i) & 0x1) {
                    if (*((MaskType *)&vector_vs1(i)) & (1<<(sizeof(Type)*8)-1))
                        vector_vd(i) = -xxabs(vector_vs2(i));
                    else
                        vector_vd(i) = xxabs(vector_vs2(i));
                }
            }
        } else
            for (int i = 0; i < vl; i++) {
                if (*((MaskType *)&vector_vs1(i)) & (1<<(sizeof(Type)*8)-1))
                    vector_vd(i) = -xxabs(vector_vs2(i));
                else
                    vector_vd(i) = xxabs(vector_vs2(i));
            }


        DBG_VECTOR_VV;

        return 0;
    }

    /**
     * vsgnj_vf() vfsgnj.vf
     * @param vs2 源操作向量基地址
     * @param rs1 源标量操作数
     * @param vd 目的向量基地址
     * @param vm 不可屏蔽标识， vm=0 可屏蔽， vm=1不可屏蔽
     * @param v0 mask向量基地址
     * @param vl 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vsgnj_vf(Type *vs2, Type rs1, Type *vd, int vm, MaskType *v0, int vl)
    {
        VsgnjVecMap vector_vs2(vs2, vl);
        VsgnjVecMap vector_vd(vd, vl);
        VsgnjMaskVecMap vector_v0(v0, vl);

        if (!vm) {
            for (int i = 0; i < vl; i++) {
                if (vector_v0(i) & 0x1) {
                    if (*((MaskType *)&rs1) & (1<<(sizeof(Type)*8)-1))
                        vector_vd(i) = -xxabs(vector_vs2(i));
                    else
                        vector_vd(i) = xxabs(vector_vs2(i));
                }
            }
        } else
            for (int i = 0; i < vl; i++) {
                if (*((MaskType *)&rs1) & (1<<(sizeof(Type)*8)-1))
                    vector_vd(i) = -xxabs(vector_vs2(i));
                else
                    vector_vd(i) = xxabs(vector_vs2(i));
            }

        DBG_VECTOR_VF;

        return 0;
    }

    /**
     * vsgnjn_vv() vfsgnjn.vv
     * @param vs2 源操作向量二基地址
     * @param vs1 源操作向量一基地址
     * @param vd 目的向量基地址
     * @param vm 不可屏蔽标识， vm=0 可屏蔽， vm=1不可屏蔽
     * @param v0 mask向量基地址
     * @param vl 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vsgnjn_vv(Type *vs2, Type *vs1, Type *vd, int vm, MaskType *v0, int vl)
    {
        VsgnjVecMap vector_vs2(vs2, vl);
        VsgnjVecMap vector_vs1(vs1, vl);
        VsgnjVecMap vector_vd(vd, vl);
        VsgnjMaskVecMap vector_v0(v0, vl);

        if (!vm) {
            for (int i = 0; i < vl; i++) {
                if (vector_v0(i) & 0x1) {
                    if (*((MaskType *)&vector_vs1(i)) & (1<<(sizeof(Type)*8)-1))
                        vector_vd(i) = xxabs(vector_vs2(i));
                    else
                        vector_vd(i) = -xxabs(vector_vs2(i));
                }
            }
        } else
            for (int i = 0; i < vl; i++) {
                if (*((MaskType *)&vector_vs1(i)) & (1<<(sizeof(Type)*8)-1))
                    vector_vd(i) = xxabs(vector_vs2(i));
                else
                    vector_vd(i) = -xxabs(vector_vs2(i));
            }


        DBG_VECTOR_VV;

        return 0;
    }

    /**
     * vsgnjn_vf() vfsgnjn.vf
     * @param vs2 源操作向量基地址
     * @param rs1 源标量操作数
     * @param vd 目的向量基地址
     * @param vm 不可屏蔽标识， vm=0 可屏蔽， vm=1不可屏蔽
     * @param v0 mask向量基地址
     * @param vl 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vsgnjn_vf(Type *vs2, Type rs1, Type *vd, int vm, MaskType *v0, int vl)
    {
        VsgnjVecMap vector_vs2(vs2, vl);
        VsgnjVecMap vector_vd(vd, vl);
        VsgnjMaskVecMap vector_v0(v0, vl);

        if (!vm) {
            for (int i = 0; i < vl; i++) {
                if (vector_v0(i) & 0x1) {
                    if (*((MaskType *)&rs1) & (1<<(sizeof(Type)*8)-1))
                        vector_vd(i) = xxabs(vector_vs2(i));
                    else
                        vector_vd(i) = -xxabs(vector_vs2(i));
                }
            }
        } else
            for (int i = 0; i < vl; i++) {
                if (*((MaskType *)&rs1) & (1<<(sizeof(Type)*8)-1))
                    vector_vd(i) = xxabs(vector_vs2(i));
                else
                    vector_vd(i) = -xxabs(vector_vs2(i));
            }


        DBG_VECTOR_VF;

        return 0;
    }

    /**
     * vsgnjx_vv() vfsgnjx.vv
     * @param vs2 源操作向量二基地址
     * @param vs1 源操作向量一基地址
     * @param vd 目的向量基地址
     * @param vm 不可屏蔽标识， vm=0 可屏蔽， vm=1不可屏蔽
     * @param v0 mask向量基地址
     * @param vl 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vsgnjx_vv(Type *vs2, Type *vs1, Type *vd, int vm, MaskType *v0, int vl)
    {
        VsgnjVecMap vector_vs2(vs2, vl);
        VsgnjVecMap vector_vs1(vs1, vl);
        VsgnjVecMap vector_vd(vd, vl);
        VsgnjMaskVecMap vector_v0(v0, vl);

        if (!vm) {
            for (int i = 0; i < vl; i++) {
                if (vector_v0(i) & 0x1) {
                    if ((*((MaskType *)&vector_vs1(i)) & (1<<(sizeof(Type)*8)-1)) == 
                        (*((MaskType *)&vector_vs2(i)) & (1<<(sizeof(Type)*8)-1)))
                        vector_vd(i) = xxabs(vector_vs2(i));
                    else
                        vector_vd(i) = -xxabs(vector_vs2(i));
                }
            }
        } else
            for (int i = 0; i < vl; i++) {
                if ((*((MaskType *)&vector_vs1(i)) & (1<<(sizeof(Type)*8)-1)) == 
                    (*((MaskType *)&vector_vs2(i)) & (1<<(sizeof(Type)*8)-1)))
                    vector_vd(i) = xxabs(vector_vs2(i));
                else
                    vector_vd(i) = -xxabs(vector_vs2(i));
            }

        DBG_VECTOR_VV;

        return 0;
    }

    /**
     * vsgnjx_vf() vfsgnjx.vf
     * @param vs2 源操作向量基地址
     * @param rs1 源标量操作数
     * @param vd 目的向量基地址
     * @param vm 不可屏蔽标识， vm=0 可屏蔽， vm=1不可屏蔽
     * @param v0 mask向量基地址
     * @param vl 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vsgnjx_vf(Type *vs2, Type rs1, Type *vd, int vm, MaskType *v0, int vl)
    {
        VsgnjVecMap vector_vs2(vs2, vl);
        VsgnjVecMap vector_vd(vd, vl);
        VsgnjMaskVecMap vector_v0(v0, vl);

        if (!vm) {
            for (int i = 0; i < vl; i++) {
                if (vector_v0(i) & 0x1) {
                    if ((*((MaskType *)&rs1) & (1<<(sizeof(Type)*8)-1)) == 
                        (*((MaskType *)&vector_vs2(i)) & (1<<(sizeof(Type)*8)-1)))
                        vector_vd(i) = xxabs(vector_vs2(i));
                    else
                        vector_vd(i) = -xxabs(vector_vs2(i));
                }
            }
        } else
            for (int i = 0; i < vl; i++) {
                if ((*((MaskType *)&rs1) & (1<<(sizeof(Type)*8)-1)) == 
                    (*((MaskType *)&vector_vs2(i)) & (1<<(sizeof(Type)*8)-1)))
                    vector_vd(i) = xxabs(vector_vs2(i));
                else
                    vector_vd(i) = -xxabs(vector_vs2(i));
            }

        DBG_VECTOR_VF;

        return 0;
    }
};

/**
 * @brief 向量比较指令
 *
 * 比较指令的作用通常是为了产生屏蔽向量的值。比较指令包括相等(==),不相等(!=),
 * 大于(>),小于(<),大于等于(>=),小于等于(<=)等类型
 */
template <typename InType, typename OutType, typename MaskType>
class Vcompare
{
  public:
    int debug;

    Vcompare(): debug(GLOBAL_DBG)
    {

    }

    typedef Map<Matrix<InType, 1, Dynamic>> VcompareInVecMap;
    typedef Map<Matrix<OutType, 1, Dynamic>> VcompareOutVecMap;
    typedef Map<Matrix<MaskType, 1, Dynamic>> VcompareMaskVecMap;

    /**
     * veq_vf()   vfeq.vf vd, vs2, rs1, vm   Compare equal(==)
     * @param vs2 源操作向量基地址
     * @param rs1 源标量操作数
     * @param vd 目的向量基地址
     * @param vm 不可屏蔽标识， vm=0 可屏蔽， vm=1不可屏蔽
     * @param v0 mask向量基地址
     * @param vl 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int veq_vf(InType *vs2, InType rs1, OutType *vd, int vm, MaskType *v0, int vl)
    {
        VcompareInVecMap vector_vs2(vs2, vl);
        VcompareOutVecMap vector_vd(vd, vl);
        VcompareMaskVecMap vector_v0(v0, vl);

        #define VEQ_VF      do {             \
            if (vector_vs2(i) == rs1)        \
                vector_vd(i) = (OutType)1;   \
            else                             \
                vector_vd(i) = (OutType)0;   \
        } while(0)

        for (int i = 0; i < vl; i++) {
            if (!vm) {
                if (vector_v0(i) & 0x1)
                    VEQ_VF;
            } else
                VEQ_VF;
        }

        DBG_VECTOR_VF;

        return 0;
    }

    /**
     * veq_vv() vfeq.vv vd, vs2, vs1, vm    Compare equal(==)
     * @param vs2 源操作向量二基地址
     * @param vs1 源操作向量一基地址
     * @param vd 目的向量基地址
     * @param vm 不可屏蔽标识， vm=0 可屏蔽， vm=1不可屏蔽
     * @param v0 mask向量基地址
     * @param vl 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int veq_vv(InType *vs2, InType *vs1, OutType *vd, int vm, MaskType *v0, int vl)
    {
        VcompareInVecMap vector_vs2(vs2, vl);
        VcompareInVecMap vector_vs1(vs1, vl);
        VcompareOutVecMap vector_vd(vd, vl);
        VcompareMaskVecMap vector_v0(v0, vl);

        #define VEQ_VV      do {                   \
            if (vector_vs2(i) == vector_vs1(i))    \
                vector_vd(i) = (OutType)1;         \
            else                                   \
                vector_vd(i) = (OutType)0;         \
        } while(0)

        for (int i = 0; i < vl; i++) {
            if (!vm) {
                if (vector_v0(i) & 0x1)
                    VEQ_VV;
            } else
                VEQ_VV;
        }

        DBG_VECTOR_VV;

        return 0;
    }

    /**
     * vne_vf()   vfne.vf vd, vs2, rs1, vm  Compare not equal(!=)
     * @param vs2 源操作向量基地址
     * @param rs1 源标量操作数
     * @param vd 目的向量基地址
     * @param vm 不可屏蔽标识， vm=0 可屏蔽， vm=1不可屏蔽
     * @param v0 mask向量基地址
     * @param vl 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vne_vf(InType *vs2, InType rs1, OutType *vd, int vm, MaskType *v0, int vl)
    {
        VcompareInVecMap vector_vs2(vs2, vl);
        VcompareOutVecMap vector_vd(vd, vl);
        VcompareMaskVecMap vector_v0(v0, vl);

        #define VNE_VF     do {               \
            if (vector_vs2(i) != rs1)         \
                vector_vd(i) = (OutType)1;    \
            else                              \
                vector_vd(i) = (OutType)0;    \
        } while(0)

        for (int i = 0; i < vl; i++) {
            if (!vm) {
                if (vector_v0(i) & 0x1)
                    VNE_VF;
            } else
                VNE_VF;
        }

        DBG_VECTOR_VF;

        return 0;
    }

    /**
     * vne_vv()    vfne.vv vd, vs2, vs1, vm   Compare not equal(!=)
     * @param vs2 源操作向量二基地址
     * @param vs1 源操作向量一基地址
     * @param vd 目的向量基地址
     * @param vm 不可屏蔽标识， vm=0 可屏蔽， vm=1不可屏蔽
     * @param v0 mask向量基地址
     * @param vl 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vne_vv(InType *vs2, InType *vs1, OutType *vd, int vm, MaskType *v0, int vl)
    {
        VcompareInVecMap vector_vs2(vs2, vl);
        VcompareInVecMap vector_vs1(vs1, vl);
        VcompareOutVecMap vector_vd(vd, vl);
        VcompareMaskVecMap vector_v0(v0, vl);

        #define VNE_VV    do {                       \
            if (vector_vs2(i) != vector_vs1(i))      \
                vector_vd(i) = (OutType)1;           \
            else                                     \
                vector_vd(i) = (OutType)0;           \
        } while(0)

        for (int i = 0; i < vl; i++) {
            if (!vm) {
                if (vector_v0(i) & 0x1)
                    VNE_VV;
            } else
                VNE_VV;
        }

        DBG_VECTOR_VV;

        return 0;
    }

    /**
     * vlt_vf()   vflt.vf vd, vs2, rs1, vm  Compare less than(<)
     * @param vs2 源操作向量基地址
     * @param rs1 源标量操作数
     * @param vd 目的向量基地址
     * @param vm 不可屏蔽标识， vm=0 可屏蔽， vm=1不可屏蔽
     * @param v0 mask向量基地址
     * @param vl 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vlt_vf(InType *vs2, InType rs1, OutType *vd, int vm, MaskType *v0, int vl)
    {
        VcompareInVecMap vector_vs2(vs2, vl);
        VcompareOutVecMap vector_vd(vd, vl);
        VcompareMaskVecMap vector_v0(v0, vl);

        #define VLT_VF      do {               \
            if (vector_vs2(i) < rs1)           \
                vector_vd(i) = (OutType)1;     \
            else                               \
                vector_vd(i) = (OutType)0;     \
        } while(0)

        for (int i = 0; i < vl; i++) {
            if (!vm) {
                if (vector_v0(i) & 0x1)
                    VLT_VF;
            } else
                VLT_VF;
        }

        DBG_VECTOR_VF;

        return 0;
    }

    /**
     * vlt_vv()    vflt.vv vd, vs2, vs1, vm   Compare less than(<)
     * @param vs2 源操作向量二基地址
     * @param vs1 源操作向量一基地址
     * @param vd 目的向量基地址
     * @param vm 不可屏蔽标识， vm=0 可屏蔽， vm=1不可屏蔽
     * @param v0 mask向量基地址
     * @param vl 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vlt_vv(InType *vs2, InType *vs1, OutType *vd, int vm, MaskType *v0, int vl)
    {
        VcompareInVecMap vector_vs2(vs2, vl);
        VcompareInVecMap vector_vs1(vs1, vl);
        VcompareOutVecMap vector_vd(vd, vl);
        VcompareMaskVecMap vector_v0(v0, vl);

        #define VLT_VV       do {                 \
            if (vector_vs2(i) < vector_vs1(i))    \
                vector_vd(i) = (OutType)1;        \
            else                                  \
                vector_vd(i) = (OutType)0;        \
        } while(0)

        for (int i = 0; i < vl; i++) {
            if (!vm) {
                if (vector_v0(i) & 0x1)
                    VLT_VV;
            } else
                VLT_VV;
        }

        DBG_VECTOR_VV;

        return 0;
    }

    /**
     * vle_vf()   vfle.vf vd, vs2, rs1, vm   Compare less than or equal(<=)
     * @param vs2 源操作向量基地址
     * @param rs1 源标量操作数
     * @param vd 目的向量基地址
     * @param vm 不可屏蔽标识， vm=0 可屏蔽， vm=1不可屏蔽
     * @param v0 mask向量基地址
     * @param vl 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vle_vf(InType *vs2, InType rs1, OutType *vd, int vm, MaskType *v0, int vl)
    {
        VcompareInVecMap vector_vs2(vs2, vl);
        VcompareOutVecMap vector_vd(vd, vl);
        VcompareMaskVecMap vector_v0(v0, vl);

        #define VLE_VF         do {         \
            if (vector_vs2(i) <= rs1)       \
                vector_vd(i) = (OutType)1;  \
            else                            \
                vector_vd(i) = (OutType)0;  \
        } while(0)

        for (int i = 0; i < vl; i++) {
            if (!vm) {
                if (vector_v0(i) & 0x1)
                    VLE_VF;
            } else
                VLE_VF;
        }

        DBG_VECTOR_VF;

        return 0;
    }

    /**
     * vle_vv()    vfle.vv vd, vs2, vs1, vm   Compare less than or equal(<=)
     * @param vs2 源操作向量二基地址
     * @param vs1 源操作向量一基地址
     * @param vd 目的向量基地址
     * @param vm 不可屏蔽标识， vm=0 可屏蔽， vm=1不可屏蔽
     * @param v0 mask向量基地址
     * @param vl 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vle_vv(InType *vs2, InType *vs1, OutType *vd, int vm, MaskType *v0, int vl)
    {
        VcompareInVecMap vector_vs2(vs2, vl);
        VcompareInVecMap vector_vs1(vs1, vl);
        VcompareOutVecMap vector_vd(vd, vl);
        VcompareMaskVecMap vector_v0(v0, vl);

        #define VLE_VV    do {                   \
            if (vector_vs2(i) <= vector_vs1(i))  \
                vector_vd(i) = (OutType)1;       \
            else                                 \
                vector_vd(i) = (OutType)0;       \
        } while(0)

        for (int i = 0; i < vl; i++) {
            if (!vm) {
                if (vector_v0(i) & 0x1)
                    VLE_VV;
            } else
                VLE_VV;
        }

        DBG_VECTOR_VV;

        return 0;
    }

    /**
     * vgt_vf()   vfgt.vf vd, vs2, rs1, vm   Compare greater than(>)
     * @param vs2 源操作向量基地址
     * @param rs1 源标量操作数
     * @param vd 目的向量基地址
     * @param vm 不可屏蔽标识， vm=0 可屏蔽， vm=1不可屏蔽
     * @param v0 mask向量基地址
     * @param vl 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vgt_vf(InType *vs2, InType rs1, OutType *vd, int vm, MaskType *v0, int vl)
    {
        VcompareInVecMap vector_vs2(vs2, vl);
        VcompareOutVecMap vector_vd(vd, vl);
        VcompareMaskVecMap vector_v0(v0, vl);

        #define VGT_VF    do {                \
            if (vector_vs2(i) > rs1)          \
                vector_vd(i) = (OutType)1;    \
            else                              \
                vector_vd(i) = (OutType)0;    \
        } while(0)

        for (int i = 0; i < vl; i++) {
            if (!vm) {
                if (vector_v0(i) & 0x1)
                    VGT_VF;
            } else
                VGT_VF;
        }

        DBG_VECTOR_VF;

        return 0;
    }

    /**
     * vgt_vv()    vfgt.vv vd, vs2, vs1, vm   Compare greater than(>)
     * @param vs2 源操作向量二基地址
     * @param vs1 源操作向量一基地址
     * @param vd 目的向量基地址
     * @param vm 不可屏蔽标识， vm=0 可屏蔽， vm=1不可屏蔽
     * @param v0 mask向量基地址
     * @param vl 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vgt_vv(InType *vs2, InType *vs1, OutType *vd, int vm, MaskType *v0, int vl)
    {
        VcompareInVecMap vector_vs2(vs2, vl);
        VcompareInVecMap vector_vs1(vs1, vl);
        VcompareOutVecMap vector_vd(vd, vl);
        VcompareMaskVecMap vector_v0(v0, vl);

        #define VGT_VV     do {                  \
            if (vector_vs2(i) > vector_vs1(i))   \
                vector_vd(i) = (OutType)1;       \
            else                                 \
                vector_vd(i) = (OutType)0;       \
        } while(0)

        for (int i = 0; i < vl; i++) {
            if (!vm) {
                if (vector_v0(i) & 0x1)
                    VGT_VV;
            } else
                VGT_VV;
        }

        DBG_VECTOR_VV;

        return 0;
    }

    /**
     * vge_vf()   vfge.vf vd, vs2, rs1, vm   Compare greater than or equal (>=)
     * @param vs2 源操作向量基地址
     * @param rs1 源标量操作数
     * @param vd 目的向量基地址
     * @param vm 不可屏蔽标识， vm=0 可屏蔽， vm=1不可屏蔽
     * @param v0 mask向量基地址
     * @param vl 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vge_vf(InType *vs2, InType rs1, OutType *vd, int vm, MaskType *v0, int vl)
    {
        VcompareInVecMap vector_vs2(vs2, vl);
        VcompareOutVecMap vector_vd(vd, vl);
        VcompareMaskVecMap vector_v0(v0, vl);

        #define VGE_VF     do {                 \
            if (vector_vs2(i) >= rs1)           \
                vector_vd(i) = (OutType)1;      \
            else                                \
                vector_vd(i) = (OutType)0;      \
        } while(0)

        for (int i = 0; i < vl; i++) {
            if (!vm) {
                if (vector_v0(i) & 0x1)
                    VGE_VF;
            } else
                VGE_VF;
        }

        DBG_VECTOR_VF;

        return 0;
    }

    /**
     * vge_vv()    vfge.vv vd, vs2, vs1, vm   Compare greater than or equal (>=)
     * @param vs2 源操作向量二基地址
     * @param vs1 源操作向量一基地址
     * @param vd 目的向量基地址
     * @param vm 不可屏蔽标识， vm=0 可屏蔽， vm=1不可屏蔽
     * @param v0 mask向量基地址
     * @param vl 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vge_vv(InType *vs2, InType *vs1, OutType *vd, int vm, MaskType *v0, int vl)
    {
        VcompareInVecMap vector_vs2(vs2, vl);
        VcompareInVecMap vector_vs1(vs1, vl);
        VcompareOutVecMap vector_vd(vd, vl);
        VcompareMaskVecMap vector_v0(v0, vl);

        #define VGE_VV    do {                    \
            if (vector_vs2(i) >= vector_vs1(i))   \
                vector_vd(i) = (OutType)1;        \
            else                                  \
                vector_vd(i) = (OutType)0;        \
        } while(0)

        for (int i = 0; i < vl; i++) {
            if (!vm) {
                if (vector_v0(i) & 0x1)
                    VGE_VV;
            } else
                VGE_VV;
        }

        DBG_VECTOR_VV;

        return 0;
    }

};

/**
 * @brief 向量归约操作
 *
 * 以一个向量寄存器和一个向量寄存器元素 0 的标量为
 * 输入, 对其做 reduction 操作
 *
 */
template <typename Type, typename MaskType>
class Vfred
{
  public:
    int debug;

    Vfred(): debug(GLOBAL_DBG)
    {

    }

    typedef Map<Matrix<Type, 1, Dynamic>> VfredVecMap;
    typedef Map<Matrix<MaskType, 1, Dynamic>> VfredMaskVecMap;

    /**
     * vfredosum_vs() vfredosum.vs   vd[0] = vs1[0] + sum(vs2)
     *
     * @param vs2 源操作向量基地址
     * @param vs1 原操作数基地址
     * @param vd 目的数存放地址
     * @param vm 不可屏蔽标识， vm=0 可屏蔽， vm=1不可屏蔽
     * @param v0 mask向量基地址
     * @param vl 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vfredosum_vs(Type *vs2, Type *vs1, Type *vd, int vm, MaskType *v0, int vl)
    {
        VfredVecMap vector_vs2(vs2, vl);
        VfredMaskVecMap vector_v0(v0, vl);
        Type val;

        val = vs1[0];
        for (int i = 0; i < vl; i++)
            val += ((!vm && !(vector_v0[i] & 0x1)) ? (Type)0 : vector_vs2(i));
        vd[0] = val;

        if (debug) {
            cout << __FUNCTION__ << endl;
            cout << "vs2:\n" << vector_vs2 << endl;
            cout << "vs1:\n" << vs1[0] << endl;
            cout << "vd:\n" << vd[0] << endl;
            cout << "vm:\n" << vm << endl;
            cout << "v0:\n" << vector_v0 << endl;
            cout << "vl:\n" << vl << endl;
        }

        return 0;
    }

    /**
     * vfredsum_vs() vfredsum.vs   vd[0] = vs1[0] + sum(vs2)
     *
     * @param vs2 源操作向量基地址
     * @param vs1 原操作数基地址
     * @param vd 目的数存放地址
     * @param vm 不可屏蔽标识， vm=0 可屏蔽， vm=1不可屏蔽
     * @param v0 mask向量基地址
     * @param vl 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vfredsum_vs(Type *vs2, Type *vs1, Type *vd, int vm, MaskType *v0, int vl)
    {
        VfredVecMap vector_vs2(vs2, vl);
        VfredMaskVecMap vector_v0(v0, vl);
        Type val;
        val = vs1[0];
        for (int i = 0; i < vl; i++)
            val += ((!vm && !(vector_v0[i] & 0x1)) ? (Type)0 : vector_vs2(i));
        vd[0] = val;

        if (debug) {
            cout << __FUNCTION__ << endl;
            cout << "vs2:\n" << vector_vs2 << endl;
            cout << "vs1:\n" << vs1[0] << endl;
            cout << "vd:\n" << vd[0] << endl;
            cout << "vm:\n" << vm << endl;
            cout << "v0:\n" << vector_v0 << endl;
            cout << "vl:\n" << vl << endl;
        }
        
        return 0;
    }
    
    /**
     * vfredmin_vs() vfredmin.vs   vd[0] = min(vs1[0], vs2[0]..vs2[vl-1])
     *
     * @param vs2 源操作向量基地址
     * @param vs1 原操作数基地址
     * @param vd 目的数存放地址
     * @param vm 不可屏蔽标识， vm=0 可屏蔽， vm=1不可屏蔽
     * @param v0 mask向量基地址
     * @param vl 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vfredmin_vs(Type *vs2, Type *vs1, Type *vd, int vm, MaskType *v0, int vl)
    {
        VfredVecMap vector_vs2(vs2, vl);
        VfredMaskVecMap vector_v0(v0, vl);
        Type val;
        val = vs1[0];
        for (int i = 0; i < vl; i++)
            if (vm || (vector_v0[i] & 0x1))
                val = (val > vector_vs2(i)) ? vector_vs2(i) : val;
        vd[0] = val;

        if (debug) {
            cout << __FUNCTION__ << endl;
            cout << "vs2:\n" << vector_vs2 << endl;
            cout << "vs1:\n" << vs1[0] << endl;
            cout << "vd:\n" << vd[0] << endl;
            cout << "vm:\n" << vm << endl;
            cout << "v0:\n" << vector_v0 << endl;
            cout << "vl:\n" << vl << endl;
        }
        
        return 0;   
     }

    /**
     * vfredmax_vs() vfredmax.vs   vd[0] = max(vs1[0], vs2[0]..vs2[vl-1])
     *
     * @param vs2 源操作向量基地址
     * @param vs1 原操作数基地址
     * @param vd 目的数存放地址
     * @param vm 不可屏蔽标识， vm=0 可屏蔽， vm=1不可屏蔽
     * @param v0 mask向量基地址
     * @param vl 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vfredmax_vs(Type *vs2, Type *vs1, Type *vd, int vm, MaskType *v0, int vl)
    {
        VfredVecMap vector_vs2(vs2, vl);
        VfredMaskVecMap vector_v0(v0, vl);
        Type val;

        val = vs1[0];
        for (int i = 0; i < vl; i++)
            if (vm || (vector_v0[i] & 0x1))
                val = (val < vector_vs2(i)) ? vector_vs2(i) : val;
        vd[0] = val;

        if (debug) {
            cout << __FUNCTION__ << endl;
            cout << "vs2:\n" << vector_vs2 << endl;
            cout << "vs1:\n" << vs1[0] << endl;
            cout << "vd:\n" << vd[0] << endl;
            cout << "vm:\n" << vm << endl;
            cout << "v0:\n" << vector_v0 << endl;
            cout << "vl:\n" << vl << endl;
        }
        
        return 0;
     }
};

/**
 * @brief 向量屏蔽寄存器逻辑指令
 *
 * 向量屏蔽寄存器逻辑指令操作屏蔽寄存器
 *
 */
template <typename Type>
class Vlogic
{
  public:
    int debug;

    Vlogic(): debug(GLOBAL_DBG)
    {

    }

    typedef Map<Matrix<Type, 1, Dynamic>> VlogicVecMap;

    /**
     * vmand_mm() vmand.mm vd = vs1 & vs2
     *
     * @param vs2 源操作向量基地址
     * @param vs1 原操作数基地址
     * @param vd 目的数存放地址
     * @param vl 向量长度(准确的说应该是个数)
     * @param vstart 起始元素
     * @param vlmax 最大元素数量
     * @return 执行结果
     */
    int vmand_mm(Type *vs2, Type *vs1, Type *vd, int vl, uint32_t vstart, uint32_t vlmax)
    {
        VlogicVecMap vector_vs2(vs2, vl);
        VlogicVecMap vector_vs1(vs1, vl);
        VlogicVecMap vector_vd(vd, vlmax);

        for (int i = vstart; i < vl; i++)
            vector_vd(i) = vector_vs2(i) & vector_vs1(i);
        
        for (int j = vl; j < vlmax; j++)
            vector_vd(j) = 0;

        DBG_INFO4(debug, vector_vs2, vector_vs1, vector_vd, vl);
        return 0;
    }

    /**
     * vmnand_mm() vmnand.mm vd = ~(vs1 & vs2)
     *
     * @param vs2 源操作向量基地址
     * @param vs1 原操作数基地址
     * @param vd 目的数存放地址
     * @param vl 向量长度(准确的说应该是个数)
     * @param vstart 起始元素
     * @param vlmax 最大元素数量
     * @return 执行结果
     */
    int vmnand_mm(Type *vs2, Type *vs1, Type *vd, int vl, uint32_t vstart, uint32_t vlmax)
    {
        VlogicVecMap vector_vs2(vs2, vl);
        VlogicVecMap vector_vs1(vs1, vl);
        VlogicVecMap vector_vd(vd, vlmax);

        for (int i = vstart; i < vl; i++)
            vector_vd(i) = ~(vector_vs2(i) & vector_vs1(i));

        for (int j = vl; j < vlmax; j++)
            vector_vd(j) = 0;

        DBG_INFO4(debug, vector_vs2, vector_vs1, vector_vd, vl);
        return 0;
    }

    /**
     * vmandnot_mm() vmandnot.mm vd = vs2 & ~vs1
     *
     * @param vs2 源操作向量基地址
     * @param vs1 原操作数基地址
     * @param vd 目的数存放地址
     * @param vl 向量长度(准确的说应该是个数)
     * @param vstart 起始元素
     * @param vlmax 最大元素数量
     * @return 执行结果
     */
    int vmandnot_mm(Type *vs2, Type *vs1, Type *vd, int vl, uint32_t vstart, uint32_t vlmax)
    {
        VlogicVecMap vector_vs2(vs2, vl);
        VlogicVecMap vector_vs1(vs1, vl);
        VlogicVecMap vector_vd(vd, vlmax);

        for (int i=vstart; i < vl; i++)
            vector_vd(i) = vector_vs2(i) & (~vector_vs1(i));

        for (int j = vl; j < vlmax; j++)
            vector_vd(j) = 0;

        DBG_INFO4(debug, vector_vs2, vector_vs1, vector_vd, vl);
        return 0;
    }

    /**
     * vmxor_mm() vmxor.mm vd = vs1 ^ vs2
     *
     * @param vs2 源操作向量基地址
     * @param vs1 原操作数基地址
     * @param vd 目的数存放地址
     * @param vl 向量长度(准确的说应该是个数)
     * @param vstart 起始元素
     * @param vlmax 最大元素数量
     * @return 执行结果
     */
    int vmxor_mm(Type *vs2, Type *vs1, Type *vd, int vl, uint32_t vstart, uint32_t vlmax)
    {
        VlogicVecMap vector_vs2(vs2, vl);
        VlogicVecMap vector_vs1(vs1, vl);
        VlogicVecMap vector_vd(vd, vlmax);

        for (int i=vstart; i < vl; i++)
            vector_vd(i) = vector_vs2(i) ^ vector_vs1(i);

        for (int j = vl; j < vlmax; j++)
            vector_vd(j) = 0;

        DBG_INFO4(debug, vector_vs2, vector_vs1, vector_vd, vl);
        return 0;
    }

    /**
     * vmor_mm() vmor.mm vd = vs1 | vs2
     *
     * @param vs2 源操作向量基地址
     * @param vs1 原操作数基地址
     * @param vd 目的数存放地址
     * @param vl 向量长度(准确的说应该是个数)
     * @param vstart 起始元素
     * @param vlmax 最大元素数量
     * @return 执行结果
     */
    int vmor_mm(Type *vs2, Type *vs1, Type *vd, int vl, uint32_t vstart, uint32_t vlmax)
    {
        VlogicVecMap vector_vs2(vs2, vl);
        VlogicVecMap vector_vs1(vs1, vl);
        VlogicVecMap vector_vd(vd, vlmax);

        for (int i=vstart; i < vl; i++)
            vector_vd(i) = vector_vs2(i) | vector_vs1(i);

        for (int j = vl; j < vlmax; j++)
            vector_vd(j) = 0;

        DBG_INFO4(debug, vector_vs2, vector_vs1, vector_vd, vl);
        return 0;
    }

    /**
     * vmnor_mm() vmnor.mm vd = ~(vs1 | vs2)
     *
     * @param vs2 源操作向量基地址
     * @param vs1 原操作数基地址
     * @param vd 目的数存放地址
     * @param vl 向量长度(准确的说应该是个数)
     * @param vstart 起始元素
     * @param vlmax 最大元素数量
     * @return 执行结果
     */
    int vmnor_mm(Type *vs2, Type *vs1, Type *vd, int vl, uint32_t vstart, uint32_t vlmax)
    {
        VlogicVecMap vector_vs2(vs2, vl);
        VlogicVecMap vector_vs1(vs1, vl);
        VlogicVecMap vector_vd(vd, vlmax);

        for (int i=0; i < vl; i++)
            vector_vd(i) = ~(vector_vs2(i) | vector_vs1(i));

        for (int j = vl; j < vlmax; j++)
            vector_vd(j) = 0;

        DBG_INFO4(debug, vector_vs2, vector_vs1, vector_vd, vl);
        return 0;
    }

    /**
     * vmornot_mm() vmornot.mm vd = vs2 | ~vs1
     *
     * @param vs2 源操作向量基地址
     * @param vs1 原操作数基地址
     * @param vd 目的数存放地址
     * @param vl 向量长度(准确的说应该是个数)
     * @param vstart 起始元素
     * @param vlmax 最大元素数量
     * @return 执行结果
     */
    int vmornot_mm(Type *vs2, Type *vs1, Type *vd, int vl, uint32_t vstart, uint32_t vlmax)
    {
        VlogicVecMap vector_vs2(vs2, vl);
        VlogicVecMap vector_vs1(vs1, vl);
        VlogicVecMap vector_vd(vd, vlmax);

        for (int i=vstart; i < vl; i++)
            vector_vd(i) = vector_vs2(i) | (~vector_vs1(i));

        for (int j = vl; j < vlmax; j++)
            vector_vd(j) = 0;

        DBG_INFO4(debug, vector_vs2, vector_vs1, vector_vd, vl);
        return 0;
    }

    /**
     * vmxor_mm() vmxor.mm vd = ~(vs2 ^ vs1)
     *
     * @param vs2 源操作向量基地址
     * @param vs1 原操作数基地址
     * @param vd 目的数存放地址
     * @param vl 向量长度(准确的说应该是个数)
     * @param vstart 起始元素
     * @param vlmax 最大元素数量
     * @return 执行结果
     */
    int vmxnor_mm(Type *vs2, Type *vs1, Type *vd, int vl, uint32_t vstart, uint32_t vlmax)
    {
        VlogicVecMap vector_vs2(vs2, vl);
        VlogicVecMap vector_vs1(vs1, vl);
        VlogicVecMap vector_vd(vd, vlmax);

        for (int i=0; i < vl; i++)
            vector_vd(i) = ~(vector_vs2(i) ^ vector_vs1(i));

        for (int j = vl; j < vlmax; j++)
            vector_vd(j) = 0;

        DBG_INFO4(debug, vector_vs2, vector_vs1, vector_vd, vl);
        return 0;
    }
};

/**
 * @brief 向量屏蔽计数
 *
 * 指令用来统计向量源屏蔽寄存器中 active 元素里的最低有效位为 1 的
 * 屏蔽元素的数量,然后把结果写到标量寄存器 x 中。
 *
 */
template <typename Type, typename MaskType>
class Vmpopc
{
  public:
    int debug;

    Vmpopc(): debug(GLOBAL_DBG)
    {

    }

    typedef Map<Matrix<Type, 1, Dynamic>> VmpopcVecMap;
    typedef Map<Matrix<MaskType, 1, Dynamic>> VmpopcMaskVecMap;

    /**
     * vmpopc_m() vmpopc.m  x[rd] = sum_i ( vs2[i].LSB && v0[i].LSB )
     *
     * @param vs2 源操作向量基地址
     * @param rd 目的数存放地址
     * @param vm mask 使能标记，0使能
     * @param mask 元素mask标记，最低位有效
     * @param vl 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vmpopc_m(Type *vs2, int *rd, int vm, MaskType *mask, int vl)
    {
        int number = 0;
        VmpopcVecMap vector_vs2(vs2, vl);
        VmpopcMaskVecMap vector_mask(mask, vl);
        
        for (int i=0; i < vl; i++)
            number += (!vm && !(vector_mask(i) & 0x1)) ? 0 : vector_vs2(i) & 0x1;

        *rd = number;

        DBG_INFO5(debug, vector_vs2, vm, vector_mask, number, vl);
        return 0;
    }
};

/**
 * @brief find-first-set mask bit
 *
 * 从索引0开始, 依次扫描源操作数指定的屏蔽向量, 寻找第一个mask有效的mask元素;
 * 如果找到这样的 mask 元素, 就把对应的元素索引值写到目的寄存器 rd 中.
 * 如果没有找到即写入-1.
 *
 */
template <typename Type, typename MaskType>
class Vmfirst
{
  public:
    int debug;

    Vmfirst(): debug(GLOBAL_DBG)
    {

    }

    typedef Map<Matrix<Type, 1, Dynamic>> VmfirstVecMap;
    typedef Map<Matrix<MaskType, 1, Dynamic>> VmfirstMaskVecMap;

    /**
     * vmfirst_m()  x[rd] = first(vs2[i].LSB && v0[i].LSB)
     *
     * @param vs2 源操作向量基地址
     * @param rd 目的数存放地址
     * @param vm mask 使能标记，0使能
     * @param mask 元素mask标记，最低位有效
     * @param vl 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vmfirst_m(Type *vs2, int *rd, int vm, MaskType *mask, int vl)
    {
        VmfirstVecMap vector_vs2(vs2, vl);
        VmfirstMaskVecMap vector_mask(mask, vl);
        
        *rd = -1;
        for (int i=0; i < vl; i++)
            if ((vm || (vector_mask(i) & 0x1)) && (vector_vs2(i) & 0x1)) {
                *rd = i;
                break;
            }

        DBG_INFO4(debug, vector_vs2, vm, vector_mask, vl);
        return 0;
    }
};

/**
 * @brief set-before-first mask bit
 *
 * 第一个mask有效元素之前的所有active元素将被写 1, 之后包括自己的所有active元素则被写0.
 * 
 *
 */
template <typename Type, typename MaskType>
class Vmsbf
{
  public:
    int debug;

    Vmsbf(): debug(GLOBAL_DBG)
    {

    }

    typedef Map<Matrix<Type, 1, Dynamic>> VmsbfVecMap;
    typedef Map<Matrix<MaskType, 1, Dynamic>> VmsbfMaskVecMap;

    /**
     * vmsbf_m()  vmsbf.m vd, vs2, vm
     *
     * @param vs2 源操作向量基地址
     * @param vd 目的向量存放地址
     * @param vm mask 使能标记，0使能
     * @param mask 元素mask标记，最低位有效
     * @param vl 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vmsbf_m(Type *vs2, Type *vd, int vm, MaskType *mask, int vl)
    {
        int number = 0;
        VmsbfVecMap vector_vs2(vs2, vl);
        VmsbfVecMap vector_vd(vd, vl);
        VmsbfMaskVecMap vector_mask(mask, vl);
        
        for (int i=0; i < vl; i++)
            if (vm || (vector_mask(i) & 0x1)) {
                number += vector_vs2(i) & 0x1;
                vector_vd(i) = number ? 0 : 1;
            }

        DBG_INFO5(debug, vector_vs2, vector_vd, vector_mask, number, vl);
        return 0;
    }
};

/**
 * @brief set-inclue-first mask bit
 *
 * 第一个mask有效元素及之前的所有active元素将被写 1, 之后所有active元素则被写0.
 * 
 *
 */
template <typename Type, typename MaskType>
class Vmsif
{
  public:
    int debug;

    Vmsif(): debug(GLOBAL_DBG)
    {

    }

    typedef Map<Matrix<Type, 1, Dynamic>> VmsifVecMap;
    typedef Map<Matrix<MaskType, 1, Dynamic>> VmsifMaskVecMap;

    /**
     * vmsbf_m()  vmsbf.m vd, vs2, vm
     *
     * @param vs2 源操作向量基地址
     * @param vd 目的向量存放地址
     * @param vm mask 使能标记，0使能
     * @param mask 元素mask标记，最低位有效
     * @param vl 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vmsif_m(Type *vs2, Type *vd, int vm, MaskType *mask, int vl)
    {
        int number = 0;
        VmsifVecMap vector_vs2(vs2, vl);
        VmsifVecMap vector_vd(vd, vl);
        VmsifMaskVecMap vector_mask(mask, vl);
        
        for (int i=0; i < vl; i++)
            if (vm || (vector_mask(i) & 0x1)) {
                vector_vd(i) = number ? 0 : 1;
                number += vector_vs2(i) & 0x1;
            }

        DBG_INFO5(debug, vector_vs2, vector_vd, vector_mask, number, vl);
        return 0;
    }
};

/**
 * @brief set-only-first mask bit
 *
 * 第一个mask有效元素将被写 1, 其他active元素则被写0.
 * 
 *
 */
template <typename Type, typename MaskType>
class Vmsof
{
  public:
    int debug;

    Vmsof(): debug(GLOBAL_DBG)
    {

    }

    typedef Map<Matrix<Type, 1, Dynamic>> VmsofVecMap;
    typedef Map<Matrix<MaskType, 1, Dynamic>> VmsofMaskVecMap;

    /**
     * vmsof_m()  vmsof.m vd, vs2, vm
     *
     * @param vs2 源操作向量基地址
     * @param vd 目的向量存放地址
     * @param vm mask 使能标记，0使能
     * @param mask 元素mask标记，最低位有效
     * @param vl 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vmsof_m(Type *vs2, Type *vd, int vm, MaskType *mask, int vl)
    {
        int number = 0;
        VmsofVecMap vector_vs2(vs2, vl);
        VmsofVecMap vector_vd(vd, vl);
        VmsofMaskVecMap vector_v0(mask, vl);
        
        for (int i=0; i < vl; i++)
            if (vm || (vector_v0(i) & 0x1)) {
                number += vector_vs2(i) & 0x1;
                vector_vd(i) = (number == 1) ? 0 : 1;
            }

        DBG_INFO5(debug, vector_vs2, vector_vd, vector_v0, number, vl);
        return 0;
    }
};

/**
 *
 * @brief 对源操作数的向量元素所有之前active元素的mask有效位求和.
 *
 */
template <typename Type, typename Typed, typename MaskType>
class Viota
{
  public:
    int debug;

    Viota(): debug(GLOBAL_DBG)
    {

    }

    typedef Map<Matrix<Type, 1, Dynamic>> ViotaVecMap;
    typedef Map<Matrix<Typed, 1, Dynamic>> ViotaDstVecMap;
    typedef Map<Matrix<MaskType, 1, Dynamic>> ViotaMaskVecMap;

    /**
     * viota_m()  viota.m vd, vs2, vm
     *
     * @param vs2 源操作向量基地址
     * @param vd 目的向量存放地址
     * @param vm mask 使能标记，0使能
     * @param mask 元素mask标记，最低位有效
     * @param vl 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int viota_m(Type *vs2, Typed *vd, int vm, MaskType *mask, int vl)
    {
        Type number = static_cast<Type>(0);
        ViotaVecMap vector_vs2(vs2, vl);
        ViotaDstVecMap vector_vd(vd, vl);
        ViotaMaskVecMap vector_v0(mask, vl);
        
        for (int i=0; i < vl; i++)
            if (vm || (vector_v0(i) & 0x1)) {
                vector_vd(i) = number;
                number += vector_vs2(i) & 0x1;
            }
        
        DBG_INFO4(debug, vector_vs2, vector_vd, vector_v0, vl);
        return 0;
    }
};

/**
 * 向量元素索引指令
 * @brief 把每个元素的索引写入到目标向量寄存器组里, 从0到vl.
 *        vs2 不会进行实际操作, 但必须设置成v0.
 *
 */
template <typename Type, typename Typed, typename MaskType>
class Vid
{
  public:
    int debug;

    Vid(): debug(GLOBAL_DBG)
    {

    }

    typedef Map<Matrix<Type, 1, Dynamic>> VidVecMap;
    typedef Map<Matrix<Typed, 1, Dynamic>> VidDstVecMap;
    typedef Map<Matrix<MaskType, 1, Dynamic>> VidMaskVecMap;

    /**
     * vid_v()  vid.v vd, vs2, vm
     *
     * @param vs2 源操作向量基地址
     * @param vd 目的向量存放地址
     * @param vm mask 使能标记，0使能
     * @param mask 元素mask标记，最低位有效
     * @param vl 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vid_v(Type *vs2, Typed *vd, int vm, MaskType *mask, int vl)
    {
        VidVecMap vector_vs2(vs2, vl);
        VidDstVecMap vector_vd(vd, vl);
        VidMaskVecMap vector_v0(mask, vl);
        
        for (int i=0; i < vl; i++)
            if (vm || (vector_v0(i) & 0x1)) 
                vector_vd(i) = i;
        
        DBG_INFO4(debug, vector_vs2, vector_vd, vector_v0, vl);
        return 0;
    }
};

/**
 * @brief 向量平移指令(Vector Slide Instructions) 
 *
 * slide指令对一个向量寄存器组的元素进行上下移动操作(以元素为单位)。 
 *
 */
template <typename Type, typename TypeMask>
class Vslide
{
  public:
    int debug;

    Vslide(): debug(GLOBAL_DBG)
    {

    }

    typedef Map<Matrix<Type, 1, Dynamic>> VslideVecMap;
    typedef Map<Matrix<TypeMask, 1, Dynamic>> VslideMaskVecMap;

    /**
     * vslideup_vx() vslideup.vx 
     *
     * @param vs2 源操作向量基地址
     * @param rs1 移动大小
     * @param vd 目的向量存放地址
     * @param vm mask 使能标记，0使能
     * @param v0 元素mask标记，最低位有效
     * @param vl 向量长度(准确的说应该是个数)
     * @param vstart 移动起始元素
     * @param vlmax 元素总数
     * @return 执行结果
     */
    int vslideup_vx(Type *vs2, uint32_t rs1, Type *vd, uint32_t vm, 
            TypeMask *v0, int vl, uint32_t vstart, uint32_t vlmax)
    {
        VslideVecMap vector_vs2(vs2, vl);
        VslideVecMap vector_vd(vd, vl);
        VslideMaskVecMap vector_v0(v0, vl);
        uint32_t start = (rs1 < vstart) ? vstart : rs1;

        for (int i = start; i < vl; i++)
            if (vm || (vector_v0(i) && 0x1))
                vector_vd(i) = vector_vs2(i - start);
        
        //clear tail elment
        for (int j = vl; j < vlmax; j++)
            vector_vd(j) = 0;
        
        DBG_INFO5(debug, vector_vs2, vector_vd, vector_v0, start, vm);
        return 0;
    }
    
    /**
     * vslideup_vi() vslideup.vi 
     *
     * @param vs2 源操作向量基地址
     * @param rs1 移动大小, 立即数版本
     * @param vd 目的向量存放地址
     * @param vm mask 使能标记，0使能
     * @param v0 元素mask标记，最低位有效
     * @param vl 向量长度(准确的说应该是个数)
     * @param vstart 移动起始元素
     * @param vlmax 元素总数
     * @return 执行结果
     */
    int vslideup_vi(Type *vs2, uint32_t rs1, Type *vd, uint32_t vm, 
            TypeMask *v0, int vl, uint32_t vstart, uint32_t vlmax)
    {
        return vslideup_vx(vs2, rs1, vd, vm, v0, vl, vstart, vlmax);
    }

    /**
     * vslide1up_vx() vslide1up.vx 
     *
     * @param vs2 源操作向量基地址
     * @param rs1 插入到vd[0]的数
     * @param vd 目的向量存放地址
     * @param vm mask 使能标记，0使能
     * @param v0 元素mask标记，最低位有效
     * @param vl 向量长度(准确的说应该是个数)
     * @param vstart 移动起始元素
     * @param vlmax 元素总数
     * @return 执行结果
     */
    int vslide1up_vx(Type *vs2, Type rs1, Type *vd, uint32_t vm, TypeMask *v0, 
            int vl, uint32_t vstart, uint32_t vlmax)
    {
        vd[0] = rs1;
        return vslideup_vx(vs2, 1, vd, vm, v0, vl, vstart, vlmax);
    }

    /**
     * vslidedown_vx() vslidedown.vx 
     *
     * @param vs2 源操作向量基地址
     * @param rs1 向下移动大小
     * @param vd 目的向量存放地址
     * @param vm mask 使能标记，0使能
     * @param v0 元素mask标记，最低位有效
     * @param vl 向量长度(准确的说应该是个数)
     * @param vstart 移动起始元素
     * @param vlmax 元素总数
     * @return 执行结果
     */
    int vslidedown_vx(Type *vs2, uint32_t rs1, Type *vd, uint32_t vm, 
            TypeMask *v0, int vl, uint32_t vstart, uint32_t vlmax)
    {
        VslideVecMap vector_vs2(vs2, vl);
        VslideVecMap vector_vd(vd, vl);
        VslideMaskVecMap vector_v0(v0, vl);

        for (int i = vstart; i < vl; i++)
            if (vm || (vector_v0(i) && 0x1))
                vector_vd(i) = vector_vs2(i + rs1);
        
        for (int j = vl; j < vlmax; j++)
            vector_vd(j) = 0;

        DBG_INFO5(debug, vector_vs2, vector_vd, vector_v0, vl, vstart);
        return 0;
    }
    
    /**
     * vslidedown_vi() vslidedown.vi 
     *
     * @param vs2 源操作向量基地址
     * @param rs1 向下移动大小
     * @param vd 目的向量存放地址
     * @param vm mask 使能标记，0使能
     * @param v0 元素mask标记，最低位有效
     * @param vl 向量长度(准确的说应该是个数)
     * @param vstart 移动起始元素
     * @param vlmax 元素总数
     * @return 执行结果
     */
    int vslidedown_vi(Type *vs2, uint32_t rs1, Type *vd, uint32_t vm, 
            TypeMask *v0, int vl, uint32_t vstart, uint32_t vlmax)
    {
        return vslidedown_vx(vs2, rs1, vd, vm, v0, vl, vstart, vlmax);
    }

    /**
     * vslide1down_vx() vslide1down.vx 
     *
     * @param vs2 源操作向量基地址
     * @param rs1 插入尾部的向量
     * @param vd 目的向量存放地址
     * @param vm mask 使能标记，0使能
     * @param v0 元素mask标记，最低位有效
     * @param vl 向量长度(准确的说应该是个数)
     * @param vstart 移动起始元素
     * @param vlmax 元素总数
     * @return 执行结果
     */
    int vslide1down_vx(Type *vs2, Type rs1, Type *vd, uint32_t vm, 
          TypeMask *v0, int vl, uint32_t vstart, uint32_t vlmax)
    {
        vd[vl-1] = rs1;
        return vslidedown_vx(vs2, rs1, vd, vm, v0, vl, vstart, vlmax);        
    }
};

/**
 * @brief 向量寄存器聚集指令(Vector Register Gather Instructions) 
 *
 * 向量寄存器聚集指令从第二个源操作数读取向量元素, 元素索引由第一个源向量寄存器组给出。 
 *
 */
template <typename Type, typename Type2, typename TypeMask>
class Vrgather
{
  public:
    int debug;

    Vrgather(): debug(GLOBAL_DBG)
    {

    }

    typedef Map<Matrix<Type, 1, Dynamic>> VrgatherVecMap;
    typedef Map<Matrix<Type2, 1, Dynamic>> VIndexVecMap;
    typedef Map<Matrix<TypeMask, 1, Dynamic>> VrgatherMaskVecMap;

    /**
     * vrgather_vv() vrgather.vv 
     *
     * @param vs2 源操作向量基地址
     * @param vs1 索引存放基地址
     * @param vd 目的向量存放地址
     * @param vm mask 使能标记，0使能
     * @param v0 元素mask标记，最低位有效
     * @param vl 向量长度(准确的说应该是个数)
     * @param vlmax 元素总数
     * @return 执行结果
     */
    int vrgather_vv(Type *vs2, Type2 *vs1, Type *vd, uint32_t vm, 
         TypeMask *v0, int vl, uint32_t vlmax)
    {
        VrgatherVecMap vector_vs2(vs2, vl);
        VIndexVecMap vector_vs1(vs1, vl);
        VrgatherVecMap vector_vd(vd, vl);
        VrgatherMaskVecMap vector_v0(v0, vl);
        
        for (int i = 0; i < vl; i++)
            if (vm || (vector_v0(i) & 0x1))
                vector_vd(i) = (vector_vs1(i) < vlmax) ? vector_vs2(vector_vs1(i)) : 0;
        
        DBG_INFO5(debug, vector_vs2, vector_vs1, vector_vd, vector_v0, vm);
        return 0;
    }

    /**
     * vrgather_vx() vrgather.vx 
     *
     * @param vs2 源操作向量基地址
     * @param rs1 索引值
     * @param vd 目的向量存放地址
     * @param vm mask 使能标记，0使能
     * @param v0 元素mask标记，最低位有效
     * @param vl 向量长度(准确的说应该是个数)
     * @param vlmax 元素总数
     * @return 执行结果
     */
    int vrgather_vx(Type *vs2, uint32_t rs1, Type *vd, uint32_t vm, 
         TypeMask *v0, int vl, uint32_t vlmax)
    {
        VrgatherVecMap vector_vs2(vs2, vl);
        VrgatherVecMap vector_vd(vd, vl);
        VrgatherMaskVecMap vector_v0(v0, vl);
        
        for (int i = 0; i < vl; i++)
            if (vm || (vector_v0(i) & 0x1))
                vector_vd(i) = (rs1 < vlmax) ? vector_vs2(rs1) : 0;
        
        DBG_INFO5(debug, vector_vs2, rs1, vector_vd, vector_v0, vm);
        return 0;
    }
    
    /**
     * vrgather_vi() vrgather.vi 
     *
     * @param vs2 源操作向量基地址
     * @param imm 索引值
     * @param vd 目的向量存放地址
     * @param vm mask 使能标记，0使能
     * @param v0 元素mask标记，最低位有效
     * @param vl 向量长度(准确的说应该是个数)
     * @param vlmax 元素总数
     * @return 执行结果
     */
    int vrgather_vi(Type *vs2, uint32_t imm, Type *vd, uint32_t vm, 
         TypeMask *v0, int vl, uint32_t vlmax)
    {
        return vrgather_vx(vs2, imm, vd, vm, v0, vl, vlmax);
    }
    
    /**
     * vcompress_vm()  
     *
     * @param vs2 源操作向量基地址
     * @param vs1 索引值列表
     * @param vd 目的向量存放地址
     * @param vl 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vcompress_vm(Type *vs2, Type2 *vs1, Type *vd, int vl)
    {
        VrgatherVecMap vector_vs2(vs2, vl);
        VIndexVecMap vector_vs1(vs1, vl);
        VrgatherVecMap vector_vd(vd, vl);
        uint32_t number = 0;

        for (int i = 0; i < vl; i++) {
            if (vector_vs1(i) & 0x1) {
                vector_vd(number) = vector_vs2(i);
                number++;
            }
        }
        
        DBG_INFO5(debug, vector_vs2, vector_vs1, vector_vd, number, vl);
        return 0;
    }
};

/**
 * @brief 向量浮点分类(Classify)指令
 *
 * Classify 指令检查源操作数中的每个浮点元素,并按照其值的不同将其分为 10 类。
 *
 */
template <typename Type, typename MaskType>
class Vfclass
{
  public:
    int debug;

    Vfclass(): debug(GLOBAL_DBG)
    {

    }

    typedef Map<Matrix<half, 1, Dynamic>> VfclassVecMap;
    typedef Map<Matrix<Type, 1, Dynamic>> VfclassDstVecMap;
    typedef Map<Matrix<MaskType, 1, Dynamic>> VfclassMaskVecMap;

    /**
     * vfclass_v() vfclass.v
     * @param vs2 源操作向量基地址
     * @param vd 目的向量基地址
     * @param vm 不可屏蔽标识， vm=0 可屏蔽， vm=1不可屏蔽
     * @param v0 mask向量基地址
     * @param vl 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vfclass_v(half *vs2, Type *vd, int vm, MaskType *v0, int vl)
    {
        VfclassVecMap vector_vs2(vs2, vl);
        VfclassDstVecMap vector_vd(vd, vl);
        VfclassMaskVecMap vector_v0(v0, vl);
        uint32_t bit = 0;

        for (int i = 0; i < vl; i++) 
        {
            if (vm || (vector_v0(i) & 0x1))
            {
                if (isinf(vector_vs2(i)))                     //isinf 无穷大
                    bit = vector_vs2(i).x & 0x8000 ? 0 : 7;
                else if (isnan(vector_vs2(i)))                //isnan
                    bit = vector_vs2(i).x & 0x200 ? 9 : 8;
                else if (0 == (vector_vs2(i).x & 0x7fff))     //+-0
                    bit = vector_vs2(i).x & 0x8000 ? 3 : 4;
                else if (!(vector_vs2(i).x & 0x7c00) && (vector_vs2(i).x & 0x3ff))
                    bit = vector_vs2(i).x & 0x8000 ? 2 : 5;   //非规格化数
                else
                    bit = vector_vs2(i).x & 0x8000 ? 1 : 6;   //规格化数

                vector_vd(i) = 1 << bit;
            }
        }

        DBG_INFO4(debug, vector_vs2, vector_vd, vector_v0, vm);
        return 0;
    }
};

/**
 * @brief 单宽度向量除法指令
 *
 * 目的元素的宽度和源操作数中的元素宽度保持一致， 可以通过Type指定数据类型
 *
 */
template <typename Type, typename MaskType>
class Vdiv
{
  public:
    int debug;

    Vdiv(): debug(GLOBAL_DBG)
    {
    }

    typedef Map<Matrix<Type, 1, Dynamic>> VdivVecMap;
    typedef Map<Matrix<MaskType, 1, Dynamic>> VdivMaskVecMap;

    /**
     * vdiv_vf() vfdiv.vf
     * @param vs2 源操作向量基地址
     * @param rs1 源标量操作数
     * @param vd 目的向量基地址
     * @param vm 不可屏蔽标识， vm=0 可屏蔽， vm=1不可屏蔽
     * @param v0 mask向量基地址
     * @param vl 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vdiv_vf(Type *vs2, Type rs1, Type *vd, int vm, MaskType *v0, int vl)
    {
        VdivVecMap vector_vs2(vs2, vl);
        VdivVecMap vector_vd(vd, vl);
        VdivMaskVecMap vector_v0(v0, vl);

        if (!vm) {
            for (int i = 0; i < vl; i++) {
                if (vector_v0(i) & 0x1)
                    vector_vd(i) = vector_vs2(i) / rs1;
            }
        } else
            vector_vd = vector_vs2.array() / rs1;

        DBG_VECTOR_VF;

        return 0;
    }

    /**
     * vrdiv_vf() vfrdiv.vf
     * @param vs2 源操作向量基地址
     * @param rs1 源标量操作数
     * @param vd 目的向量基地址
     * @param vm 不可屏蔽标识， vm=0 可屏蔽， vm=1不可屏蔽
     * @param v0 mask向量基地址
     * @param vl 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vrdiv_vf(Type *vs2, Type rs1, Type *vd, int vm, MaskType *v0, int vl)
    {
        VdivVecMap vector_vs2(vs2, vl);
        VdivVecMap vector_vd(vd, vl);
        VdivMaskVecMap vector_v0(v0, vl);

        if (!vm) {
            for (int i = 0; i < vl; i++) {
                if (vector_v0(i) & 0x1)
                    vector_vd(i) = rs1 / vector_vs2(i);
            }
        } else
            vector_vd = rs1 / vector_vs2.array();

        DBG_VECTOR_VF;

        return 0;
    }

    /**
     * vdiv_vv() vfdiv.vv
     * @param vs2 源操作向量二基地址
     * @param vs1 源操作向量一基地址
     * @param vd 目的向量基地址
     * @param vm 不可屏蔽标识， vm=0 可屏蔽， vm=1不可屏蔽
     * @param v0 mask向量基地址
     * @param vl 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vdiv_vv(Type *vs2, Type *vs1, Type *vd, int vm, MaskType *v0, int vl)
    {
        VdivVecMap vector_vs2(vs2, vl);
        VdivVecMap vector_vs1(vs1, vl);
        VdivVecMap vector_vd(vd, vl);
        VdivMaskVecMap vector_v0(v0, vl);

        if (!vm) {
            for (int i = 0; i < vl; i++) {
                if (vector_v0(i) & 0x1)
                    vector_vd(i) = vector_vs2(i) / vector_vs1(i);
            }
        } else
            vector_vd = vector_vs2.array() / vector_vs1.array();

        DBG_VECTOR_VV;

        return 0;
    }
};

/**
 * @brief 向量平方根运算
 *
 * 一元向量-向量指令，用于求向量元素的平方根, 即：x^0.5，其中x为向量元素.
 * 
 *
 */
template <typename Type, typename MaskType>
class Vfsqrt
{
  public:
    int debug;

    Vfsqrt(): debug(GLOBAL_DBG)
    {

    }

    typedef Map<Matrix<Type, 1, Dynamic>> VfsqrtVecMap;
    typedef Map<Matrix<MaskType, 1, Dynamic>> VfsqrtMaskVecMap;

    /**
     * vfsqrt_v()  vfsqrt.v vd, vs2, vm
     *
     * @param vs2 源操作向量基地址
     * @param vd 目的向量存放地址
     * @param vm mask 使能标记，0使能
     * @param mask 元素mask标记，最低位有效
     * @param vl 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vfsqrt_v(Type *vs2, Type *vd, int vm, MaskType *mask, int vl)
    {
        int number = 0;
        VfsqrtVecMap vector_vs2(vs2, vl);
        VfsqrtVecMap vector_vd(vd, vl);
        VfsqrtMaskVecMap vector_mask(mask, vl);
        
        for (int i=0; i < vl; i++)
            if (vm || (vector_mask(i) & 0x1)) {
                vector_vd(i) = sqrt(vector_vs2(i));
            }

        DBG_INFO5(debug, vector_vs2, vector_vd, vector_mask, number, vl);
        return 0;
    }
};

/**
 * @brief 向量指数运算
 *
 * 一元向量-向量指令，用于求向量元素指数运算, 即：e^x，其中x为向量元素.
 * 
 *
 */
template <typename Type, typename MaskType>
class Vfexp
{
  public:
    int debug;

    Vfexp(): debug(GLOBAL_DBG)
    {

    }

    typedef Map<Matrix<Type, 1, Dynamic>> VfexpVecMap;
    typedef Map<Matrix<MaskType, 1, Dynamic>> VfexpMaskVecMap;

    /**
     * vfexp_v()  vfexp.v vd, vs2, vm
     *
     * @param vs2 源操作向量基地址
     * @param vd 目的向量存放地址
     * @param vm mask 使能标记，0使能
     * @param mask 元素mask标记，最低位有效
     * @param vl 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vfexp_v(Type *vs2, Type *vd, int vm, MaskType *mask, int vl)
    {
        int number = 0;
        VfexpVecMap vector_vs2(vs2, vl);
        VfexpVecMap vector_vd(vd, vl);
        VfexpMaskVecMap vector_mask(mask, vl);
        
        for (int i=0; i < vl; i++)
            if (vm || (vector_mask(i) & 0x1)) {
                vector_vd(i) = exp(vector_vs2(i));
            }

        DBG_INFO5(debug, vector_vs2, vector_vd, vector_mask, number, vl);
        return 0;
    }
};

#endif


