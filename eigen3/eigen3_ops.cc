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
/* Matrix_uint16_t     Map_uint16_t */
MY_MATRIX_DEFINE(uint16_t)


#undef MY_MATRIX_DEFINE

/**
 * CustomInsns() 构造函数
 * 
 * 默认不开启debug
 */
CustomInsns::CustomInsns(): debug(1)
{
}

CustomInsns::~CustomInsns()
{
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
    if (!debug)
        return;

    printf("\nShapeStride:\n");
    printf("shape1: (%d:%d)\n", ss->shape1_row, ss->shape1_column);
    printf("shape2: (%d:%d)\n", ss->shape2_row, ss->shape2_column);
    printf("stride rs1: %d\n", ss->stride_rs1);
    printf("stride rs2: %d\n", ss->stride_rs2);
    printf("stride rd : %d\n\n", ss->stride_rd);
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
    if ((rs1 == rd) && (ss->stride_rs1 != ss->stride_rd)) {
        cout << __FUNCTION__ << ": when rs1 equal rs2, stride_rs1 must equal stride_rd" << endl;
        return -BR_EPARAM;
    }

    shapestride_dbg(ss);
    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    rd_matrix = rs1_matrix * rs2;

    if (debug) {
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rs2:" << endl << rs2 << endl;
        cout << "rd:" << endl << rd_matrix << endl;
    }

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
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    shapestride_dbg(ss);

    rd_matrix = rs1_matrix.array() * rs2_matrix.array();

    if (debug) {
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rs2:" << endl << rs2_matrix << endl;
        cout << "rd:" << endl << rd_matrix << endl;
    }

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
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));
    Map_half vector_dim1(rs2, ss->shape1_row, 1, DynStride(1, 1));
    Map_half vector_dim0(rs2, 1, ss->shape1_column, DynStride(1, 1));
    
    switch (dim) {
    case 0: // column dir
        for (int row = 0; row < rs1_matrix.rows(); row++)
            rd_matrix.row(row) = rs1_matrix.row(row).array() * vector_dim0.array();
        if (debug) {
            cout << "rs1:" << endl << rs1_matrix << endl;
            cout << "rs2:" << endl << vector_dim0 << endl;
            cout << "rd:" << endl << rd_matrix << endl;
        }
        break;
    case 1: // row dir
        for (int col = 0; col < rs1_matrix.cols(); col++)
            rd_matrix.col(col) = rs1_matrix.col(col).array() * vector_dim1.array();
        if (debug) {
            cout << "rs1:" << endl << rs1_matrix << endl;
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

    shapestride_dbg(ss);

    *rd = (rs1_matrix.array() * rs2_matrix.array()).sum();

    if (debug) {
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rs2:" << endl << rs2_matrix << endl;
    }

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
    Map_half vector_dim1(rd, ss->shape1_row, 1, DynStride(1, 1));
    Map_half vector_dim0(rd, 1, ss->shape1_column, DynStride(1, 1));
    
    shapestride_dbg(ss);

    if (debug) {
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
    Map_half vec_rd_dim1(rd, ss->shape1_row, 1, DynStride(1, 1));
    Map_half vec_rd_dim0(rd, 1, ss->shape1_column, DynStride(1, 1));
    Matrix_half rd_matrix(ss->shape1_row, ss->shape1_column);

    switch (dim) {
    case 0: // column dir
        for (int col = 0; col < rs1_matrix.cols(); col++)
            rd_matrix.col(col) = rs1_matrix.col(col).array() * vec_rs2_dim0.array();
        vec_rd_dim0 = rd_matrix.colwise().sum();

        if (debug) {
            cout << "rs1:" << endl << rs1_matrix << endl;
            cout << "rs2:" << endl << vec_rs2_dim0 << endl;
            cout << "mul:" << endl << rd_matrix << endl;
            cout << "rd:" << endl << vec_rd_dim0 << endl;
        }
        break;
    case 1: // row dir
        for (int row = 0; row < rs1_matrix.rows(); row++)
            rd_matrix.row(row) = rs1_matrix.row(row).array() * vec_rs2_dim1.array();
        vec_rd_dim1 = rd_matrix.rowwise().sum();
        if (debug) {
            cout << "rs1:" << endl << rs1_matrix << endl;
            cout << "rs2:" << endl << vec_rs2_dim1 << endl;
            cout << "mul:" << endl << rd_matrix << endl;
            cout << "rd:" << endl << vec_rd_dim1 << endl;
        }
        break;
    default:
        cout << __FUNCTION__ << "error dim" << endl;
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
    /* param check */
    if ((rs1 == rd) && (ss->stride_rs1 != ss->stride_rd)) {
        cout << __FUNCTION__ << ": when rs1 equal rs2, stride_rs1 must equal stride_rd" << endl;
        return -BR_EPARAM;
    }

    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    shapestride_dbg(ss);

    /* eigen not support matrix + scalar, so we creat a matrix init to const f, to
     * convert this operation to matrix + matrix
     */
    Matrix_half const_matrix(ss->shape1_row, ss->shape1_column);
    const_matrix = const_matrix.Constant(ss->shape1_row, ss->shape1_column, rs2);
    rd_matrix = rs1_matrix + const_matrix;

    if (debug) {
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rs2:" << endl << rs2 << endl;
        cout << "rd:" << endl << rd_matrix << endl;
    }

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
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    shapestride_dbg(ss);

    rd_matrix = rs1_matrix + rs2_matrix;

    if (debug) {
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rs2:" << endl << rs2_matrix << endl;
        cout << "rd:" << endl << rd_matrix << endl;
    }

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
    Map_half vector_dim1(rs2, ss->shape1_row, 1, DynStride(1, 1));
    Map_half vector_dim0(rs2, 1, ss->shape1_column, DynStride(1, 1));
    
    switch (dim) {
    case 0: // column dir
        for (int row = 0; row < rs1_matrix.rows(); row++)
            rd_matrix.row(row) = rs1_matrix.row(row).array() + vector_dim0.array();
        if (debug) {
            cout << "rs1:" << endl << rs1_matrix << endl;
            cout << "rs2:" << endl << vector_dim0 << endl;
            cout << "rd:" << endl << rd_matrix << endl;
        }
        break;
    case 1: // row dir
        for (int col = 0; col < rs1_matrix.cols(); col++)
            rd_matrix.col(col) = rs1_matrix.col(col).array() + vector_dim1.array();
        if (debug) {
            cout << "rs1:" << endl << rs1_matrix << endl;
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
    
    switch (dim) {
    case 0: // column dir
        rd_col_max = rs1_matrix.colwise().maxCoeff();
        if (debug) {
            cout << "rs1:" << endl << rs1_matrix << endl;
            cout << "rd:" << endl << rd_col_max << endl;
        }
        break;
    case 1: // row dir
        rd_row_max = rs1_matrix.rowwise().maxCoeff();
        if (debug) {
            cout << "rs1:" << endl << rs1_matrix << endl;
            cout << "rd:" << endl << rd_row_max << endl;
        }
        break;
    default:
        cout << __FUNCTION__ << "error dim" << endl;
        return -BR_EPARAM;
    }
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
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    shapestride_dbg(ss);

    rd_matrix = (rs1_matrix.array() > rs2_matrix.array()).select(rs1_matrix, rs2_matrix);

    if (debug) {
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rs2:" << endl << rs2_matrix << endl;
        cout << "rd:" << endl << rd_matrix << endl;
    }

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
    /* param check */
    if ((rs1 == rd) && (ss->stride_rs1 != ss->stride_rd)) {
        cout << __FUNCTION__ << ": when rs1 equal rs2, stride_rs1 must equal stride_rd" << endl;
        return -BR_EPARAM;
    }

    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    shapestride_dbg(ss);

    rd_matrix = (rs1_matrix.array() > rs2).select(rs1_matrix, rs2);

    if (debug) {
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rs2:" << endl << rs2 << endl;
        cout << "rd:" << endl << rd_matrix << endl;
    }

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
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));
    Map_half vector_dim1(rs2, ss->shape1_row, 1, DynStride(1, 1));
    Map_half vector_dim0(rs2, 1, ss->shape1_column, DynStride(1, 1));
    
    switch (dim) {
    case 0: // column dir
        for (int row = 0; row < rs1_matrix.rows(); row++)
            rd_matrix.row(row) = (rs1_matrix.row(row).array() > vector_dim0.array()).select(
                rs1_matrix.row(row), vector_dim0);
        if (debug) {
            cout << "rs1:" << endl << rs1_matrix << endl;
            cout << "rs2:" << endl << vector_dim0 << endl;
            cout << "rd:" << endl << rd_matrix << endl;
        }
        break;
    case 1: // row dir
        for (int col = 0; col < rs1_matrix.cols(); col++)
            rd_matrix.col(col) = (rs1_matrix.col(col).array() > vector_dim1.array()).select(
                rs1_matrix.col(col), vector_dim1);
        if (debug) {
            cout << "rs1:" << endl << rs1_matrix << endl;
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
    
    switch (dim) {
    case 0: // column dir
        rd_col_max = rs1_matrix.colwise().minCoeff();
        if (debug) {
            cout << "rs1:" << endl << rs1_matrix << endl;
            cout << "rd:" << endl << rd_col_max << endl;
        }
        break;
    case 1: // row dir
        rd_row_max = rs1_matrix.rowwise().minCoeff();
        if (debug) {
            cout << "rs1:" << endl << rs1_matrix << endl;
            cout << "rd:" << endl << rd_row_max << endl;
        }
        break;
    default:
        cout << __FUNCTION__ << "error dim" << endl;
        return -BR_EPARAM;
    }
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
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    shapestride_dbg(ss);

    rd_matrix = (rs1_matrix.array() < rs2_matrix.array()).select(rs1_matrix, rs2_matrix);

    if (debug) {
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rs2:" << endl << rs2_matrix << endl;
        cout << "rd:" << endl << rd_matrix << endl;
    }

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
    /* param check */
    if ((rs1 == rd) && (ss->stride_rs1 != ss->stride_rd)) {
        cout << __FUNCTION__ << ": when rs1 equal rs2, stride_rs1 must equal stride_rd" << endl;
        return -BR_EPARAM;
    }

    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    shapestride_dbg(ss);

    rd_matrix = (rs1_matrix.array() < rs2).select(rs1_matrix, rs2);

    if (debug) {
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rs2:" << endl << rs2 << endl;
        cout << "rd:" << endl << rd_matrix << endl;
    }

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
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));
    Map_half vector_dim1(rs2, ss->shape1_row, 1, DynStride(1, 1));
    Map_half vector_dim0(rs2, 1, ss->shape1_column, DynStride(1, 1));
    
    switch (dim) {
    case 0: // column dir
        for (int row = 0; row < rs1_matrix.rows(); row++)
            rd_matrix.row(row) = (rs1_matrix.row(row).array() < vector_dim0.array()).select(
                rs1_matrix.row(row), vector_dim0);
        if (debug) {
            cout << "rs1:" << endl << rs1_matrix << endl;
            cout << "rs2:" << endl << vector_dim0 << endl;
            cout << "rd:" << endl << rd_matrix << endl;
        }
        break;
    case 1: // row dir
        for (int col = 0; col < rs1_matrix.cols(); col++)
            rd_matrix.col(col) = (rs1_matrix.col(col).array() < vector_dim1.array()).select(
                rs1_matrix.col(col), vector_dim1);
        if (debug) {
            cout << "rs1:" << endl << rs1_matrix << endl;
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
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));
    shapestride_dbg(ss);

    /* our half not support operator - (const half& a, const half& b),
     * but can support operator - (const half& a),
     *  so we use (a + -b) to instead
     */
    rd_matrix = rs1_matrix + -rs2_matrix;

    if (debug) {
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rs2:" << endl << rs2_matrix << endl;
        cout << "rd:" << endl << rd_matrix << endl;
    }

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
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));
    Map_half vector_dim1(rs2, ss->shape1_row, 1, DynStride(1, 1));
    Map_half vector_dim0(rs2, 1, ss->shape1_column, DynStride(1, 1));

    /* our half not support operator - (const half& a, const half& b),
     * but can support operator - (const half& a),
     *  so we use (a + -b) to instead
     */
    switch (dim) {
    case 0: // column dir
        for (int row = 0; row < rs1_matrix.rows(); row++)
            rd_matrix.row(row) = rs1_matrix.row(row).array() + -vector_dim0.array();
        if (debug) {
            cout << "rs1:" << endl << rs1_matrix << endl;
            cout << "rs2:" << endl << vector_dim0 << endl;
            cout << "rd:" << endl << rd_matrix << endl;
        }
        break;
    case 1: // row dir
        for (int col = 0; col < rs1_matrix.cols(); col++)
            rd_matrix.col(col) = rs1_matrix.col(col).array() + -vector_dim1.array();
        if (debug) {
            cout << "rs1:" << endl << rs1_matrix << endl;
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
    /* param check */
    if ((rs1 == rd) && (ss->stride_rs1 != ss->stride_rd)) {
        cout << __FUNCTION__ << ": when rs1 equal rs2, stride_rs1 must equal stride_rd" << endl;
        return -BR_EPARAM;
    }

    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    shapestride_dbg(ss);

    /* eigen not support matrix + scalar, so we creat a matrix init to const f, to
     * convert this operation to matrix + matrix
     * Our half not support operator - (const half& a, const half& b),
     * but can support operator - (const half& a),
     *  so we use (a + -b) to instead
     */
    Matrix_half const_matrix(ss->shape1_row, ss->shape1_column);
    const_matrix = const_matrix.Constant(ss->shape1_row, ss->shape1_column, -rs2);
    rd_matrix = rs1_matrix + const_matrix;

    if (debug) {
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rs2:" << endl << rs2 << endl;
        cout << "rd:" << endl << rd_matrix << endl;
    }

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
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape2_column, DynStride(ss->stride_rd, 1));

    shapestride_dbg(ss);

    /* dot only support vector not support matrix, so we use '*' to do calculation */
    rd_matrix = rs1_matrix * rs2_matrix;
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
    /* param check */
    if (ss->shape2_column != 1 && ss->shape2_row != 1) {
        cout << __FUNCTION__ << ": rs2 need a vector" << endl;
        return -BR_EPARAM;
    }

    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    Map_half rs2_vector(rs2, ss->shape2_row, ss->shape2_column, DynStride(ss->stride_rs2, 1));
    Map_half rd_vector(rd, ss->shape1_row, ss->shape2_column, DynStride(ss->stride_rd, 1));

    shapestride_dbg(ss);
    
    /* dot only support vector not support matrix, so we use '*' to do calculation */
    rd_vector = rs1_matrix * rs2_vector;
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
    Map_half rd_row_sum(rd, ss->shape1_row, 1, DynStride(1, 1));
    
    switch (dim) {
    case 0: // column dir
        rd_col_sum = rs1_matrix.colwise().sum();
        break;
    case 1: // row dir
        rd_row_sum = rs1_matrix.rowwise().sum();
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

    *rd = rs1_matrix.sum();
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
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));
    
    rd_matrix = (rs1_matrix.array() > (half)0).select(rs1_matrix, rs1_matrix * rs2);
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
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));
    Map_half vector_dim1(rs2, ss->shape1_row, 1, DynStride(1, 1));
    Map_half vector_dim0(rs2, 1, ss->shape1_column, DynStride(1, 1));

    switch (dim) {
    case 0: // column dir
        for (int row = 0; row < rs1_matrix.rows(); row++)
            rd_matrix.row(row) = (rs1_matrix.row(row).array() > (half)0).select(
                rs1_matrix.row(row), 
                rs1_matrix.row(row).array() * vector_dim0.array());
        if (debug) {
            cout << "rs1:" << endl << rs1_matrix << endl;
            cout << "rs2:" << endl << vector_dim0 << endl;
            cout << "rd:" << endl << rd_matrix << endl;
        }
        break;
    case 1: // row dir
        for (int col = 0; col < rs1_matrix.cols(); col++)
            rd_matrix.col(col) = (rs1_matrix.col(col).array() > (half)0).select(
                rs1_matrix.col(col),
                rs1_matrix.col(col).array() * vector_dim1.array());
        if (debug) {
            cout << "rs1:" << endl << rs1_matrix << endl;
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
int CustomInsns::velut_m(uint16_t *rs1, uint64_t rs2, half *rd, struct ShapeStride *ss)
{
    Map_uint16_t rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    //rs2 = rs2 & 0x3ff;
    for (int i = 0; i < ss->shape1_row; i++)
        for (int j = 0; j < ss->shape1_column; j++)
            rd_matrix(i, j) = *(half *)(rs2 + rs1_matrix(i, j));
    
    if (debug) {
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rd:" << endl << rd_matrix << endl;
    }

    return 0;
}

/**
 * vemv_m() vemv.m
 * 
 * 将矩阵从一个地方搬移到另一个地方
 * @param rs1 M1,源操作矩阵基地址
 * @param rd V,目的矩阵基地址
 * @param ss 矩阵形状描述
 * @return 执行结果
 */
int CustomInsns::vemv_m(half *rs1, half *rd, struct ShapeStride *ss)
{
    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rs1, 1));
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));

    shapestride_dbg(ss);
    rd_matrix = rs1_matrix;

    if (debug) {
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rd:" << endl << rd_matrix << endl;
    }

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
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape1_column, DynStride(ss->stride_rd, 1));
    
    shapestride_dbg(ss);
    rd_matrix = rs1_matrix.cast<half>();

    if (debug) {
        cout << "rs1:" << endl << rs1_matrix << endl;
        cout << "rd:" << endl << rd_matrix << endl;
    }

    return 0;
}

/**
 * vfwcvt_f_xu_v() vfwcvt.f.xu.v
 * 
 * convert uinsigned integer to fp16 (uint8 -> fp16)
 * @param vs2 源操作向量基地址
 * @param vd 目的向量基地址
 * @param num 向量长度(准确的说应该是个数)
 * @return 执行结果
 */
int Vfwcvt::vfwcvt_f_xu_v(uint8_t *vs2, half *vd, int num)
{
    typedef Map<Matrix<uint8_t, 1, Dynamic>> Uint8VecMap;
    typedef Map<Matrix<half, 1, Dynamic>> HalfVecMap;
 
    Uint8VecMap vector_vs2(vs2, num);
    HalfVecMap vector_vd(vd, num);
        
    vector_vd = vector_vs2.cast<half>();
    return 0;
}

/**
 * vfwcvt_f_x_v() vfwcvt.f.x.v
 * 
 * convert signed integer to fp16 (int8 -> fp16)
 * @param vs2 源操作向量基地址
 * @param vd 目的向量基地址
 * @param num 向量长度(准确的说应该是个数)
 * @return 执行结果
 */
int vfwcvt_f_x_v(int8_t *vs2, half *vd, int num)
{
    return 0;
}

