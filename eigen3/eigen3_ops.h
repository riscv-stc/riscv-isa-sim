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
#include <type_traits>
#include <queue>

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
        dtype odd_acc = dtype(-0); \
        dtype even_acc = dtype(-0); \
        \
        for (int _row = 0; _row < row; _row++) { \
            if ((_row % 2) == 1) {\
                odd_acc = src(_row, _col) + odd_acc; \
            } else {\
                even_acc = src(_row, _col) + even_acc; \
            } \
        } \
        dest(0, _col) = odd_acc + even_acc; \
    } \
} while(0);

#define MATRIX_ACC_DIMH_4PART(src, dest, dtype, row, column) do { \
    for (int _col = 0; _col < column; _col++) { \
        dtype acc0 = dtype(-0); \
        dtype acc1 = dtype(-0); \
        dtype acc2 = dtype(-0); \
        dtype acc3 = dtype(-0); \
        int new_row = row % 2  == 0 ? row :  row + 1; \
        for (int _row = 0; _row < new_row; _row++) { \
            if (_row < row) { \
                if ((_row % 4) == 0) {\
                    acc0 += src(_row, _col); \
                } \
                if ((_row % 4) == 1) {\
                    acc1 += src(_row, _col); \
                } \
                if ((_row % 4) == 2) {\
                    acc2 += src(_row, _col); \
                } \
                if ((_row % 4) == 3) {\
                    acc3 += src(_row, _col); \
                } \
            } else { \
                if ((_row % 4) == 0) {\
                    acc0 += dtype(0); \
                } \
                if ((_row % 4) == 1) {\
                    acc1 += dtype(0); \
                } \
                if ((_row % 4) == 2) {\
                    acc2 += dtype(0); \
                } \
                if ((_row % 4) == 3) {\
                    acc3 += dtype(0); \
                } \
            } \
        } \
        acc2 += acc0; \
        acc3 += acc1; \
        dest(0, _col) = acc2 + acc3; \
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

#define MATRIX_ACC_DIMW(src, dest, dtype, row, column) do { \
    uint8_t once_max = 128; \
    queue<dtype> dat_q; \
    queue<dtype> dat_q_128;\
    queue<dtype> acc_dat_q; \
    queue<dtype> acc_dat_q_tmp; \
    queue<dtype> acc_last; \
    dtype mul_ret, a, b, acc_ab, acc_current, new_last, old_last; \
    if(is_same< dtype, Float32 >::value) {\
        once_max = 64; \
    } \
    \
    for (int _row = 0; _row < row; _row++) { \
        for (int _col = 0; _col < column; _col++) { \
            mul_ret = src(_row, _col); \
            dat_q.push(mul_ret); \
        } \
        while (dat_q.size() != 0) { \
            dat_q_128 = queue<dtype>(); \
            for(int i = 0; i < once_max; i++) { \
                if(dat_q.size() != 0) { \
                    dat_q_128.push(dat_q.front()); \
                    dat_q.pop(); \
                } \
            } \
            \
            acc_dat_q = queue<dtype>(); \
            int end_flag = 0; \
            while(end_flag == 0) { \
                acc_dat_q = queue<dtype>(); \
                while(dat_q_128.size() >=2) { \
                    a = dat_q_128.front(); \
                    dat_q_128.pop(); \
                    b = dat_q_128.front(); \
                    dat_q_128.pop(); \
                    acc_ab = a + b; \
                    acc_dat_q.push(acc_ab); \
                } \
                if(dat_q_128.size() ==1) {\
                    acc_dat_q.push(dat_q_128.front()); \
                    dat_q_128.pop(); \
                } \
                int q_size = acc_dat_q.size(); \
                if (q_size == 1) { \
                    end_flag = 1;\
                } else { \
                    for(auto i = 0; i < q_size; i++) {\
                        dat_q_128.push(acc_dat_q.front()); \
                        acc_dat_q.pop(); \
                    } \
                } \
            } \
            acc_current = acc_dat_q.front(); \
            acc_dat_q.pop(); \
            if (acc_last.size() !=0) { \
                old_last = acc_last.front(); \
                acc_last.pop(); \
                new_last = acc_current + old_last; \
                acc_last.push(new_last); \
            } else \
                acc_last.push(acc_current); \
        } \
        \
        dest(_row, 0) = acc_last.front(); \
        acc_last.pop(); \
    } \
} while(0);

#define PADDING_3D_HW_C(src, dest, padding, dest_row_2d, dest_row_3d, dest_column_3d, u, d, l, r) do { \
    unsigned int src_row = 0; \
    for (int _row = 0; _row < dest_row_2d; _row++) { \
        if ((_row < (dest_column_3d * u)) || \
                ((_row % dest_column_3d) < l) || \
                (_row % dest_column_3d >= (dest_column_3d - r)) || \
                (_row >= (dest_column_3d * (dest_row_3d - d)))) { \
            dest.row(_row) = padding.row(0); \
        } else { \
            dest.row(_row) = src.row(src_row); \
            src_row++; \
        } \
    } \
} while(0)

#define LINE_WINDOWS_COMMON_LENGTH(start, kh, pt, pb, hin, cnt) do { \
    if (start + kh < pt) \
        cnt = 0; \
    else if (start < pt && start + kh < pt + hin) \
        cnt = kh - (pt - start); \
    else if (start < pt && start + kh >= pt + hin) \
        cnt = hin; \
    else if (start >= pt && start < pt + hin && start + kh < pt + hin) \
        cnt = kh; \
    else if (start >= pt && start < pt + hin && start + kh >= pt + hin) \
        cnt = pt + hin - start ; \
    else \
        cnt = 0; \
} while(0);

// threshhold typed float32_t
#define MATRIX_RELU_THRESHHOLD(dest, src, row, column, dtype, threshhold) do { \
    Float32 f32th; \
    f32th.x = threshhold; \
    for (int _row = 0; _row < row; _row++) { \
        for (int _col = 0; _col < column; _col++) { \
            if (src(_row, _col) <= dtype(0)) {\
                if(is_same< dtype, Float32 >::value) {\
                    dest(_row, _col).x = 0x00000000; \
                } else {\
                    dest(_row, _col).x = 0x0000; \
                } \
            \
            } else if ((f32th != Float32(0)) && (src(_row, _col) > dtype(f32th))) \
                dest(_row, _col) = dtype(f32th); \
            else \
                dest(_row, _col) = src(_row, _col); \
            \
            if(is_same< dtype, half >::value) { \
                if ((dest(_row, _col).x & 0x3ff) && ((dest(_row, _col).x & 0x7c00) == 0x7c00)) \
                    dest(_row, _col).x = 0xfe00; \
            } else if(is_same< dtype, Bfloat16 >::value) { \
                if ((dest(_row, _col).x & 0x7f) && ((dest(_row, _col).x & 0x7f80) == 0x7f80)) \
                    dest(_row, _col).x = 0xffc0; \
                if ((dest(_row, _col).x & 0xffff) == 0x8000) \
                    dest(_row, _col).x = 0x0000; \
            } else if(is_same< dtype, Float32 >::value) {\
                if ((dest(_row, _col).x & 0x7fffff) && ((dest(_row, _col).x & 0x7f800000) == 0x7f800000)) \
                    dest(_row, _col).x = 0xffc00000; \
            } \
        } \
    } \
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

const uint16_t recip_table_Bfloat16[65] = {
    0x0000, 0x3f80, 0x3f00, 0x3eaa, 0x3e80, 0x3e4c,
    0x3e2a, 0x3e12, 0x3e00, 0x3de3, 0x3dcc, 0x3dba,
    0x3daa, 0x3d9d, 0x3d92, 0x3d88, 0x3d80, 0x3d70,
    0x3d63, 0x3d57, 0x3d4c, 0x3d43, 0x3d3a, 0x3d32,
    0x3d2a, 0x3d23, 0x3d1d, 0x3d17, 0x3d12, 0x3d0d,
    0x3d08, 0x3d04, 0x3d00, 0x3cf8, 0x3cf0, 0x3cea,
    0x3ce3, 0x3cdd, 0x3cd7, 0x3cd2, 0x3ccc, 0x3cc7,
    0x3cc3, 0x3cbe, 0x3cba, 0x3cb6, 0x3cb2, 0x3cae,
    0x3caa, 0x3ca7, 0x3ca3, 0x3ca0, 0x3c9d, 0x3c9a,
    0x3c97, 0x3c94, 0x3c92, 0x3c8f, 0x3c8d, 0x3c8a,
    0x3c88, 0x3c86, 0x3c84, 0x3c82, 0x3c80
};

const uint32_t recip_table_Float32[65] = {
    0x00000000, 0x3f800000, 0x3f000000, 0x3eaaaaab, 0x3e800000, 0x3e4ccccd,
    0x3e2aaaab, 0x3e124925, 0x3e000000, 0x3de38e39, 0x3dcccccd, 0x3dba2e8c,
    0x3daaaaab, 0x3d9d89d9, 0x3d924925, 0x3d888889, 0x3d800000, 0x3d70f0f1,
    0x3d638e39, 0x3d579436, 0x3d4ccccd, 0x3d430c31, 0x3d3a2e8c, 0x3d321643,
    0x3d2aaaab, 0x3d23d70a, 0x3d1d89d9, 0x3d17b426, 0x3d124925, 0x3d0d3dcb,
    0x3d088889, 0x3d042108, 0x3d000000, 0x3cf83e10, 0x3cf0f0f1, 0x3cea0ea1,
    0x3ce38e39, 0x3cdd67c9, 0x3cd79436, 0x3cd20d21, 0x3ccccccd, 0x3cc7ce0c,
    0x3cc30c31, 0x3cbe82fa, 0x3cba2e8c, 0x3cb60b61, 0x3cb21643, 0x3cae4c41,
    0x3caaaaab, 0x3ca72f05, 0x3ca3d70a, 0x3ca0a0a1, 0x3c9d89d9, 0x3c9a90e8,
    0x3c97b426, 0x3c94f209, 0x3c924925, 0x3c8fb824, 0x3c8d3dcb, 0x3c8ad8f3,
    0x3c888889, 0x3c864b8a, 0x3c842108, 0x3c820821, 0x3c800000
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

    unsigned int relu_threshhold;
};

struct DmaeShapeStride
{
    unsigned short shape_x;
    unsigned short shape_y;
    unsigned short shape_z;

    unsigned int stride_s_x;
    unsigned int stride_s_y;

    unsigned int stride_d_x;
    unsigned int stride_d_y;

};

struct VmeShapeStride
{
    int row, column;
    int ifm_c_stride, cin;
    int wout, hout;
    int ofm_c_stride, k_c_stride;
    int kw, kh, sw, sh;
    int n_pad_u, n_pad_d, n_pad_l, n_pad_r;
    int k_dilation_w, k_dilation_h;
    unsigned int relu_threshhold;
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
    unsigned int conv_cin;

    /* CSR  FM out*/
    unsigned int conv_fm_out;
    unsigned int conv_cout;

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
int veadd_mm(DType* rs1, DType* rd, DType* rs2, struct ShapeStride *ss, bool relu)
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

    if (relu) {
        MATRIX_RELU_THRESHHOLD(rd_matrix, rd_matrix, ss->shape1_row, ss->shape1_column, DType, ss->relu_threshhold);
    }

    if (GLOBAL_DBG)
        cout << "rd:" << endl << rd_matrix << endl;

    return 0;
}

template <typename DType>
int veadd_mv(DType *rs1, DType *rd, DType *rs2, struct ShapeStride *ss, int dim, bool relu)
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

    if (relu) {
        MATRIX_RELU_THRESHHOLD(rd_matrix, rd_matrix, ss->shape1_row, ss->shape1_column, DType, ss->relu_threshhold);
    }

    return 0;
}

template <typename DType>
int veadd_mf(DType *rs1, DType *rd, DType rs2, struct ShapeStride *ss, bool relu)
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

    if (relu) {
        MATRIX_RELU_THRESHHOLD(rd_matrix, rd_matrix, ss->shape1_row, ss->shape1_column, DType, ss->relu_threshhold);
    }

    if (GLOBAL_DBG)
        cout << "rd:" << endl << rd_matrix << endl;

    return 0;
}

template <typename DType>
int vesub_mm(DType *rs1, DType *rd, DType *rs2, struct ShapeStride *ss, bool relu)
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

    if (relu) {
        MATRIX_RELU_THRESHHOLD(rd_matrix, rd_matrix, ss->shape1_row, ss->shape1_column, DType, ss->relu_threshhold);
    }

    if (GLOBAL_DBG)
        cout << "rd:" << endl << rd_matrix << endl;

    return 0;
}

template <typename DType>
int vesub_mv(DType *rs1, DType *rd, DType *rs2, struct ShapeStride *ss, int dim, bool relu)
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
            rd_matrix.row(row) = rs1_matrix.row(row).array() - vector_dim0.array();
        break;
    case 1:
        if (GLOBAL_DBG) {
            SHAPE_STRIDE_INFO(ss);
            cout << "rs1:" << endl << rs1_matrix << endl;
            cout << "rs2:" << endl << vector_dim1 << endl;
        }

        for (int col = 0; col < rs1_matrix.cols(); col++)
            rd_matrix.col(col) = rs1_matrix.col(col).array() - vector_dim1.array();
        break;
    default:
        cout << __FUNCTION__ << "error dim" << endl;
        return -BR_EPARAM;
    }

    if (relu) {
        MATRIX_RELU_THRESHHOLD(rd_matrix, rd_matrix, ss->shape1_row, ss->shape1_column, DType, ss->relu_threshhold);
    }

    if (GLOBAL_DBG)
        cout << "rd:" << endl << rd_matrix << endl;
    return 0;
}

template <typename DType>
int versub_mv(DType *rs1, DType *rd, DType *rs2, struct ShapeStride *ss, int dim)
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
            for (int column = 0; column < rs1_matrix.cols(); column++) {
                if (isnan(rs1_matrix(row, column)) && isnan(vector_dim0(0, column)))
                    rd_matrix(row, column) = rs1_matrix(row, column);
                else
                    rd_matrix(row, column) = vector_dim0(0, column) - rs1_matrix(row, column);
            }
        break;
    case 1:
        if (GLOBAL_DBG) {
            SHAPE_STRIDE_INFO(ss);
            cout << "rs1:" << endl << rs1_matrix << endl;
            cout << "rs2:" << endl << vector_dim1 << endl;
        }

        
        for (int row = 0; row < rs1_matrix.rows(); row++)
            for (int column = 0; column < rs1_matrix.cols(); column++) {
                if (isnan(rs1_matrix(row, column)) && isnan(vector_dim1(row, 0)))
                    rd_matrix(row, column) = rs1_matrix(row, column);
                else
                    rd_matrix(row, column) = vector_dim1(row, 0) - rs1_matrix(row, column);
            }
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
int vesub_mf(DType *rs1, DType *rd, DType rs2, struct ShapeStride *ss, bool relu)
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

    if (relu) {
        MATRIX_RELU_THRESHHOLD(rd_matrix, rd_matrix, ss->shape1_row, ss->shape1_column, DType, ss->relu_threshhold);
    }

    if (GLOBAL_DBG)
        cout << "rd:" << endl << rd_matrix << endl;

    return 0;
}

template <typename DType>
int versub_mf(DType *rs1, DType *rd, DType rs2, struct ShapeStride *ss)
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
    const_matrix = const_matrix.Constant(ss->shape1_row, ss->shape1_column, rs2);

    for (int row = 0; row < rs1_matrix.rows(); row++)
        for (int column = 0; column < rs1_matrix.cols(); column++) {
            if (isnan(rs1_matrix(row, column)) && isnan(const_matrix(row, column)))
                rd_matrix(row, column) = rs1_matrix(row, column);
            else
                rd_matrix(row, column) =  const_matrix(row, column) - rs1_matrix(row, column);
        }

    if (GLOBAL_DBG)
        cout << "rd:" << endl << rd_matrix << endl;

    return 0;
}

template <typename DType>
int verev_m(DType *rs1, DType *rd, struct ShapeStride *ss, int dim)
{
    DEFINE_MAP_DTYPE(DType)

    Map_DType rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_DType rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    switch (dim) {
    case 0:
        if (GLOBAL_DBG) {
            SHAPE_STRIDE_INFO(ss);
            cout << "rs1:" << endl << rs1_matrix << endl;
        }

        for (int col = 0; col < ss->shape1_column; col++)
            for (int row = ss->shape1_row -1, dst_row = 0; row >= 0; row--, dst_row++)
                rd_matrix(dst_row, col) = rs1_matrix(row, col);
        break;
    case 1:
        if (GLOBAL_DBG) {
            SHAPE_STRIDE_INFO(ss);
            cout << "rs1:" << endl << rs1_matrix << endl;
        }

        for (int row = 0; row < ss->shape1_row; row++)
             for (int col = ss->shape1_column - 1, dst_col = 0; col >= 0; col--, dst_col++) {
                rd_matrix(row, dst_col) = rs1_matrix(row, col);
             }
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
int verot180_m(DType *rs1, DType *rd, struct ShapeStride *ss)
{
    DEFINE_MAP_DTYPE(DType)

    Map_DType rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_DType rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    uint8_t esise = 0;
    if(is_same< DType, Float32 >::value)
        esise = sizeof(uint32_t);
    else
        esise = sizeof(uint16_t);

    DType *rd_tmp_buf = (DType *)malloc(ss->shape1_column * ss->shape1_row * esise);
    Map_DType rd_h_rev(rd_tmp_buf, ss->shape1_row, ss->shape1_column, DynStride(ss->shape1_column, 1));

    if (GLOBAL_DBG) {
        SHAPE_STRIDE_INFO(ss);
        cout << "rs1:" << endl << rs1_matrix << endl;
    }

    for (int col = 0; col < ss->shape1_column; col++)
        for (int row = ss->shape1_row -1, dst_row = 0; row >= 0; row--, dst_row++)
            rd_h_rev(dst_row, col) = rs1_matrix(row, col);

    for (int row = 0; row < ss->shape1_row; row++)
        for (int col = ss->shape1_column - 1, dst_col = 0; col >= 0; col--, dst_col++)
            rd_matrix(row, dst_col) = rd_h_rev(row, col);

    if (GLOBAL_DBG)
        cout << "rd:" << endl << rd_matrix << endl;
    
    free(rd_tmp_buf);
    return 0;
}

template <typename DType>
int vediv_mm(DType* rs1, DType* rd, DType* rs2, struct ShapeStride *ss, bool relu)
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

    if(is_same< DType, half >::value)
    {
        float16_t rs1_f16, rs2_f16, rd_f16;

        for (int row = 0; row < rs1_matrix.rows(); row ++)
            for (int col = 0; col < rs1_matrix.cols(); col ++) {
                rs1_f16.v = rs1_matrix(row, col).x;
                rs2_f16.v = rs2_matrix(row, col).x;
                rd_f16 = f16_div_(rs2_f16, rs1_f16);
                rd_matrix(row, col).x =  rd_f16.v;
            }
    }
    else if(is_same< DType, Bfloat16 >::value)
    {
        bfloat16_t rs1_bf16, rs2_bf16, rd_bf16;

        for (int row = 0; row < rs1_matrix.rows(); row ++)
            for (int col = 0; col < rs1_matrix.cols(); col ++) {
                rs1_bf16.v = rs1_matrix(row, col).x;
                rs2_bf16.v = rs2_matrix(row, col).x;
                rd_bf16 = bf16_div_(rs2_bf16, rs1_bf16);
                rd_matrix(row, col).x =  rd_bf16.v;
            }        
    }
    else if(is_same< DType, Float32 >::value)
    {
        float32_t rs1_f32, rs2_f32, rd_f32;

        for (int row = 0; row < rs1_matrix.rows(); row ++)
            for (int col = 0; col < rs1_matrix.cols(); col ++) {
                rs1_f32.v = rs1_matrix(row, col).x;
                rs2_f32.v = rs2_matrix(row, col).x;
                rd_f32 = f32_div_(rs2_f32, rs1_f32);
                rd_matrix(row, col).x =  rd_f32.v;
            }         
    }

    if (relu) {
        MATRIX_RELU_THRESHHOLD(rd_matrix, rd_matrix, ss->shape1_row, ss->shape1_column, DType, ss->relu_threshhold);
    }

    if (GLOBAL_DBG)
        cout << "rd:" << endl << rd_matrix << endl;

    return 0;
}

template <typename DType>
int vediv_mv(DType *rs1, DType *rd, DType *rs2, struct ShapeStride *ss, int dim, bool relu)
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

        if(is_same< DType, half >::value)
        {
            float16_t rs1_f16, rs2_f16, rd_f16;

            for (int row = 0; row < rs1_matrix.rows(); row ++)
                for (int col = 0; col < rs1_matrix.cols(); col ++) {
                    rs1_f16.v = rs1_matrix(row, col).x;
                    rs2_f16.v = vector_dim0(0, col).x;
                    rd_f16 = f16_div_(rs2_f16, rs1_f16);
                    rd_matrix(row, col).x =  rd_f16.v;
                }
        }
        else if(is_same< DType, Bfloat16 >::value)
        {
            bfloat16_t rs1_bf16, rs2_bf16, rd_bf16;
       
            for (int row = 0; row < rs1_matrix.rows(); row ++)
                for (int col = 0; col < rs1_matrix.cols(); col ++) {
                    rs1_bf16.v = rs1_matrix(row, col).x;
                    rs2_bf16.v = vector_dim0(0, col).x;
                    rd_bf16 = bf16_div_(rs2_bf16, rs1_bf16);
                    rd_matrix(row, col).x =  rd_bf16.v;
                }        
        }
        else if(is_same< DType, Float32 >::value)
        {
            float32_t rs1_f32, rs2_f32, rd_f32;

            for (int row = 0; row < rs1_matrix.rows(); row ++)
                for (int col = 0; col < rs1_matrix.cols(); col ++) {
                    rs1_f32.v = rs1_matrix(row, col).x;
                    rs2_f32.v = vector_dim0(0, col).x;
                    rd_f32 = f32_div_(rs2_f32, rs1_f32);
                    rd_matrix(row, col).x =  rd_f32.v;
                }          
        }

        if (GLOBAL_DBG)
            cout << "rd:" << endl << rd_matrix << endl;
        break;
    case 1:
        if (GLOBAL_DBG) {
            SHAPE_STRIDE_INFO(ss);
            cout << "rs1:" << endl << rs1_matrix << endl;
            cout << "rs2:" << endl << vector_dim1 << endl;
        }

        if(is_same< DType, half >::value)
        {
            float16_t rs1_f16, rs2_f16, rd_f16;

            for (int col = 0; col < rs1_matrix.cols(); col++)
                for (int row = 0; row < rs1_matrix.rows(); row ++) {
                    rs1_f16.v = rs1_matrix(row, col).x;
                    rs2_f16.v = vector_dim1(row, 0).x;
                    rd_f16 = f16_div_(rs2_f16, rs1_f16);
                    rd_matrix(row, col).x =  rd_f16.v;
                }
        }
        else if(is_same< DType, Bfloat16 >::value)
        {
            bfloat16_t rs1_bf16, rs2_bf16, rd_bf16;
       
            for (int col = 0; col < rs1_matrix.cols(); col++)
                for (int row = 0; row < rs1_matrix.rows(); row ++) {
                    rs1_bf16.v = rs1_matrix(row, col).x;
                    rs2_bf16.v = vector_dim1(row, 0).x;
                    rd_bf16 = bf16_div_(rs2_bf16, rs1_bf16);
                    rd_matrix(row, col).x =  rd_bf16.v;
                }        
        }
        else if(is_same< DType, Float32 >::value)
        {
            float32_t rs1_f32, rs2_f32, rd_f32;

            for (int col = 0; col < rs1_matrix.cols(); col++)
                for (int row = 0; row < rs1_matrix.rows(); row ++) {
                    rs1_f32.v = rs1_matrix(row, col).x;
                    rs2_f32.v = vector_dim1(row, 0).x;
                    rd_f32 = f32_div_(rs2_f32, rs1_f32);
                    rd_matrix(row, col).x =  rd_f32.v;
                }          
        }

        if (GLOBAL_DBG)
            cout << "rd:" << endl << rd_matrix << endl;
        break;
    default:
        cout << __FUNCTION__ << " error dim" << endl;
        return -BR_EPARAM;
    }

    if (relu) {
        MATRIX_RELU_THRESHHOLD(rd_matrix, rd_matrix, ss->shape1_row, ss->shape1_column, DType, ss->relu_threshhold);
    }

    return 0;
}

template <typename DType>
int vediv_mf(DType *rs1, DType *rd, DType rs2, struct ShapeStride *ss, bool relu)
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

    if(is_same< DType, half >::value)
    {
        float16_t rs1_f16, rs2_f16, rd_f16;
        rs2_f16.v = rs2.x;
        for (int row = 0; row < rs1_matrix.rows(); row ++)
            for (int col = 0; col < rs1_matrix.cols(); col ++) {
                rs1_f16.v = rs1_matrix(row, col).x;
                rd_f16 = f16_div_(rs2_f16, rs1_f16);
                rd_matrix(row, col).x =  rd_f16.v;
            }
    }
    else if(is_same< DType, Bfloat16 >::value)
    {
        bfloat16_t rs1_bf16, rs2_bf16, rd_bf16;
        rs2_bf16.v = rs2.x;        
        for (int row = 0; row < rs1_matrix.rows(); row ++)
            for (int col = 0; col < rs1_matrix.cols(); col ++) {
                rs1_bf16.v = rs1_matrix(row, col).x;
                rd_bf16 = bf16_div_(rs2_bf16, rs1_bf16);
                rd_matrix(row, col).x =  rd_bf16.v;
            }        
    }
    else if(is_same< DType, Float32 >::value)
    {
        float32_t rs1_f32, rs2_f32, rd_f32;
        rs2_f32.v = rs2.x;
        for (int row = 0; row < rs1_matrix.rows(); row ++)
            for (int col = 0; col < rs1_matrix.cols(); col ++) {
                rs1_f32.v = rs1_matrix(row, col).x;
                rd_f32 = f32_div_(rs2_f32, rs1_f32);
                rd_matrix(row, col).x =  rd_f32.v;
            }          
    }

    if (relu) {
        MATRIX_RELU_THRESHHOLD(rd_matrix, rd_matrix, ss->shape1_row, ss->shape1_column, DType, ss->relu_threshhold);
    }

    if (GLOBAL_DBG)
        cout << "rd:" << endl << rd_matrix << endl;

    return 0;
}

template <typename OutDType, typename InDType>
int veacc_m(OutDType *rs1, OutDType *rd, struct ShapeStride *ss, int dim, bool relu)
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
        uint32_t MAX_COLUMN;
        if(is_same< OutDType, Float32 >::value)
            MAX_COLUMN = 32;
        else
            MAX_COLUMN = 64;

        if (ss->shape1_column <= MAX_COLUMN  && ss->shape1_row >= 2
            && ss->stride_rs1 == ss->shape1_column) {
            MATRIX_ACC_DIMH_4PART(rs1_matrix_inner, rd_col_sum_inner, InDType, ss->shape1_row, ss->shape1_column);
        } else {
            MATRIX_ACC_DIMH_PARITY(rs1_matrix_inner, rd_col_sum_inner, InDType, ss->shape1_row, ss->shape1_column);
        }

        if (GLOBAL_DBG)
            cout << "rdinner:\n" << rd_col_sum_inner << endl;
        //rd_col_sum = rd_col_sum_inner.cast<OutDType>();
        MATRIX_CAST(rd_col_sum_inner, rd_col_sum, OutDType, 1, ss->shape1_column);
        free(rd_col_buf);

        if (relu) {
            MATRIX_RELU_THRESHHOLD(rd_col_sum, rd_col_sum, 1, ss->shape1_column, OutDType, ss->relu_threshhold);
        }

        if (GLOBAL_DBG)
            cout << "rd:\n" << rd_col_sum << endl;
    } else {
        Map_OutDType rd_row_sum(rd, ss->shape1_row, 1, DynStride(ss->stride_rd, 1));
        InDType *rd_row_buf = (InDType *)malloc(ss->shape1_row * sizeof(InDType));
        Map_InDType rd_row_sum_inner(rd_row_buf, ss->shape1_row, 1, DynStride(1, 1));
        //rd_row_sum_inner = rs1_matrix_inner.rowwise().sum();
        MATRIX_ACC_DIMW(rs1_matrix_inner, rd_row_sum_inner, InDType, ss->shape1_row, ss->shape1_column);
        //rd_row_sum = rd_row_sum_inner.cast<OutDType>();
        MATRIX_CAST(rd_row_sum_inner, rd_row_sum, OutDType, ss->shape1_row, 1);
        free(rd_row_buf);

        if (relu) {
            MATRIX_RELU_THRESHHOLD(rd_row_sum, rd_row_sum, ss->shape1_row, 1, OutDType, ss->relu_threshhold);
        }

        if (GLOBAL_DBG)
            cout << "rd:\n" << rd_row_sum << endl;
    }

    free(rs1_buf);

    return 0;
}

template <typename OutDType, typename InDType>
int veacc_m(OutDType *rs1, OutDType *rd, struct ShapeStride *ss, bool relu)
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

    uint32_t MAX_COLUMN;
    if(is_same< OutDType, Float32 >::value)
        MAX_COLUMN = 32;
    else
        MAX_COLUMN = 64;

    //rd_col_sum = rs1_matrix_inner.colwise().sum();
    if (ss->shape1_column <= MAX_COLUMN  && ss->shape1_row >= 2
            && ss->stride_rs1 == ss->shape1_column) {
        MATRIX_ACC_DIMH_4PART(rs1_matrix_inner, rd_col_sum, InDType, ss->shape1_row, ss->shape1_column);
    } else {
        MATRIX_ACC_DIMH_PARITY(rs1_matrix_inner, rd_col_sum, InDType, ss->shape1_row, ss->shape1_column);
    }

    //InDType rd_tmp = rd_col_sum.sum();
    Matrix_InDType rd_acc(1, 1);
    MATRIX_ACC_DIMW(rd_col_sum, rd_acc, InDType, 1, ss->shape1_column);
    //*rd = OutDType(rd_tmp);

    if (relu) {
        MATRIX_RELU_THRESHHOLD(rd_acc, rd_acc, 1, 1, InDType, ss->relu_threshhold);
    }

    *rd = OutDType(rd_acc(0, 0));

    if (GLOBAL_DBG)
        cout << "rd:\n" << *rd << endl;

    free(pcol_sum);
    free(rs1_buf);

    return 0;
}

template <typename OutDType, typename InDType>
int veemacc_mm(OutDType *rs1, OutDType *rd, OutDType *rs2, struct ShapeStride *ss, int dim, bool relu)
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
    
    uint32_t MAX_COLUMN;
    if (is_same< OutDType, Float32 >::value) {
        MAX_COLUMN = 32;
    } else {
        MAX_COLUMN = 64;
    }

    if (dim == 0) {
        Map_OutDType rd_col_sum(rd, 1, ss->shape1_column, DynStride(1, 1));
        InDType *rd_col_buf = (InDType *)malloc(ss->shape1_column * sizeof(InDType));
        Map_InDType rd_col_sum_inner(rd_col_buf, 1, ss->shape1_column, DynStride(1, 1));

        if (ss->shape1_column <= MAX_COLUMN  && ss->shape1_row >= 2
            && ss->stride_rs1 == ss->shape1_column && ss->stride_rs2 == ss->shape1_column) {
            MATRIX_ACC_DIMH_4PART(mul_result, rd_col_sum_inner, InDType, ss->shape1_row, ss->shape1_column);
        } else {
            MATRIX_ACC_DIMH_PARITY(mul_result, rd_col_sum_inner, InDType, ss->shape1_row, ss->shape1_column);
        }

        if (GLOBAL_DBG)
            cout << "rdinner:\n" << rd_col_sum_inner << endl;

        MATRIX_CAST(rd_col_sum_inner, rd_col_sum, OutDType, 1, ss->shape1_column);
        free(rd_col_buf);

        if (relu) {
            MATRIX_RELU_THRESHHOLD(rd_col_sum, rd_col_sum, 1, ss->shape1_column, OutDType, ss->relu_threshhold);
        }

        if (GLOBAL_DBG)
            cout << "rd:\n" << rd_col_sum << endl;
    } else {
        Map_OutDType rd_row_sum(rd, ss->shape1_row, 1, DynStride(ss->stride_rd, 1));
        InDType *rd_row_buf = (InDType *)malloc(ss->shape1_row * sizeof(InDType));
        Map_InDType rd_row_sum_inner(rd_row_buf, ss->shape1_row, 1, DynStride(1, 1));

        MATRIX_ACC_DIMW(mul_result, rd_row_sum_inner, InDType, ss->shape1_row, ss->shape1_column);

        MATRIX_CAST(rd_row_sum_inner, rd_row_sum, OutDType, ss->shape1_row, 1);
        free(rd_row_buf);

        if (relu) {
            MATRIX_RELU_THRESHHOLD(rd_row_sum, rd_row_sum, ss->shape1_row, 1, OutDType, ss->relu_threshhold);
        }

        if (GLOBAL_DBG)
            cout << "rd:\n" << rd_row_sum << endl;
    }

    free(mul_buf);

    return 0;
}

template <typename OutDType, typename InDType>
int veemacc_mm(OutDType *rs1, OutDType *rd, OutDType *rs2, struct ShapeStride *ss, bool relu)
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

    uint32_t MAX_COLUMN;
    if (is_same< OutDType, Float32 >::value) {
        MAX_COLUMN = 32;
    } else {
        MAX_COLUMN = 64;
    }

    if (ss->shape1_column <= MAX_COLUMN  && ss->shape1_row >= 2
            && ss->stride_rs1 == ss->shape1_column && ss->stride_rs2 == ss->shape1_column) {
        MATRIX_ACC_DIMH_4PART(mul_result, rd_col_sum, InDType, ss->shape1_row, ss->shape1_column);
    } else {
        MATRIX_ACC_DIMH_PARITY(mul_result, rd_col_sum, InDType, ss->shape1_row, ss->shape1_column);
    }

    Matrix_InDType rd_acc(1, 1);
    MATRIX_ACC_DIMW(rd_col_sum, rd_acc, InDType, 1, ss->shape1_column);

    if (relu) {
        MATRIX_RELU_THRESHHOLD(rd_acc, rd_acc, 1, 1, InDType, ss->relu_threshhold);
    }

    *rd = OutDType(rd_acc(0, 0));

    if (GLOBAL_DBG)
        cout << "rd:\n" << *rd << endl;

    free(pcol_sum);
    free(mul_buf);

    return 0;
}

template <typename OutDType, typename InDType>
int veemacc_mv(OutDType *rs1, OutDType *rd, OutDType *rs2, struct ShapeStride *ss, int dim, bool relu)
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

    uint32_t MAX_COLUMN;
    if (is_same< OutDType, Float32 >::value) {
        MAX_COLUMN = 32;
    } else {
        MAX_COLUMN = 64;
    }

    if (0 == dim) {
        Map_OutDType vec_rs2_dim0(rs2, ss->shape1_row, 1, DynStride(1, 1));
        if (GLOBAL_DBG) {
            cout << "rs2:" << endl << vec_rs2_dim0 << endl;
        }

        MATRIX_MUL_VEC_V_CONVERT(rs1_matrix, vec_rs2_dim0, mul_result, ss->shape1_row, ss->shape1_column, InDType);

        InDType *rd_col_buf = (InDType *)malloc(ss->shape1_column * sizeof(InDType));
        Map_InDType rd_col_sum_inner(rd_col_buf, 1, ss->shape1_column, DynStride(1, 1));
        if (ss->shape1_column <= MAX_COLUMN  && ss->shape1_row >= 2 && ss->stride_rs1 == ss->shape1_column) {
            MATRIX_ACC_DIMH_4PART(mul_result, rd_col_sum_inner, InDType, ss->shape1_row, ss->shape1_column);
        } else {
            MATRIX_ACC_DIMH_PARITY(mul_result, rd_col_sum_inner, InDType, ss->shape1_row, ss->shape1_column);
        }

        Map_OutDType vec_rd_dim0(rd, 1, ss->shape1_column, DynStride(1, 1));
        MATRIX_CAST(rd_col_sum_inner, vec_rd_dim0, OutDType, 1, ss->shape1_column);
        free(rd_col_buf);

        if (relu) {
            MATRIX_RELU_THRESHHOLD(vec_rd_dim0, vec_rd_dim0, 1, ss->shape1_column, OutDType, ss->relu_threshhold);
        }

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
        //MATRIX_ACC_DIMW_PAIR(mul_result, rd_row_sum_inner, InDType, ss->shape1_row, ss->shape1_column);
        MATRIX_ACC_DIMW(mul_result, rd_row_sum_inner, InDType, ss->shape1_row, ss->shape1_column);
        Map_OutDType rd_row_sum(rd, ss->shape1_row, 1, DynStride(ss->stride_rd, 1));
        MATRIX_CAST(rd_row_sum_inner, rd_row_sum, OutDType, ss->shape1_row, 1);
        free(rd_row_buf);

        if (relu) {
            MATRIX_RELU_THRESHHOLD(rd_row_sum, rd_row_sum, ss->shape1_row, 1, OutDType, ss->relu_threshhold);
        }

        if (GLOBAL_DBG)
            cout << "rd:\n" << rd_row_sum << endl;
    }
    free(mul_buf);

    return 0;
}

template <typename OutDType, typename InDType>
int veemacc_mf(OutDType *rs1, OutDType *rd, OutDType rs2, struct ShapeStride *ss, int dim, bool relu)
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

    uint32_t MAX_COLUMN;
    if (is_same< OutDType, Float32 >::value) {
        MAX_COLUMN = 32;
    } else {
        MAX_COLUMN = 64;
    }

    if (dim == 0) {
        Map_OutDType rd_col_sum(rd, 1, ss->shape1_column, DynStride(1, 1));
        InDType *rd_col_buf = (InDType *)malloc(ss->shape1_column * sizeof(InDType));
        Map_InDType rd_col_sum_inner(rd_col_buf, 1, ss->shape1_column, DynStride(1, 1));

        if (ss->shape1_column <= MAX_COLUMN  && ss->shape1_row >= 2 && ss->stride_rs1 == ss->shape1_column) {
            MATRIX_ACC_DIMH_4PART(mul_result, rd_col_sum_inner, InDType, ss->shape1_row, ss->shape1_column);
        } else {
            MATRIX_ACC_DIMH_PARITY(mul_result, rd_col_sum_inner, InDType, ss->shape1_row, ss->shape1_column);
        }

        if (GLOBAL_DBG)
            cout << "rdinner:\n" << rd_col_sum_inner << endl;

        MATRIX_CAST(rd_col_sum_inner, rd_col_sum, OutDType, 1, ss->shape1_column);
        free(rd_col_buf);

        if (relu) {
            MATRIX_RELU_THRESHHOLD(rd_col_sum, rd_col_sum, 1, ss->shape1_column, OutDType, ss->relu_threshhold);
        }

        if (GLOBAL_DBG)
            cout << "rd:\n" << rd_col_sum << endl;
    } else {
        Map_OutDType rd_row_sum(rd, ss->shape1_row, 1, DynStride(ss->stride_rd, 1));
        InDType *rd_row_buf = (InDType *)malloc(ss->shape1_row * sizeof(InDType));
        Map_InDType rd_row_sum_inner(rd_row_buf, ss->shape1_row, 1, DynStride(1, 1));

        //MATRIX_ACC_DIMW_PAIR(mul_result, rd_row_sum_inner, InDType, ss->shape1_row, ss->shape1_column);
        MATRIX_ACC_DIMW(mul_result, rd_row_sum_inner, InDType, ss->shape1_row, ss->shape1_column);

        MATRIX_CAST(rd_row_sum_inner, rd_row_sum, OutDType, ss->shape1_row, 1);
        free(rd_row_buf);

        if (relu) {
            MATRIX_RELU_THRESHHOLD(rd_row_sum, rd_row_sum, ss->shape1_row, 1, OutDType, ss->relu_threshhold);
        }

        if (GLOBAL_DBG)
            cout << "rd:\n" << rd_row_sum << endl;
    }

    free(mul_buf);
    return 0;
}

// template <typename DType>
// int vemul_mm(DType *rs1, DType *rs2, DType *rd, struct ShapeStride *ss, bool relu)
// {
//     DEFINE_MAP_DTYPE(DType)
//     /* param check */
//     if (ss->shape1_column != ss->shape2_row) {
//         cout << __FUNCTION__ << ": shape1_column must equal shape2_row" << endl;
//         return -BR_EPARAM;
//     }

//     Map_DType rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
//     Map_DType rs2_matrix(rs2, ss->shape2_row, ss->shape2_column, DynStride(ss->stride_rs2, 1));
//     SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape2_column);
//     Map_DType rd_matrix(rd, ss->shape1_row, ss->shape2_column, DynStride(ss->stride_rd, 1));

//     if (GLOBAL_DBG) {
//         SHAPE_STRIDE_INFO(ss);
//         cout << "rs1:\n" << rs1_matrix << endl;
//         cout << "rs2:\n" << rs2_matrix << endl;
//     }

//     /* dot only support vector not support matrix, so we use '*' to do calculation */
//     rd_matrix = rs1_matrix * rs2_matrix;

//     if (relu) {
//         MATRIX_RELU_THRESHHOLD(rd_matrix, rd_matrix, ss->shape1_row, ss->shape2_column, DType, ss->relu_threshhold);
//     }

//     if (GLOBAL_DBG)
//         cout << "rd:\n" << rd_matrix << endl;

//     return 0;
// }

// template <typename DType>
// int vemul_mv(DType *rs1, DType *rs2, DType *rd, struct ShapeStride *ss)
// {
//     DEFINE_MAP_DTYPE(DType)

//     Map_DType rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
//     Map_DType rs2_vector(rs2, 1, ss->shape1_row, DynStride(1, 1));
//     Map_DType rd_vector(rd, 1, ss->shape1_column, DynStride(1, 1));

//     if (GLOBAL_DBG) {
//         SHAPE_STRIDE_INFO(ss);
//         cout << "rs1:\n" << rs1_matrix << endl;
//         cout << "rs2:\n" << rs2_vector << endl;
//     }

//     /* dot only support vector not support matrix, so we use '*' to do calculation */
//     rd_vector = rs2_vector * rs1_matrix;
//     if (GLOBAL_DBG)
//         cout << "rd:\n" << rd_vector << endl;

//     return 0;
// }

template <typename DType>
int veemul_mm(DType *rs1, DType *rd, DType *rs2, struct ShapeStride *ss, bool relu)
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

    rd_matrix = rs1_matrix.array() * rs2_matrix.array();

    if (relu) {
        MATRIX_RELU_THRESHHOLD(rd_matrix, rd_matrix, ss->shape1_row, ss->shape1_column, DType, ss->relu_threshhold);
    }

    if (GLOBAL_DBG)
        cout << "rd:" << endl << rd_matrix << endl;

    return 0;
}

template <typename DType>
int veemul_mv(DType *rs1, DType *rd, DType *rs2, struct ShapeStride *ss, int dim, bool relu)
{
    DEFINE_MAP_DTYPE(DType)

    Map_DType rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_DType rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));
    Map_DType vector_dim1(rs2, ss->shape1_row, 1, DynStride(1, 1));
    Map_DType vector_dim0(rs2, 1, ss->shape1_column, DynStride(1, 1));

    if (GLOBAL_DBG) {
        SHAPE_STRIDE_INFO(ss);
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rs2:" << endl << vector_dim0 << endl;
    }

    switch (dim) {
    case 0:
        for (int row = 0; row < rs1_matrix.rows(); row++)
            rd_matrix.row(row) = rs1_matrix.row(row).array() * vector_dim0.array();
        if (GLOBAL_DBG)
            cout << "rd:" << endl << rd_matrix << endl;
        break;
    case 1:
        for (int col = 0; col < rs1_matrix.cols(); col++)
            rd_matrix.col(col) = rs1_matrix.col(col).array() * vector_dim1.array();
        if (GLOBAL_DBG)
            cout << "rd:" << endl << rd_matrix << endl;
        break;
    default:
        cout << __FUNCTION__ << "error dim" << endl;
        return -BR_EPARAM;
    }

    if (relu) {
        MATRIX_RELU_THRESHHOLD(rd_matrix, rd_matrix, ss->shape1_row, ss->shape1_column, DType, ss->relu_threshhold);
    }

    return 0;
}

template <typename DType>
int veemul_mf(DType *rs1, DType *rd, DType rs2, struct ShapeStride *ss, bool relu)
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

    rd_matrix = rs1_matrix * rs2;

    if (relu) {
        MATRIX_RELU_THRESHHOLD(rd_matrix, rd_matrix, ss->shape1_row, ss->shape1_column, DType, ss->relu_threshhold);
    }

    if (GLOBAL_DBG)
        cout << "rd:" << endl << rd_matrix << endl;

    return 0;
}

template <typename DType>
int vemax_m(DType *rs1, DType *rd, struct ShapeStride *ss, int dim, bool relu)
{
    DEFINE_MAP_DTYPE(DType)

    Map_DType rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    Map_DType rd_col_max(rd, 1, ss->shape1_column, DynStride(1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, 1);
    Map_DType rd_row_max(rd, ss->shape1_row, 1, DynStride(ss->stride_rd, 1));

    if (GLOBAL_DBG) {
        SHAPE_STRIDE_INFO(ss);
        cout << "rs1:" << endl << rs1_matrix << endl;
    }

    switch (dim) {
    case 0:
        rd_col_max = rs1_matrix.colwise().maxCoeff();

        if (relu) {
            MATRIX_RELU_THRESHHOLD(rd_col_max, rd_col_max, 1, ss->shape1_column, DType, ss->relu_threshhold);
        }

        if (GLOBAL_DBG)
            cout << "rd:" << endl << rd_col_max << endl;
        break;
    case 1:
        rd_row_max = rs1_matrix.rowwise().maxCoeff();

        if (relu) {
            MATRIX_RELU_THRESHHOLD(rd_row_max, rd_row_max, ss->shape1_row, 1, DType, ss->relu_threshhold);
        }

        if (GLOBAL_DBG)
            cout << "rd:" << endl << rd_row_max << endl;
        break;
    default:
        cout << __FUNCTION__ << "error dim" << endl;
        return -BR_EPARAM;
    }
    return 0;
}

template <typename DType>
int vemax_m(DType *rs1, DType *rd, struct ShapeStride *ss, bool relu)
{
    DEFINE_MAP_DTYPE(DType)

    Map_DType rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    Matrix_DType rd_matrix(1, 1);

    if (GLOBAL_DBG) {
        SHAPE_STRIDE_INFO(ss);
        cout << "rs1:" << endl << rs1_matrix << endl;
    }

    rd_matrix(0, 0) = rs1_matrix.maxCoeff();
    if (relu) {
        MATRIX_RELU_THRESHHOLD(rd_matrix, rd_matrix, 1, 1, DType, ss->relu_threshhold);
    }

    *rd = rd_matrix(0, 0);
    return 0;
}

template <typename DType>
int veargmax_m(DType *rs1, uint16_t *rd, struct ShapeStride *ss, int dim)
{
    DEFINE_MAP_DTYPE(DType)
    DEFINE_MAP_DTYPE(uint16_t)

    Map_DType rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    Map_uint16_t rd_col_max(rd, 1, ss->shape1_column, DynStride(1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, 1);
    Map_uint16_t rd_row_max(rd, ss->shape1_row, 1, DynStride(ss->stride_rd, 1));

    Index maxRow, maxCol;

    if (GLOBAL_DBG) {
        SHAPE_STRIDE_INFO(ss);
        cout << "rs1:" << endl << rs1_matrix << endl;
    }

    switch (dim) {
    case 0:
        for (uint32_t i = 0; i < ss->shape1_column; i++) {
            rs1_matrix.col(i).maxCoeff(&maxRow, &maxCol);
            rd_col_max(0, i) = maxRow;
        }

        if (GLOBAL_DBG)
            cout << "rd:" << endl << rd_col_max << endl;
        break;
    case 1:
        for (uint32_t i = 0; i < ss->shape1_row; i++) {
            rs1_matrix.row(i).maxCoeff(&maxRow, &maxCol);
            rd_row_max(i, 0) = maxCol;
        }

        if (GLOBAL_DBG)
            cout << "rd:" << endl << rd_row_max << endl;
        break;
    default:
        cout << __FUNCTION__ << "error dim" << endl;
        return -BR_EPARAM;
    }
    return 0;
}

template <typename DType>
int veargmax_m(DType *rs1, uint32_t *rd, struct ShapeStride *ss)
{
    DEFINE_MAP_DTYPE(DType)
    Map_DType rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));

    if (GLOBAL_DBG) {
        SHAPE_STRIDE_INFO(ss);
        cout << "rs1:" << endl << rs1_matrix << endl;
    }

    Index maxRow, maxCol;
    DType max, max1;
    //rs1_matrix.maxCoeff(&maxRow, &maxCol);
    max = rs1_matrix(0, 0);
    maxRow = 0;
    maxCol = 0;

    for (uint32_t i = 0; i < ss->shape1_row; i++) {
        for (uint32_t j = 0; j < ss->shape1_column; j++) {
            max1 = rs1_matrix(i, j);
            if (max < max1) {
                max = max1;
                maxRow = i;
                maxCol = j;
            }
        }
    }

    *(uint32_t *)rd = maxCol << 16 | maxRow;

    if (GLOBAL_DBG) {
        std::cout << "max:" << max << std::endl;
        std::cout << "maxRow:" << maxRow <<  "maxCol:" << maxCol << std::endl;
        std::cout << "rd:" << *rd << std::endl;
    }

    return 0;
}

template <typename DType>
int veargmin_m(DType *rs1, uint16_t *rd, struct ShapeStride *ss, int dim)
{
    DEFINE_MAP_DTYPE(DType)
    DEFINE_MAP_DTYPE(uint16_t)

    Map_DType rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    Map_uint16_t rd_col_max(rd, 1, ss->shape1_column, DynStride(1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, 1);
    Map_uint16_t rd_row_max(rd, ss->shape1_row, 1, DynStride(ss->stride_rd, 1));

    Index minRow, minCol;

    if (GLOBAL_DBG) {
        SHAPE_STRIDE_INFO(ss);
        cout << "rs1:" << endl << rs1_matrix << endl;
    }

    switch (dim) {
    case 0:
        for (uint32_t i = 0; i < ss->shape1_column; i++) {
            rs1_matrix.col(i).minCoeff(&minRow, &minCol);
            rd_col_max(0, i) = minRow;
        }

        if (GLOBAL_DBG)
            cout << "rd:" << endl << rd_col_max << endl;
        break;
    case 1:
        for (uint32_t i = 0; i < ss->shape1_row; i++) {
            rs1_matrix.row(i).minCoeff(&minRow, &minCol);
            rd_row_max(i, 0) = minCol;
        }

        if (GLOBAL_DBG)
            cout << "rd:" << endl << rd_row_max << endl;
        break;
    default:
        cout << __FUNCTION__ << "error dim" << endl;
        return -BR_EPARAM;
    }
    return 0;
}

template <typename DType>
int veargmin_m(DType *rs1, uint32_t *rd, struct ShapeStride *ss)
{
    DEFINE_MAP_DTYPE(DType)
    Map_DType rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));

    if (GLOBAL_DBG) {
        SHAPE_STRIDE_INFO(ss);
        cout << "rs1:" << endl << rs1_matrix << endl;
    }

    Index minRow, minCol;
    //rs1_matrix.minCoeff(&minRow, &minCol);

    DType min, min1;
    min = rs1_matrix(0, 0);
    minRow = 0;
    minCol = 0;

    for (uint32_t i = 0; i < ss->shape1_row; i++) {
        for (uint32_t j = 0; j < ss->shape1_column; j++) {
            min1 = rs1_matrix(i, j);
            if (min > min1) {
                min = min1;
                minRow = i;
                minCol = j;
            }
        }
    }

    *(uint32_t *)rd = minCol << 16 | minRow;

    if (GLOBAL_DBG) {
        std::cout << "min:" << min << std::endl;
        std::cout << "minRow:" << minRow <<  "minCol:" << minCol << std::endl;
        std::cout << "rd:" << *rd << std::endl;
    }

    return 0;
}

template <typename DType>
int vemax_mm(DType *rs1, DType *rd, DType *rs2, struct ShapeStride *ss, bool relu)
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

    rd_matrix = (rs1_matrix.array() > rs2_matrix.array()).select(rs1_matrix, rs2_matrix);
    if (relu) {
        MATRIX_RELU_THRESHHOLD(rd_matrix, rd_matrix, ss->shape1_row, ss->shape1_column, DType, ss->relu_threshhold);
    }

    if (GLOBAL_DBG)
        cout << "rd:" << endl << rd_matrix << endl;

    return 0;
}

template <typename DType>
int vemax_mf(DType *rs1, DType *rd, DType rs2, struct ShapeStride *ss, bool relu)
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

    rd_matrix = (rs1_matrix.array() > rs2).select(rs1_matrix, rs2);
    if (relu) {
        MATRIX_RELU_THRESHHOLD(rd_matrix, rd_matrix, ss->shape1_row, ss->shape1_column, DType, ss->relu_threshhold);
    }

    if (GLOBAL_DBG)
        cout << "rd:" << endl << rd_matrix << endl;

    return 0;
}

template <typename DType>
int vemax_mv(DType *rs1, DType *rd, DType *rs2, struct ShapeStride *ss, int dim, bool relu)
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
            rd_matrix.row(row) = (rs1_matrix.row(row).array() > vector_dim0.array()).select(
                rs1_matrix.row(row), vector_dim0);

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
            rd_matrix.col(col) = (rs1_matrix.col(col).array() > vector_dim1.array()).select(
                rs1_matrix.col(col), vector_dim1);

        if (GLOBAL_DBG)
            cout << "rd:" << endl << rd_matrix << endl;

        break;
    default:
        cout << __FUNCTION__ << "error dim" << endl;
        return -BR_EPARAM;
    }

    if (relu) {
        MATRIX_RELU_THRESHHOLD(rd_matrix, rd_matrix, ss->shape1_row, ss->shape1_column, DType, ss->relu_threshhold);
    }

    return 0;
}

template <typename DType>
int vemin_m(DType *rs1, DType *rd, struct ShapeStride *ss, int dim, bool relu)
{
    DEFINE_MAP_DTYPE(DType)

    Map_DType rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    Map_DType rd_col_max(rd, 1, ss->shape1_column, DynStride(1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, 1);
    Map_DType rd_row_max(rd, ss->shape1_row, 1, DynStride(ss->stride_rd, 1));

    if (GLOBAL_DBG) {
        SHAPE_STRIDE_INFO(ss);
        cout << "rs1:" << endl << rs1_matrix << endl;
    }

    switch (dim) {
    case 0:
        rd_col_max = rs1_matrix.colwise().minCoeff();
        if (relu) {
            MATRIX_RELU_THRESHHOLD(rd_col_max, rd_col_max, 1, ss->shape1_column, DType, ss->relu_threshhold);
        }

        if (GLOBAL_DBG)
            cout << "rd:" << endl << rd_col_max << endl;
        break;
    case 1:
        rd_row_max = rs1_matrix.rowwise().minCoeff();
        if (relu) {
            MATRIX_RELU_THRESHHOLD(rd_row_max, rd_row_max, ss->shape1_row, 1, DType, ss->relu_threshhold);
        }

        if (GLOBAL_DBG)
            cout << "rd:" << endl << rd_row_max << endl;
        break;
    default:
        cout << __FUNCTION__ << "error dim" << endl;
        return -BR_EPARAM;
    }
    return 0;
}

template <typename DType>
int vemin_m(DType *rs1, DType *rd, struct ShapeStride *ss, bool relu)
{
    DEFINE_MAP_DTYPE(DType)

    Map_DType rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    Matrix_DType rd_matrix(1, 1);

    if (GLOBAL_DBG) {
        SHAPE_STRIDE_INFO(ss);
        cout << "rs1:" << endl << rs1_matrix << endl;
    }

    rd_matrix(0, 0) = rs1_matrix.minCoeff();
    if (relu) {
        MATRIX_RELU_THRESHHOLD(rd_matrix, rd_matrix, 1, 1, DType, ss->relu_threshhold);
    }

    *rd = rd_matrix(0, 0);
    return 0;
}

template <typename DType>
int vemin_mm(DType *rs1, DType *rd, DType *rs2, struct ShapeStride *ss, bool relu)
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
        cout << "rd:" << endl << rd_matrix << endl;
    }

    rd_matrix = (rs1_matrix.array() < rs2_matrix.array()).select(rs1_matrix, rs2_matrix);
    if (relu) {
        MATRIX_RELU_THRESHHOLD(rd_matrix, rd_matrix, ss->shape1_row, ss->shape1_column, DType, ss->relu_threshhold);
    }

    if (GLOBAL_DBG)
        cout << "rd:" << endl << rd_matrix << endl;

    return 0;
}

template <typename DType>
int vemin_mf(DType *rs1, DType *rd, DType rs2, struct ShapeStride *ss, bool relu)
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

    rd_matrix = (rs1_matrix.array() < rs2).select(rs1_matrix, rs2);
    if (relu) {
        MATRIX_RELU_THRESHHOLD(rd_matrix, rd_matrix, ss->shape1_row, ss->shape1_column, DType, ss->relu_threshhold);
    }

    if (GLOBAL_DBG)
        cout << "rd:" << endl << rd_matrix << endl;

    return 0;
}

template <typename DType>
int vemin_mv(DType *rs1, DType *rd, DType *rs2, struct ShapeStride *ss, int dim, bool relu)
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
            rd_matrix.row(row) = (rs1_matrix.row(row).array() < vector_dim0.array()).select(
                rs1_matrix.row(row), vector_dim0);
        break;
    case 1:
        if (GLOBAL_DBG) {
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

    if (relu) {
        MATRIX_RELU_THRESHHOLD(rd_matrix, rd_matrix, ss->shape1_row, ss->shape1_column, DType, ss->relu_threshhold);
    }

    if (GLOBAL_DBG)
        cout << "rd:" << endl << rd_matrix << endl;
    return 0;
}

template <typename DType>
int velkrelu_mf(DType *rs1, DType rs2, DType *rd, struct ShapeStride *ss, bool relu)
{
    DEFINE_MAP_DTYPE(DType)

    Map_DType rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_DType rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    if (GLOBAL_DBG) {
        SHAPE_STRIDE_INFO(ss);
        cout << "rs1:\n" << rs1_matrix << endl;
    }

    rd_matrix = (rs1_matrix.array() < (DType)0).select(rs1_matrix * rs2, rs1_matrix);
    if (relu) {
        MATRIX_RELU_THRESHHOLD(rd_matrix, rd_matrix, ss->shape1_row, ss->shape1_column, DType, ss->relu_threshhold);
    }
    if (GLOBAL_DBG)
        cout << "rd:\n" << rd_matrix << endl;

    return 0;
}

template <typename DType>
int velkrelu_mv(DType *rs1, DType *rd, DType *rs2, struct ShapeStride *ss, int dim, bool relu)
{
    DEFINE_MAP_DTYPE(DType)

    Map_DType rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_DType rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));
    Map_DType vector_dim1(rs2, ss->shape1_row, 1, DynStride(1, 1));
    Map_DType vector_dim0(rs2, 1, ss->shape1_column, DynStride(1, 1));

    if (GLOBAL_DBG) {
        SHAPE_STRIDE_INFO(ss);
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "dim: " << dim << endl;
    }

    switch (dim) {
    case 0:
        for (int row = 0; row < rs1_matrix.rows(); row++)
            rd_matrix.row(row) = (rs1_matrix.row(row).array() < (DType)0).select(
                rs1_matrix.row(row).array() * vector_dim0.array(),
                rs1_matrix.row(row));

        if (GLOBAL_DBG) {
            cout << "rs2:" << endl << vector_dim0 << endl;
            cout << "rd:" << endl << rd_matrix << endl;
        }
        break;
    case 1:
        for (int col = 0; col < rs1_matrix.cols(); col++)
            rd_matrix.col(col) = (rs1_matrix.col(col).array() < (DType)0).select(
                rs1_matrix.col(col).array() * vector_dim1.array(),
                rs1_matrix.col(col));

        if (GLOBAL_DBG) {
            cout << "rs2:" << endl << vector_dim1 << endl;
            cout << "rd:" << endl << rd_matrix << endl;
        }
        break;
    default:
        cout << __FUNCTION__ << "error dim" << endl;
        return -BR_EPARAM;
    }

    if (relu) {
        MATRIX_RELU_THRESHHOLD(rd_matrix, rd_matrix, ss->shape1_row, ss->shape1_column, DType, ss->relu_threshhold);
    }

    return 0;
}

template <typename AddrDType, typename DType>
int velut_m(AddrDType *rs1, unsigned long rs2, DType *rd, struct ShapeStride *ss, bool relu)
{
    DEFINE_MAP_DTYPE(AddrDType)
    DEFINE_MAP_DTYPE(DType)

    Map_AddrDType rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_DType rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    if (GLOBAL_DBG) {
        SHAPE_STRIDE_INFO(ss);
        cout << "rs1:" << endl << rs1_matrix << endl;
    }


    DType *rd_buf = (DType *)malloc(ss->shape1_row * ss->shape1_column * sizeof(DType));
    Map_DType rd_result(rd_buf, ss->shape1_row, ss->shape1_column, DynStride(ss->shape1_column, 1));

    for (int i = 0; i < ss->shape1_row; i++)
        for (int j = 0; j < ss->shape1_column; j++)
            rd_result(i, j) = *((DType *)rs2 + rs1_matrix(i, j));

    rd_matrix = rd_result;
    free(rd_buf);

    if (relu) {
        MATRIX_RELU_THRESHHOLD(rd_matrix, rd_matrix, ss->shape1_row, ss->shape1_column, DType, ss->relu_threshhold);
    }

    if (GLOBAL_DBG)
        cout << "rd:" << endl << rd_matrix << endl;

    return 0;
}

template <typename OutDType, typename InDType>
int veavgpool_m(OutDType *rs1, OutDType *rd, struct VmeShapeStride *vss, bool relu)
{
    DEFINE_MAP_DTYPE(OutDType)
    DEFINE_MAP_DTYPE(InDType)
  
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
    Map_OutDType rs1_2d(rs1, row_2d, column_2d, DynStride(vss->ifm_c_stride, 1));

    // rs1 with padding
    int row_2d_padded, column_2d_padded, row_3d_padded, column_3d_padded;
    row_3d_padded = vss->row + vss->n_pad_u + vss->n_pad_d;
    column_3d_padded = vss->column + vss->n_pad_l + vss->n_pad_r;
    row_2d_padded = row_3d_padded * column_3d_padded;
    column_2d_padded = vss->cin;
    OutDType *padded_buf = (OutDType *)malloc(row_2d_padded * column_2d_padded * sizeof(OutDType));
    Map_OutDType rs1_2d_padded(padded_buf, row_2d_padded, column_2d_padded, DynStride(vss->cin, 1));

    // zero padding
    OutDType *padding_buf = (OutDType *)malloc(vss->cin * sizeof(OutDType));
    Map_OutDType padding(padding_buf, 1, vss->cin, DynStride(1, 1));
    padding = padding.Constant(1, vss->cin, OutDType(0));
    PADDING_3D_HW_C(rs1_2d, rs1_2d_padded, padding, row_2d_padded, row_3d_padded, column_3d_padded,
        vss->n_pad_u, vss->n_pad_d, vss->n_pad_l, vss->n_pad_r);


    // to save fetched block
    int row_2d_fetch, column_2d_fetch, stride_fetch;
    row_2d_fetch = vss->kh;
    column_2d_fetch = vss->cin * vss->kw;
    stride_fetch = vss->cin * column_3d_padded;
    OutDType *fb_buf = (OutDType *)malloc(row_2d_fetch * column_2d_fetch * sizeof(OutDType));
    Map_OutDType fetch_block(fb_buf, row_2d_fetch, column_2d_fetch, DynStride(column_2d_fetch, 1));

    // reshape fetched block by using same buf
    int row_block_reshaped, col_block_reshaped;
    row_block_reshaped = vss->kw * vss->kh;
    col_block_reshaped = vss->cin;
    Map_OutDType block_reshaped(fb_buf, row_block_reshaped, col_block_reshaped, DynStride(col_block_reshaped, 1));

    // save (block * 1/n), inner dtype
    InDType *block_indtype_buf = (InDType *)malloc(row_block_reshaped * col_block_reshaped * sizeof(InDType));
    Map_InDType block_avged(block_indtype_buf, row_block_reshaped, col_block_reshaped, DynStride(col_block_reshaped, 1));

    // recip vector 1/n
    OutDType *recip_buf = (OutDType *)malloc(row_block_reshaped * sizeof(OutDType));
    Map_OutDType vec_rescip(recip_buf, row_block_reshaped, 1, DynStride(1, 1));

    // output, flatten to 2d
    int row_2d_out, column_2d_out;
    int row_3d_out = (vss->row + vss->n_pad_u + vss->n_pad_d - vss->kh) / vss->sh + 1;
    int col_3d_out = (vss->column + vss->n_pad_l + vss->n_pad_r - vss->kw) / vss->sw + 1;
    row_2d_out = row_3d_out * col_3d_out;
    column_2d_out = vss->cin;
    OutDType *out_buf = (OutDType *)malloc(row_2d_out * column_2d_out * sizeof(OutDType));
    Map_OutDType out_2d(out_buf, row_2d_out, column_2d_out, DynStride(column_2d_out, 1));
    Map_OutDType rd_2d(rd, vss->hout * vss->wout, vss->cin, DynStride(vss->ofm_c_stride,1));
    rd_2d = rd_2d.Constant(vss->hout * vss->wout, vss->cin, OutDType(0));

    // internal result, one row, get rd_row_intype then convert to rd_row
    OutDType *rd_row_buf = (OutDType *)malloc(column_2d_out * sizeof(OutDType));
    Map_OutDType rd_row(rd_row_buf, 1, column_2d_out, DynStride(1, 1));
    InDType *rd_row_intype_buf = (InDType *)malloc(column_2d_out * sizeof(InDType));
    Map_InDType rd_row_intype(rd_row_intype_buf, 1, column_2d_out, DynStride(1, 1));

    int n, row_valid, col_valid;
    int start_row = 0;
    int start_col = 0;
    int rd_row_idx = 0;
    OutDType *recip_tab = nullptr;
    if (typeid(OutDType) == typeid(half))
        recip_tab = (OutDType *)recip_table_half;
    else if (typeid(OutDType) == typeid(Bfloat16))
        recip_tab = (OutDType *)recip_table_Bfloat16;
    else if (typeid(OutDType) == typeid(Float32))
        recip_tab = (OutDType *)recip_table_Float32;


    OutDType *fetch_base = padded_buf;
    while (1) {
        // calculate n and creat a 1/n vector
        LINE_WINDOWS_COMMON_LENGTH(start_row, vss->kh, vss->n_pad_u, vss->n_pad_d, vss->row, row_valid);
        LINE_WINDOWS_COMMON_LENGTH(start_col, vss->kw, vss->n_pad_l, vss->n_pad_r, vss->column, col_valid);
        n = row_valid * col_valid;
        vec_rescip = vec_rescip.Constant(row_block_reshaped, 1, recip_tab[n]);

        // fetch next block and reshape
        Map_OutDType fetch(fetch_base, row_2d_fetch, column_2d_fetch, DynStride(stride_fetch, 1));
        fetch_block = fetch;

        // mul 1/n    DEFINE_MAP_DTYPE(InDType)
        MATRIX_MUL_VEC_V_CONVERT(block_reshaped, vec_rescip, block_avged, row_block_reshaped, col_block_reshaped, InDType);

        // acc and convert back to OutDType, only support odd&even mode
        MATRIX_ACC_DIMH_PARITY(block_avged, rd_row_intype, InDType, row_block_reshaped, col_block_reshaped);
        MATRIX_CAST(rd_row_intype, rd_row, OutDType, 1, column_2d_out);

        // get a row in output
        out_2d.row(rd_row_idx) = rd_row;

        if (GLOBAL_DBG) {
            std::cout << "n = " << n << std::endl;
            std::cout << "output row index = " << rd_row_idx << std::endl;
            std::cout << "current start point(" << start_row << "," << start_col << ")" << std::endl;
        }

        // move point
        rd_row_idx++;
        start_col += vss->sw;
        if (start_col + vss->kw > column_3d_padded) {
            start_col = 0;
            start_row += vss->sh;
            if (start_row + vss->kh > row_3d_padded)
                break;
        }
        fetch_base = padded_buf + start_col * vss->cin + start_row * column_3d_padded * vss->cin;
    }

    if (GLOBAL_DBG){
        cout << "rs1: \n" << rs1_2d << endl;
        cout << "rd: \n" << out_2d << endl;
    }

    free(rd_row_intype_buf);
    free(rd_row_buf);
    free(recip_buf);
    free(block_indtype_buf);
    free(fb_buf);
    free(padding_buf);
    free(padded_buf);

    if (relu) {
        MATRIX_RELU_THRESHHOLD(out_2d, out_2d, row_2d_out, column_2d_out, OutDType, vss->relu_threshhold);
    }

    for(int i = 0; i < min(row_3d_out, vss->hout); i++) {
        for (int j = 0; j < min(col_3d_out, vss->wout); j++) {
            rd_2d.row(i * vss->wout + j) = out_2d.row(i * col_3d_out + j);
        }
    }

    free(out_buf);
    
    return 0;
}

template <typename OutDType>
int vemaxpool_m(OutDType *rs1, OutDType *rd, struct VmeShapeStride *vss, bool relu)
{
    int pad_top, pad_bottom, pad_left, pad_right;
    int kw, kh, okw, okh, k_stride, sk_h, sk_w;
    int in_w, in_h, in_c, in_stride;
    int out_w, out_h, out_stride;
    int w, h, c;
    int dilation_h, dilation_w;
    int i, j, k, ii, jj, kk, index_cin, counter;
    int row, col;
    OutDType *rs1_start;
    OutDType *left_val, *row_val, *col_val;
    OutDType *start;
    OutDType val;

    //get the padding
    pad_top = vss->n_pad_u;
    pad_bottom = vss->n_pad_d;
    pad_left = vss->n_pad_l;
    pad_right = vss->n_pad_r;

    //get the input shape
    in_w = vss->column;
    in_h = vss->row;
    in_c = vss->cin;
    assert(in_w > 0 && in_h > 0 && in_c > 0);
    in_stride = vss->ifm_c_stride;
    in_stride = in_stride > 0 ? in_stride : in_c;

    //get the output shape
    out_w = vss->wout;
    out_h = vss->hout;
    assert(out_w > 0 && out_h > 0);
    out_stride = vss->ofm_c_stride;
    out_stride = out_stride > 0 ? out_stride : in_c;

    //get the kernel shape
    kw = vss->kw;
    kh = vss->kh;
    sk_h = vss->sh;
    sk_w = vss->sw == 0? vss->sh: vss->sw;
    assert((kw > 0) && (kh > 0) && (vss->kw * vss->kh <= 64));
    assert(sk_h >0 && sk_w > 0);
    k_stride = vss->k_c_stride;
    k_stride = k_stride > 0 ? k_stride : in_c;

    DEFINE_MAP_DTYPE(OutDType)

    VME_SHAPE_STRIDE_INFO(vss);


    h = kh;
    w = kw;
    okh = kh;
    okw = kw;
    kh = h;
    kw = w;

    /*calculate the input shape*/
    Map_OutDType rs1_matrix(rs1, in_h * in_w, in_c, DynStride(in_stride, 1));

    if (GLOBAL_DBG) {
        cout << "rs1:" << endl << rs1_matrix << endl;
    }

    /*calculate the output shape*/
    h = (in_h + pad_top + pad_bottom - kh + 1 + sk_h - 1) / sk_h;
    w = (in_w + pad_left + pad_right - kw + 1 + sk_w - 1) / sk_w;
    left_val = (OutDType *)malloc(h * w * okh * okw * in_c * sizeof(OutDType));

    for (i = 0; i < h; i++) {
        for (j = 0; j < w; j++) {
            start = left_val + i * w * okh * okw * in_c + j * okh * okw * in_c;
            rs1_start = rs1;

            for (ii = 0; ii < kh; ii++) {
                for (jj = 0; jj < kw; jj++) {
                    for (kk = 0; kk < in_c; kk++) {
                        row = i * sk_h + ii;
                        col = j * sk_w + jj;
				        int start_offset = ii * okw * in_c + jj * in_c + kk;
				        int rs1_offset = (row - pad_top) * in_w * in_stride + (col - pad_left) * in_stride + kk;
                        if (row >= pad_top && row < pad_top + in_h && col >= pad_left && col < pad_left + in_w) {
                            *(start + start_offset) = *(rs1_start + rs1_offset);
                            val = *(rs1_start + rs1_offset);
                        } else {
                            *(start + start_offset) = OutDType(0);
                            val =  OutDType(0);
                        }
                    }//kk
                }//jj
            }//ii
        }//j
    }//i

    if (GLOBAL_DBG)
        printf("h = %d w = %d out_c = %d\n", h, w, in_c);
    assert(h == out_h && w == out_w);

    /*calculate convolution*/
    Map_OutDType left_matrix(left_val, h * w, okh * okw * in_c, DynStride(okh * okw * in_c, 1));
    Map_OutDType rd_matrix(rd, out_h * out_w, in_c, DynStride(out_stride, 1));
    OutDType res;

    //rd_matrix = left_matrix * rs2_matrix;
    for (i = 0; i < out_h * out_w; i++) {
        for (j = 0; j < in_c; j++) {
            counter=0;
            for (k = 0; k < okh; k++) {
                if (((i / out_w) * sk_h + k) < pad_top) continue;
                if (((i / out_w) * sk_h + k) >= (in_h + pad_top)) continue;
                for (ii = 0; ii < okw; ii++) {
                    if ((((i % out_w) * sk_w) + ii) < pad_left) continue;
                    if ((((i % out_w) * sk_w) + ii) >= (in_w + pad_left)) continue;
                    if (counter == 0) 
                        res = left_matrix(i, (okw*k+ii)*in_c+j);
                    if (res < left_matrix(i, (okw*k+ii)*in_c+j))
                        res = left_matrix(i, (okw*k+ii)*in_c+j);
                    counter++;
                }
            }
            rd_matrix(i, j) = res;
        }
        
    }
    if (GLOBAL_DBG) {
        cout << "rd: " << rd_matrix << endl;
    }

    free(row_val);
    free(col_val);
    free(left_val);
    return 0;
}

template <typename OutDType, typename InDType>
int vedwconv_mm(OutDType *rs1, OutDType *rs2, OutDType *rd, struct VmeShapeStride *vss)
{
    int pad_top, pad_bottom, pad_left, pad_right;
    int kw, kh, okw, okh, k_stride, sk_h, sk_w;
    int in_w, in_h, in_c, in_stride;
    int out_w, out_h, out_stride;
    int w, h, c;
    int dilation_h, dilation_w;
    int i, j, k, ii, jj, kk, index_cin, counter;
    int row, col;
    OutDType *rs1_start;
    OutDType *left_val, *row_val, *col_val;
    OutDType *start;
    OutDType val;

    //get the padding
    pad_top = vss->n_pad_u;
    pad_bottom = vss->n_pad_d;
    pad_left = vss->n_pad_l;
    pad_right = vss->n_pad_r;

    //get the input shape
    in_w = vss->column;
    in_h = vss->row;
    in_c = vss->cin;
    assert(in_w > 0 && in_h > 0 && in_c > 0);
    in_stride = vss->ifm_c_stride;
    in_stride = in_stride > 0 ? in_stride : in_c;

    //get the output shape
    out_w = vss->wout;
    out_h = vss->hout;
    assert(out_w > 0 && out_h > 0);
    out_stride = vss->ofm_c_stride;
    out_stride = out_stride > 0 ? out_stride : in_c;

    //get the kernel shape
    kw = vss->kw;
    kh = vss->kh;
    dilation_h = vss->k_dilation_h;
    dilation_w = vss->k_dilation_w;
    sk_h = vss->sh;
    sk_w = vss->sw == 0? vss->sh: vss->sw;
    assert((kw > 0) && (kh > 0) && (vss->kw * vss->kh <= 64));
    assert(dilation_h > 0 && dilation_w > 0 && sk_h >0 && sk_w > 0);
    k_stride = vss->k_c_stride;
    k_stride = k_stride > 0 ? k_stride : in_c;

    DEFINE_MAP_DTYPE(OutDType)
    DEFINE_MAP_DTYPE(InDType)

    VME_SHAPE_STRIDE_INFO(vss);

    /*calculate the kernel shape*/
    Map_OutDType rs2_matrix(rs2, kh * kw, in_c, DynStride(k_stride, 1)); // the depth is same as in_c

    h = dilation_h > 1 ? dilation_h * (kh - 1) + 1 : kh;
    w = dilation_w > 1 ? dilation_w * (kw - 1) + 1 : kw;
    okh = kh;
    okw = kw;
    kh = h;
    kw = w;

    /*calculate the input shape*/
    Map_OutDType rs1_matrix(rs1, in_h * in_w, in_c, DynStride(in_stride, 1));

    if (GLOBAL_DBG) {
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rs2:" << endl << rs2_matrix << endl;
    }

    /*calculate the output shape*/
    h = (in_h + pad_top + pad_bottom - kh + 1 + sk_h - 1) / sk_h;
    w = (in_w + pad_left + pad_right - kw + 1 + sk_w - 1) / sk_w;
    left_val = (OutDType *)malloc(h * w * okh * okw * in_c * sizeof(OutDType));

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
                                    *(start + start_offset) = OutDType(0);
                                    val =  OutDType(0);
                                }
                            }//jj % dilation_w
                        }//ii % dilation_h
                    }//kk
                }//jj
            }//ii
        }//j
    }//i

    if (GLOBAL_DBG)
        printf("h = %d w = %d out_c = %d\n", h, w, in_c);
    assert(h == out_h && w == out_w);

    /*calculate convolution*/
    Map_OutDType left_matrix(left_val, h * w, okh * okw * in_c, DynStride(okh * okw * in_c, 1));
    Map_OutDType rd_matrix(rd, out_h * out_w, in_c, DynStride(out_stride, 1));
    InDType odd, even;
    InDType res_tmp;

    //rd_matrix = left_matrix * rs2_matrix;
    for (i = 0; i < out_h * out_w; i++) {
        for (j = 0; j < in_c; j++) {
            odd.x = 0x80000000;
            even.x = 0x80000000;
            for (k = 0; k < okh * okw; k++) {
                if (k%2)
                    odd = InDType::mulConvert(left_matrix(i, k*in_c+j), rs2_matrix(k, j)) + odd;
                else
                    even = InDType::mulConvert(left_matrix(i, k*in_c+j), rs2_matrix(k, j)) + even;
            }
            rd_matrix(i, j) = OutDType(odd+even);
        }
    }
    if (GLOBAL_DBG) {
        cout << "rd: " << rd_matrix << endl;
    }

    free(row_val);
    free(col_val);
    free(left_val);
    return 0;
}

template <typename OutDType, typename InDType>
int veemul_xx_xx_mf(InDType *rs1, OutDType *rd, InDType rs2, struct ShapeStride *ss)
{
    DEFINE_MAP_DTYPE(OutDType)
    DEFINE_MAP_DTYPE(InDType)

    Map_InDType rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_OutDType rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    if (GLOBAL_DBG)  {
        SHAPE_STRIDE_INFO(ss);
        cout << "Start veemul mf" << endl;
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rs2:" << endl << rs2 << endl;
    }


    for (int row = 0; row < rs1_matrix.rows(); row++) {
        for (int col = 0; col < rs1_matrix.cols(); col++) {
            InDType val = rs2 * rs1_matrix(row, col);
            rd_matrix(row, col) = (OutDType)val;
        }
    }

    if (GLOBAL_DBG)
        cout << "rd:" << endl << rd_matrix << endl;

    return 0;
}

template <typename OutDType, typename InDType>
int veemul_x8_hf_mf(InDType *rs1, OutDType *rd, InDType rs2, struct ShapeStride *ss, uint32_t rounding_mode)
{
    DEFINE_MAP_DTYPE(OutDType)
    DEFINE_MAP_DTYPE(InDType)

    Map_InDType rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_OutDType rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    if (GLOBAL_DBG)  {
        SHAPE_STRIDE_INFO(ss);
        cout << "Start veemul mf" << endl;
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rs2:" << endl << rs2 << endl;
    }

    InDType val;
    float16_t f16;
    for (int row = 0; row < rs1_matrix.rows(); row++) {
        for (int col = 0; col < rs1_matrix.cols(); col++) {
            val =  rs1_matrix(row, col) * rs2;
            f16.v = val.x;
            rd_matrix(row, col) = f16_to_i8(f16, rounding_mode, true);
        }
    }

    if (GLOBAL_DBG)
        cout << "rd:" << endl << rd_matrix << endl;

    return 0;
}

template <typename OutDType, typename InDType>
int veemul_xu8_hf_mf(InDType *rs1, OutDType *rd, InDType rs2, struct ShapeStride *ss, uint32_t rounding_mode)
{
    DEFINE_MAP_DTYPE(OutDType)
    DEFINE_MAP_DTYPE(InDType)

    Map_InDType rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_OutDType rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    if (GLOBAL_DBG)  {
        SHAPE_STRIDE_INFO(ss);
        cout << "Start veemul mf" << endl;
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rs2:" << endl << rs2 << endl;
    }

    InDType val;
    float16_t f16;
    for (int row = 0; row < rs1_matrix.rows(); row++) {
        for (int col = 0; col < rs1_matrix.cols(); col++) {
            val = rs1_matrix(row, col) * rs2;
            f16.v = val.x;
            rd_matrix(row, col) = f16_to_ui8(f16, rounding_mode, true);
        }
    }

    if (GLOBAL_DBG)
        cout << "rd:" << endl << rd_matrix << endl;

    return 0;
}

template <typename OutDType, typename InDType>
int veemul_x8_bf_mf(InDType *rs1, OutDType *rd, InDType rs2, struct ShapeStride *ss, uint32_t rounding_mode)
{
    DEFINE_MAP_DTYPE(OutDType)
    DEFINE_MAP_DTYPE(InDType)

    Map_InDType rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_OutDType rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    if (GLOBAL_DBG)  {
        SHAPE_STRIDE_INFO(ss);
        cout << "Start veemul mf" << endl;
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rs2:" << endl << rs2 << endl;
    }

    InDType val;
    bfloat16_t bf16, rs1_bf16, rs2_bf16;
    for (int row = 0; row < rs1_matrix.rows(); row++) {
        for (int col = 0; col < rs1_matrix.cols(); col++) {
            rs1_bf16.v = rs1_matrix(row, col).x;
            rs2_bf16.v = rs2.x;
            bf16 = bf16_mul(rs1_bf16, rs2_bf16);
            rd_matrix(row, col) = bf16_to_i8(bf16, rounding_mode, true);
        }
    }

    if (GLOBAL_DBG)
        cout << "rd:" << endl << rd_matrix << endl;

    return 0;
}

template <typename OutDType, typename InDType>
int veemul_xu8_bf_mf(InDType *rs1, OutDType *rd, InDType rs2, struct ShapeStride *ss, uint32_t rounding_mode)
{
    DEFINE_MAP_DTYPE(OutDType)
    DEFINE_MAP_DTYPE(InDType)

    Map_InDType rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_OutDType rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    if (GLOBAL_DBG)  {
        SHAPE_STRIDE_INFO(ss);
        cout << "Start veemul mf" << endl;
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rs2:" << endl << rs2 << endl;
    }

    InDType val;
    bfloat16_t bf16, rs1_bf16, rs2_bf16;
    for (int row = 0; row < rs1_matrix.rows(); row++) {
        for (int col = 0; col < rs1_matrix.cols(); col++) {
            rs1_bf16.v = rs1_matrix(row, col).x;
            rs2_bf16.v = rs2.x;
            bf16 = bf16_mul(rs1_bf16, rs2_bf16);
            rd_matrix(row, col) = bf16_to_ui8(bf16, rounding_mode, true);
        }
    }

    if (GLOBAL_DBG)
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

/**
 * versqrt_m() versqrt.m
 * 
 * 矩阵元素rsqrt运算，正常算术运算 M = rsqrt(Mij)
 * @param rs1 M1,源操作矩阵一基地址
 * @param rd M,目的矩阵基地址
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
template <typename DType>
int versqrt_m(DType *rs1, DType *rd, struct ShapeStride *ss)
{
    DEFINE_MAP_DTYPE(DType)

    Map_DType rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_DType rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    SHAPE_STRIDE_INFO(ss);
    if (GLOBAL_DBG) {
        
        cout << "versqrt_m-rs1:" << endl << rs1_matrix << endl;
    }

    if( is_same< DType, half >::value )
    {
        half rs1;
        float16_t rs1_f16, rd_f16;
        for (int row = 0; row < rs1_matrix.rows(); row ++)
        for (int col = 0; col < rs1_matrix.cols(); col ++) {
            rs1_f16.v = rs1_matrix(row, col).x;
            rd_f16 = f16_rsqrt( rs1_f16 );
            rd_matrix(row, col).x =  rd_f16.v;
        }
    }
    else if( is_same< DType, Bfloat16 >::value )
    {
        Bfloat16 rs1;
        bfloat16_t rs1_bf16, rd_bf16;
        for (int row = 0; row < rs1_matrix.rows(); row ++)
        for (int col = 0; col < rs1_matrix.cols(); col ++) {
            rs1_bf16.v = rs1_matrix(row, col).x;
            rd_bf16 = bf16_rsqrt( rs1_bf16 );
            rd_matrix(row, col).x =  rd_bf16.v;
        }        
    }
    else if( is_same< DType, Float32 >::value )
    {
        Float32 rs1;
        float32_t rs1_f32, rd_f32;
        for (int row = 0; row < rs1_matrix.rows(); row ++)
        for (int col = 0; col < rs1_matrix.cols(); col ++) {
            rs1_f32.v = rs1_matrix(row, col).x;
            rd_f32 = f32_rsqrt( rs1_f32 );
            rd_matrix(row, col).x =  rd_f32.v;
        }          
    }

    if (GLOBAL_DBG)
        cout << "versqrt_m-rd:" << endl << rd_matrix << endl;

    return 0;
}

/**
 * vesqrt_m() vesqrt.m
 * 
 * 矩阵元素sqrt运算，正常算术运算 M = sqrt(Mij)
 * @param rs1 M1,源操作矩阵一基地址
 * @param rd M,目的矩阵基地址
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
template <typename DType>
int vesqrt_m(DType *rs1, DType *rd, struct ShapeStride *ss)
{
    DEFINE_MAP_DTYPE(DType)

    Map_DType rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_DType rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    SHAPE_STRIDE_INFO(ss);
    if (GLOBAL_DBG) {
        
        cout << "vesqrt_m-rs1:" << endl << rs1_matrix << endl;
    }

    if( is_same< DType, half >::value )
    {
        half rs1;
        float16_t rs1_f16, rd_f16;
        for (int row = 0; row < rs1_matrix.rows(); row ++)
        for (int col = 0; col < rs1_matrix.cols(); col ++) {
            rs1_f16.v = rs1_matrix(row, col).x;
            rd_f16 = f16_sqrt_( rs1_f16 );
            rd_matrix(row, col).x =  rd_f16.v;
        }
    }
    else if( is_same< DType, Bfloat16 >::value )
    {
        Bfloat16 rs1;
        bfloat16_t rs1_bf16, rd_bf16;
        for (int row = 0; row < rs1_matrix.rows(); row ++)
        for (int col = 0; col < rs1_matrix.cols(); col ++) {
            rs1_bf16.v = rs1_matrix(row, col).x;
            rd_bf16 = bf16_sqrt_( rs1_bf16 );
            rd_matrix(row, col).x =  rd_bf16.v;
        }        
    }
    else if( is_same< DType, Float32 >::value )
    {
        Float32 rs1;
        float32_t rs1_f32, rd_f32;
        for (int row = 0; row < rs1_matrix.rows(); row ++)
        for (int col = 0; col < rs1_matrix.cols(); col ++) {
            rs1_f32.v = rs1_matrix(row, col).x;
            rd_f32 = f32_sqrt_( rs1_f32 );
            rd_matrix(row, col).x =  rd_f32.v;
        }          
    }

    if (GLOBAL_DBG)
        cout << "vesqrt_m-rd:" << endl << rd_matrix << endl;

    return 0;
}

/**
 * verecip_m() verecip.m
 * 
 * 矩阵元素recip运算，正常算术运算 M = recip(Mij)
 * @param rs1 M1,源操作矩阵一基地址
 * @param rd M,目的矩阵基地址
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
template <typename DType>
int verecip_m(DType *rs1, DType *rd, struct ShapeStride *ss)
{
    DEFINE_MAP_DTYPE(DType)

    Map_DType rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_DType rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    SHAPE_STRIDE_INFO(ss);
    if (GLOBAL_DBG) {
        
        cout << "verecip_m-rs1:" << endl << rs1_matrix << endl;
    }

    if( is_same< DType, half >::value )
    {
        half rs1;
        float16_t rs1_f16, rd_f16;
        for (int row = 0; row < rs1_matrix.rows(); row ++)
        for (int col = 0; col < rs1_matrix.cols(); col ++) {
            rs1_f16.v = rs1_matrix(row, col).x;
            rd_f16 = f16_reciprocal( rs1_f16 );
            rd_matrix(row, col).x =  rd_f16.v;
        }
    }
    else if( is_same< DType, Bfloat16 >::value )
    {
        Bfloat16 rs1;
        bfloat16_t rs1_bf16, rd_bf16;
        for (int row = 0; row < rs1_matrix.rows(); row ++)
        for (int col = 0; col < rs1_matrix.cols(); col ++) {
            rs1_bf16.v = rs1_matrix(row, col).x;
            rd_bf16 = bf16_reciprocal( rs1_bf16 );
            rd_matrix(row, col).x =  rd_bf16.v;
        }        
    }
    else if( is_same< DType, Float32 >::value )
    {
        Float32 rs1;
        float32_t rs1_f32, rd_f32;
        for (int row = 0; row < rs1_matrix.rows(); row ++)
        for (int col = 0; col < rs1_matrix.cols(); col ++) {
            rs1_f32.v = rs1_matrix(row, col).x;
            rd_f32 = f32_reciprocal( rs1_f32 );
            rd_matrix(row, col).x =  rd_f32.v;
        }          
    }

    if (GLOBAL_DBG)
        cout << "verecip_m-rd:" << endl << rd_matrix << endl;

    return 0;
}

/**
 * veexp_m() veexp.m
 * 
 * 矩阵元素exp运算，正常算术运算 M = exp(Mij)
 * @param rs1 M1,源操作矩阵一基地址
 * @param rd M,目的矩阵基地址
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
template <typename DType>
int veexp_m(DType *rs1, DType *rd, struct ShapeStride *ss)
{
    DEFINE_MAP_DTYPE(DType)

    Map_DType rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_DType rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    SHAPE_STRIDE_INFO(ss);
    if (GLOBAL_DBG) {
        
        cout << "veexp_m-rs1:" << endl << rs1_matrix << endl;
    }

    if( is_same< DType, half >::value )
    {
        half rs1;
        float16_t rs1_f16, rd_f16;
        for (int row = 0; row < rs1_matrix.rows(); row ++)
        for (int col = 0; col < rs1_matrix.cols(); col ++) {
            rs1_f16.v = rs1_matrix(row, col).x;
            rd_f16 = f16_exp( rs1_f16 );
            rd_matrix(row, col).x =  rd_f16.v;
        }
    }
    else if( is_same< DType, Bfloat16 >::value )
    {
        Bfloat16 rs1;
        bfloat16_t rs1_bf16, rd_bf16;
        for (int row = 0; row < rs1_matrix.rows(); row ++)
        for (int col = 0; col < rs1_matrix.cols(); col ++) {
            rs1_bf16.v = rs1_matrix(row, col).x;
            rd_bf16 = bf16_exp( rs1_bf16 );
            rd_matrix(row, col).x =  rd_bf16.v;
        }        
    }
    else if( is_same< DType, Float32 >::value )
    {
        Float32 rs1;
        float32_t rs1_f32, rd_f32;
        for (int row = 0; row < rs1_matrix.rows(); row ++)
        for (int col = 0; col < rs1_matrix.cols(); col ++) {
            rs1_f32.v = rs1_matrix(row, col).x;
            rd_f32 = f32_exp( rs1_f32 );
            rd_matrix(row, col).x =  rd_f32.v;
        }          
    }

    if (GLOBAL_DBG)
        cout << "veexp_m-rd:" << endl << rd_matrix << endl;

    return 0;
}

/**
 * vesigmoid_m() vesigmoid.m
 * 
 * 矩阵元素sigmoid运算，正常算术运算 M = sigmoid(Mij)
 * @param rs1 M1,源操作矩阵一基地址
 * @param rd M,目的矩阵基地址
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
template <typename DType>
int vesigmoid_m(DType *rs1, DType *rd, struct ShapeStride *ss)
{
    DEFINE_MAP_DTYPE(DType)

    Map_DType rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_DType rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    SHAPE_STRIDE_INFO(ss);
    if (GLOBAL_DBG) {
        
        cout << "vesigmoid_m-rs1:" << endl << rs1_matrix << endl;
    }

    if( is_same< DType, half >::value )
    {
        half rs1;
        float16_t rs1_f16, rd_f16;
        for (int row = 0; row < rs1_matrix.rows(); row ++)
        for (int col = 0; col < rs1_matrix.cols(); col ++) {
            rs1_f16.v = rs1_matrix(row, col).x;
            rd_f16 = f16_sigmoid( rs1_f16 );
            rd_matrix(row, col).x =  rd_f16.v;
        }
    }
    else if( is_same< DType, Bfloat16 >::value )
    {
        Bfloat16 rs1;
        bfloat16_t rs1_bf16, rd_bf16;
        for (int row = 0; row < rs1_matrix.rows(); row ++)
        for (int col = 0; col < rs1_matrix.cols(); col ++) {
            rs1_bf16.v = rs1_matrix(row, col).x;
            rd_bf16 = bf16_sigmoid( rs1_bf16 );
            rd_matrix(row, col).x =  rd_bf16.v;
        }        
    }
    else if( is_same< DType, Float32 >::value )
    {
        Float32 rs1;
        float32_t rs1_f32, rd_f32;
        for (int row = 0; row < rs1_matrix.rows(); row ++)
        for (int col = 0; col < rs1_matrix.cols(); col ++) {
            rs1_f32.v = rs1_matrix(row, col).x;
            rd_f32 = f32_sigmoid( rs1_f32 );
            rd_matrix(row, col).x =  rd_f32.v;
        }          
    }

    if (GLOBAL_DBG)
        cout << "vesigmoid_m-rd:" << endl << rd_matrix << endl;

    return 0;
}

/**
 * vesinh_m() vesinh.m
 * 
 * 矩阵元素sinh运算，正常算术运算 M = sinh(Mij)
 * @param rs1 M1,源操作矩阵一基地址
 * @param rd M,目的矩阵基地址
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
template <typename DType>
int vesinh_m(DType *rs1, DType *rd, struct ShapeStride *ss)
{
    DEFINE_MAP_DTYPE(DType)

    Map_DType rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_DType rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    SHAPE_STRIDE_INFO(ss);
    if (GLOBAL_DBG) {
        
        cout << "vesinh_m-rs1:" << endl << rs1_matrix << endl;
    }

    if( is_same< DType, half >::value )
    {
        half rs1;
        float16_t rs1_f16, rd_f16;
        for (int row = 0; row < rs1_matrix.rows(); row ++)
        for (int col = 0; col < rs1_matrix.cols(); col ++) {
            rs1_f16.v = rs1_matrix(row, col).x;
            rd_f16 = f16_sinh( rs1_f16 );
            rd_matrix(row, col).x =  rd_f16.v;
        }
    }
    else if( is_same< DType, Bfloat16 >::value )
    {
        Bfloat16 rs1;
        bfloat16_t rs1_bf16, rd_bf16;
        for (int row = 0; row < rs1_matrix.rows(); row ++)
        for (int col = 0; col < rs1_matrix.cols(); col ++) {
            rs1_bf16.v = rs1_matrix(row, col).x;
            rd_bf16 = bf16_sinh( rs1_bf16 );
            rd_matrix(row, col).x =  rd_bf16.v;
        }        
    }
    else if( is_same< DType, Float32 >::value )
    {
        Float32 rs1;
        float32_t rs1_f32, rd_f32;
        for (int row = 0; row < rs1_matrix.rows(); row ++)
        for (int col = 0; col < rs1_matrix.cols(); col ++) {
            rs1_f32.v = rs1_matrix(row, col).x;
            rd_f32 = f32_sinh( rs1_f32 );
            rd_matrix(row, col).x =  rd_f32.v;
        }          
    }

    if (GLOBAL_DBG)
        cout << "vesinh_m-rd:" << endl << rd_matrix << endl;

    return 0;
}

/**
 * vecosh_m() vecosh.m
 * 
 * 矩阵元素cosh运算，正常算术运算 M = cosh(Mij)
 * @param rs1 M1,源操作矩阵一基地址
 * @param rd M,目的矩阵基地址
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
template <typename DType>
int vecosh_m(DType *rs1, DType *rd, struct ShapeStride *ss)
{
    DEFINE_MAP_DTYPE(DType)

    Map_DType rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_DType rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    SHAPE_STRIDE_INFO(ss);
    if (GLOBAL_DBG) {
        
        cout << "vecosh_m-rs1:" << endl << rs1_matrix << endl;
    }

    if( is_same< DType, half >::value )
    {
        half rs1;
        float16_t rs1_f16, rd_f16;
        for (int row = 0; row < rs1_matrix.rows(); row ++)
        for (int col = 0; col < rs1_matrix.cols(); col ++) {
            rs1_f16.v = rs1_matrix(row, col).x;
            rd_f16 = f16_cosh( rs1_f16 );
            rd_matrix(row, col).x =  rd_f16.v;
        }
    }
    else if( is_same< DType, Bfloat16 >::value )
    {
        Bfloat16 rs1;
        bfloat16_t rs1_bf16, rd_bf16;
        for (int row = 0; row < rs1_matrix.rows(); row ++)
        for (int col = 0; col < rs1_matrix.cols(); col ++) {
            rs1_bf16.v = rs1_matrix(row, col).x;
            rd_bf16 = bf16_cosh( rs1_bf16 );
            rd_matrix(row, col).x =  rd_bf16.v;
        }        
    }
    else if( is_same< DType, Float32 >::value )
    {
        Float32 rs1;
        float32_t rs1_f32, rd_f32;
        for (int row = 0; row < rs1_matrix.rows(); row ++)
        for (int col = 0; col < rs1_matrix.cols(); col ++) {
            rs1_f32.v = rs1_matrix(row, col).x;
            rd_f32 = f32_cosh( rs1_f32 );
            rd_matrix(row, col).x =  rd_f32.v;
        }          
    }

    if (GLOBAL_DBG)
        cout << "vecosh_m-rd:" << endl << rd_matrix << endl;

    return 0;
}

/**
 * vetanh_m() vetanh.m
 * 
 * 矩阵元素tanh运算，正常算术运算 M = tanh(Mij)
 * @param rs1 M1,源操作矩阵一基地址
 * @param rd M,目的矩阵基地址
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
template <typename DType>
int vetanh_m(DType *rs1, DType *rd, struct ShapeStride *ss)
{
    DEFINE_MAP_DTYPE(DType)

    Map_DType rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_DType rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    SHAPE_STRIDE_INFO(ss);
    if (GLOBAL_DBG) {
        
        cout << "vetanh_m-rs1:" << endl << rs1_matrix << endl;
    }

    if( is_same< DType, half >::value )
    {
        half rs1;
        float16_t rs1_f16, rd_f16;
        for (int row = 0; row < rs1_matrix.rows(); row ++)
        for (int col = 0; col < rs1_matrix.cols(); col ++) {
            rs1_f16.v = rs1_matrix(row, col).x;
            rd_f16 = f16_tanh( rs1_f16 );
            rd_matrix(row, col).x =  rd_f16.v;
        }
    }
    else if( is_same< DType, Bfloat16 >::value )
    {
        Bfloat16 rs1;
        bfloat16_t rs1_bf16, rd_bf16;
        for (int row = 0; row < rs1_matrix.rows(); row ++)
        for (int col = 0; col < rs1_matrix.cols(); col ++) {
            rs1_bf16.v = rs1_matrix(row, col).x;
            rd_bf16 = bf16_tanh( rs1_bf16 );
            rd_matrix(row, col).x =  rd_bf16.v;
        }        
    }
    else if( is_same< DType, Float32 >::value )
    {
        Float32 rs1;
        float32_t rs1_f32, rd_f32;
        for (int row = 0; row < rs1_matrix.rows(); row ++)
        for (int col = 0; col < rs1_matrix.cols(); col ++) {
            rs1_f32.v = rs1_matrix(row, col).x;
            rd_f32 = f32_tanh( rs1_f32 );
            rd_matrix(row, col).x =  rd_f32.v;
        }          
    }

    if (GLOBAL_DBG)
        cout << "vetanh_m-rd:" << endl << rd_matrix << endl;

    return 0;
}

/**
 * veln_m() veln.m
 * 
 * 矩阵元素ln运算，正常算术运算 M = ln(Mij)
 * @param rs1 M1,源操作矩阵一基地址
 * @param rd M,目的矩阵基地址
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
template <typename DType>
int veln_m(DType *rs1, DType *rd, struct ShapeStride *ss)
{
    DEFINE_MAP_DTYPE(DType)

    Map_DType rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_DType rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    SHAPE_STRIDE_INFO(ss);
    if (GLOBAL_DBG) {
        
        cout << "veln_m-rs1:" << endl << rs1_matrix << endl;
    }

    if( is_same< DType, half >::value )
    {
        half rs1;
        float16_t rs1_f16, rd_f16;
        for (int row = 0; row < rs1_matrix.rows(); row ++)
        for (int col = 0; col < rs1_matrix.cols(); col ++) {
            rs1_f16.v = rs1_matrix(row, col).x;
            rd_f16 = f16_ln( rs1_f16 );
            rd_matrix(row, col).x =  rd_f16.v;
        }
    }
    else if( is_same< DType, Bfloat16 >::value )
    {
        Bfloat16 rs1;
        bfloat16_t rs1_bf16, rd_bf16;
        for (int row = 0; row < rs1_matrix.rows(); row ++)
        for (int col = 0; col < rs1_matrix.cols(); col ++) {
            rs1_bf16.v = rs1_matrix(row, col).x;
            rd_bf16 = bf16_ln( rs1_bf16 );
            rd_matrix(row, col).x =  rd_bf16.v;
        }        
    }
    else if( is_same< DType, Float32 >::value )
    {
        Float32 rs1;
        float32_t rs1_f32, rd_f32;
        for (int row = 0; row < rs1_matrix.rows(); row ++)
        for (int col = 0; col < rs1_matrix.cols(); col ++) {
            rs1_f32.v = rs1_matrix(row, col).x;
            rd_f32 = f32_ln( rs1_f32 );
            rd_matrix(row, col).x =  rd_f32.v;
        }          
    }

    if (GLOBAL_DBG)
        cout << "veln_m-rd:" << endl << rd_matrix << endl;

    return 0;
}

/**
 * vesin_m() vesin.m
 * 
 * 矩阵元素sin运算，正常算术运算 M = sin(Mij)
 * @param rs1 M1,源操作矩阵一基地址
 * @param rd M,目的矩阵基地址
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
template <typename DType>
int vesin_m(DType *rs1, DType *rd, struct ShapeStride *ss)
{
    DEFINE_MAP_DTYPE(DType)

    Map_DType rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_DType rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    SHAPE_STRIDE_INFO(ss);
    if (GLOBAL_DBG) {
        
        cout << "vesin_m-rs1:" << endl << rs1_matrix << endl;
    }

    if( is_same< DType, half >::value )
    {
        half rs1;
        float16_t rs1_f16, rd_f16;
        for (int row = 0; row < rs1_matrix.rows(); row ++)
        for (int col = 0; col < rs1_matrix.cols(); col ++) {
            rs1_f16.v = rs1_matrix(row, col).x;
            rd_f16 = f16_sin( rs1_f16 );
            rd_matrix(row, col).x =  rd_f16.v;
        }
    }
    else if( is_same< DType, Bfloat16 >::value )
    {
        Bfloat16 rs1;
        bfloat16_t rs1_bf16, rd_bf16;
        for (int row = 0; row < rs1_matrix.rows(); row ++)
        for (int col = 0; col < rs1_matrix.cols(); col ++) {
            rs1_bf16.v = rs1_matrix(row, col).x;
            rd_bf16 = bf16_sin( rs1_bf16 );
            rd_matrix(row, col).x =  rd_bf16.v;
        }        
    }
    else if( is_same< DType, Float32 >::value )
    {
        Float32 rs1;
        float32_t rs1_f32, rd_f32;
        for (int row = 0; row < rs1_matrix.rows(); row ++)
        for (int col = 0; col < rs1_matrix.cols(); col ++) {
            rs1_f32.v = rs1_matrix(row, col).x;
            rd_f32 = f32_sin( rs1_f32 );
            rd_matrix(row, col).x =  rd_f32.v;
        }          
    }

    if (GLOBAL_DBG)
        cout << "vesin_m-rd:" << endl << rd_matrix << endl;

    return 0;
}

/**
 * vecos_m() vecos.m
 * 
 * 矩阵元素cos运算，正常算术运算 M = cos(Mij)
 * @param rs1 M1,源操作矩阵一基地址
 * @param rd M,目的矩阵基地址
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
template <typename DType>
int vecos_m(DType *rs1, DType *rd, struct ShapeStride *ss)
{
    DEFINE_MAP_DTYPE(DType)

    Map_DType rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    SET_DEFAULT_STRIDE(ss->stride_rd, ss->shape1_column);
    Map_DType rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    SHAPE_STRIDE_INFO(ss);
    if (GLOBAL_DBG) {
        
        cout << "vecos_m-rs1:" << endl << rs1_matrix << endl;
    }

    if( is_same< DType, half >::value )
    {
        half rs1;
        float16_t rs1_f16, rd_f16;
        for (int row = 0; row < rs1_matrix.rows(); row ++)
        for (int col = 0; col < rs1_matrix.cols(); col ++) {
            rs1_f16.v = rs1_matrix(row, col).x;
            rd_f16 = f16_cos( rs1_f16 );
            rd_matrix(row, col).x =  rd_f16.v;
        }
    }
    else if( is_same< DType, Bfloat16 >::value )
    {
        Bfloat16 rs1;
        bfloat16_t rs1_bf16, rd_bf16;
        for (int row = 0; row < rs1_matrix.rows(); row ++)
        for (int col = 0; col < rs1_matrix.cols(); col ++) {
            rs1_bf16.v = rs1_matrix(row, col).x;
            rd_bf16 = bf16_cos( rs1_bf16 );
            rd_matrix(row, col).x =  rd_bf16.v;
        }        
    }
    else if( is_same< DType, Float32 >::value )
    {
        Float32 rs1;
        float32_t rs1_f32, rd_f32;
        for (int row = 0; row < rs1_matrix.rows(); row ++)
        for (int col = 0; col < rs1_matrix.cols(); col ++) {
            rs1_f32.v = rs1_matrix(row, col).x;
            rd_f32 = f32_cos( rs1_f32 );
            rd_matrix(row, col).x =  rd_f32.v;
        }          
    }

    if (GLOBAL_DBG)
        cout << "vecos_m-rd:" << endl << rd_matrix << endl;

    return 0;
}

extern int vecvt_hf_x8_m(int8_t *rs1, half *rd, struct ShapeStride *ss);
extern int vecvt_hf_xu8_m(uint8_t *rs1, half *rd, struct ShapeStride *ss);
extern int vecvt_x8_hf_m(half *rs1, int8_t *rd, struct ShapeStride *ss, uint32_t rounding_mode);
extern int vecvt_xu8_hf_m(half *rs1, uint8_t *rd, struct ShapeStride *ss, uint32_t rounding_mode);
extern int vecvt_hf_x16_m(int16_t *rs1, half *rd, struct ShapeStride *ss);
extern int vecvt_x16_hf_m(half *rs1, int16_t *rd, struct ShapeStride *ss, uint32_t rounding_mode);
extern int vecvt_f32_hf_m(half *rs1, Float32 *rd, struct ShapeStride *ss);
extern int vecvt_hf_f32_m(Float32 *rs1, half *rd, struct ShapeStride *ss);
extern int vecvt_bf_x8_m(int8_t *rs1, Bfloat16 *rd, struct ShapeStride *ss);
extern int vecvt_bf_xu8_m(uint8_t *rs1, Bfloat16 *rd, struct ShapeStride *ss);
extern int vecvt_x8_bf_m(Bfloat16 *rs1, int8_t *rd, struct ShapeStride *ss, uint32_t rounding_mode);
extern int vecvt_xu8_bf_m(Bfloat16 *rs1, uint8_t *rd, struct ShapeStride *ss, uint32_t rounding_mode);
extern int vecvt_bf_x16_m(int16_t *rs1, Bfloat16 *rd, struct ShapeStride *ss);
extern int vecvt_x16_bf_m(Bfloat16 *rs1, int16_t *rd, struct ShapeStride *ss, uint32_t rounding_mode);
extern int vecvt_f32_bf_m(Bfloat16 *rs1, Float32 *rd, struct ShapeStride *ss);
extern int vecvt_bf_f32_m(Float32 *rs1, Bfloat16 *rd, struct ShapeStride *ss);
extern int vecvt_bf_hf_m(half *rs1, Bfloat16 *rd, struct ShapeStride *ss);
extern int vecvt_hf_bf_m(Bfloat16 *rs1, half *rd, struct ShapeStride *ss);
extern int vecvt_f32_x32_m(int32_t *rs1, Float32 *rd, struct ShapeStride *ss);
extern int vecvt_x32_f32_m(Float32 *rs1, int32_t *rd, struct ShapeStride *ss, uint32_t rounding_mode);
extern void dmae_mov(uint8_t* src, uint8_t *dst, uint32_t data_type, struct DmaeShapeStride *dmae_ss);
extern uint64_t dmae_src_len(uint32_t data_type, struct DmaeShapeStride *dmae_ss);
extern uint64_t dmae_dst_len(uint32_t data_type, struct DmaeShapeStride *dmae_ss);

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
    Bfloat16 int32_mul_bf16(int a, Bfloat16 b);
public:
    int debug;

    CustomInsns();

    int memul_mm(half *rs1, half *rs2, half *rd, struct ShapeStride *ss);
    int memul_mm(half *rs1, half *rs2, float32_t *rd, struct ShapeStride *ss);
    int memul_mm(Bfloat16 *rs1, Bfloat16 *rs2, Bfloat16 *rd, struct ShapeStride *ss);
    int memul_mm(Bfloat16 *rs1, Bfloat16 *rs2, Float32 *rd, struct ShapeStride *ss);
    int memul_mm(float32_t *rs1, float32_t *rs2, float32_t *rd, struct ShapeStride *ss);
    int memul_mm(int8_t *rs1, int8_t *rs2, half *rd, struct ShapeStride *ss, half *deq_addr=nullptr);
    int memul_mm(uint8_t *rs1, int8_t *rs2, half *rd, struct ShapeStride *ss, half *deq_addr=nullptr);
    int memul_mm(int8_t *rs1, int8_t *rs2, Bfloat16 *rd, struct ShapeStride *ss, Bfloat16 *deq_addr=nullptr);
    int memul_mm(uint8_t *rs1, int8_t *rs2, Bfloat16 *rd, struct ShapeStride *ss, Bfloat16 *deq_addr=nullptr);
    int memul_mm(half *rs1, int8_t *rs2, half *rd, struct ShapeStride *ss, bool isSign, half *deq_addr=nullptr);
    int memul_mm(Bfloat16 *rs1, int8_t *rs2, Bfloat16 *rd, struct ShapeStride *ss, bool isSign, Bfloat16 *deq_addr=nullptr);
    
    int memul_sp_mm(half *rs1, half *rs2, uint8_t *sparseidx, half *rd, struct ShapeStride *ss);
    int memul_sp_mm(half *rs1, int8_t *rs2, uint8_t *sparseidx, half *rd, struct ShapeStride *ss);
    int memul_sp_mm(int8_t *rs1, int8_t *rs2, uint8_t *sparseidx, half *rd, struct ShapeStride *ss);
    int memul_sp_mm(float32_t *rs1, float32_t *rs2, uint8_t *sparseidx, float32_t *rd, struct ShapeStride *ss);
    int memul_ts_mm(half *rs1, half *rs2, half *rd, struct ShapeStride *ss);
    int memul_ts_mm(half *rs1, int8_t *rs2, half *rd, struct ShapeStride *ss);
    int memul_ts_mm(int8_t *rs1, int8_t *rs2, half *rd, struct ShapeStride *ss);
    int memul_ts_mm(float32_t *rs1, float32_t *rs2, float32_t *rd, struct ShapeStride *ss);
    int memin_m(half *rs1, half *rd, struct ShapeStride *ss);
    int memax_m(half *rs1, half *rd, struct ShapeStride *ss);
    int meacc_m(half *rs1, half *rd, struct ShapeStride *ss);
    int memin_m(float32_t *rs1, float32_t *rd, struct ShapeStride *ss);
    int memax_m(float32_t *rs1, float32_t *rd, struct ShapeStride *ss);
    int meacc_m(float32_t *rs1, float32_t *rd, struct ShapeStride *ss);
    int memin_m(Bfloat16 *rs1, Bfloat16 *rd, struct ShapeStride *ss);
    int memax_m(Bfloat16 *rs1, Bfloat16 *rd, struct ShapeStride *ss);
    int meacc_m(Bfloat16 *rs1, Bfloat16 *rd, struct ShapeStride *ss);

    int veemul_x32_mf(int32_t *rs1, half *rd, half rs2, struct ShapeStride *ss);
    int veemul_x32_mv(int32_t *rs1, half *rd, half *rs2, struct ShapeStride *ss);

    int metr_m(half *rs1, half *rd, struct ShapeStride *ss);
    int metr_m(int8_t *rs1, int8_t *rd, struct ShapeStride *ss);
    int metr_m(float32_t *rs1, float32_t *rd, struct ShapeStride *ss);


    int meconv_mm(half *rs1, half *rd, half *rs2, struct ConvShapeStride *ss);
    int meconv_mm(half *rs1, half *rd, int8_t *rs2, struct ConvShapeStride *ss);
    int meconv_mm(int8_t *rs1, half *rd, int8_t *rs2, struct ConvShapeStride *ss);
    int meconv_mm(float32_t *rs1, float32_t *rd, float32_t *rs2, struct ConvShapeStride *ss);
    int meconv_sp_mm(half *rs1, half *rs2, uint8_t *sparseidx, half *rd, ConvShapeStride *ss);
    int meconv_sp_mm(half *rs1, int8_t *rs2, uint8_t *sparseidx, half *rd, ConvShapeStride *ss);
    int meconv_sp_mm(int8_t *rs1, int8_t *rs2, uint8_t *sparseidx, half *rd, ConvShapeStride *ss);
    int meconv_sp_mm(float32_t *rs1, float32_t *rs2, uint8_t *sparseidx, float32_t *rd, ConvShapeStride *ss);
    int medeconv_mm(half *rs1, half *rd, half *rs2, struct ConvShapeStride *ss);
    int medeconv_mm(half *rs1, int8_t *rd, half *rs2, struct ConvShapeStride *ss);
    int medeconv_mm(int8_t *rs1, int8_t *rd, half *rs2, struct ConvShapeStride *ss);
    int medeconv_mm(float32_t *rs1, float32_t *rd, float32_t *rs2, struct ConvShapeStride *ss);
    int medeconv_sp_mm(half *rs1, half *rs2, uint8_t *sparseidx, half *rd, struct ConvShapeStride *ss);
    int medeconv_sp_mm(half *rs1, int8_t *rs2, uint8_t *sparseidx, half *rd, ConvShapeStride *ss);
    int medeconv_sp_mm(int8_t *rs1, int8_t *rs2, uint8_t *sparseidx, half *rd, ConvShapeStride *ss);
    int medeconv_sp_mm(float32_t *rs1, float32_t *rs2, uint8_t *sparseidx, float32_t *rd, ConvShapeStride *ss);
    int medwconv_mm(half *rs1, half *rd, half *rs2, struct ConvShapeStride *ss);
    int medwconv_mm(half *rs1, half *rd, int8_t *rs2, struct ConvShapeStride *ss);
    int medwconv_mm(int8_t *rs1, half *rd, int8_t *rs2, struct ConvShapeStride *ss);
    int medwconv_mm(float32_t *rs1, float32_t *rd, float32_t *rs2, struct ConvShapeStride *ss);
    
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


