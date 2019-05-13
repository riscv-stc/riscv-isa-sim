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

using namespace Eigen;
using namespace std;

#define GLOBAL_DBG      1

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
};

/**
 * @brief 返回值枚举
 */
enum {
    BR_OK = 0,
    BR_EPARAM
};

/**
 * @brief custom扩展指令类
 * 
 * 包含了全部的custom矩阵扩展指令
 * 可以通过设置其实例的debug字段值来动态控制debug输出
 */
class CustomInsns
{
private:
    void shapestride_dbg(struct ShapeStride *ss);
public:
    int debug;

    CustomInsns();

    int vecvt_hf_x8_m(int8_t *rs1, half *rd, struct ShapeStride *ss);
    int vecvt_hf_xu8_m(uint8_t *rs1, half *rd, struct ShapeStride *ss);
    int vecvt_hf_x16_m(int16_t *rs1, half *rd, struct ShapeStride *ss);
    int vecvt_hf_xu16_m(uint16_t *rs1, half *rd, struct ShapeStride *ss);
    
    int veadd_mm(half *rs1, half *rd, half *rs2, struct ShapeStride *ss);
    int veadd_mv(half *rs1, half *rd, half *rs2, struct ShapeStride *ss, int dim);
    int veadd_mf(half *rs1, half *rd, half rs2, struct ShapeStride *ss);

    int vesub_mm(half *rs1, half *rd, half *rs2, struct ShapeStride *ss);
    int vesub_mv(half *rs1, half *rd, half *rs2, struct ShapeStride *ss, int dim);
    int vesub_mf(half *rs1, half *rd, half rs2, struct ShapeStride *ss);

    int veacc_m(half *rs1, half *rd, struct ShapeStride *ss);
    int veacc_m(half *rs1, half *rd, struct ShapeStride *ss, int dim);

    int vemul_mm(half *rs1, half *rs2, half *rd, struct ShapeStride *ss);
    int vemul_mv(half *rs1, half *rs2, half *rd, struct ShapeStride *ss);

    int veemul_mm(half *rs1, half *rd, half *rs2, struct ShapeStride *ss);
    int veemul_mv(half *rs1, half *rd, half *rs2, struct ShapeStride *ss, int dim);
    int veemul_mf(half *rs1, half *rd, half rs2, struct ShapeStride *ss);

    int veemacc_mm(half *rs1, half *rd, half *rs2, struct ShapeStride *ss);
    int veemacc_mm(half *rs1, half *rd, half *rs2, struct ShapeStride *ss, int dim);
    int veemacc_mv(half *rs1, half *rd, half *rs2, struct ShapeStride *ss, int dim);

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

    int vemv_m(half *rs1, half *rd, struct ShapeStride *ss);
};

/**
 * @brief 加宽浮点/整数类型转换指令
 * 
 * 实现整数或浮点数到两倍宽度的转换,九章处理器只支持int8/uint8 到 fp16的转换
 */
class Vfwcvt
{
  public:
    int debug;

    Vfwcvt();

    int vfwcvt_f_x_v(int8_t *vs2, half *vd, int num);
    int vfwcvt_f_xu_v(uint8_t *vs2, half *vd, int num);
};

/**
 * @brief 单宽度向量加法指令
 * 
 * 目的元素的宽度和源操作数中的元素宽度保持一致， 可以通过Type指定数据类型
 * 
 */
template <typename Type>
class Vadd
{
  public:
    int debug;
    typedef Map<Matrix<Type, 1, Dynamic>> VaddVecMap;
    
    Vadd(): debug(GLOBAL_DBG)
    {

    }

    /**
     * vadd_vf() vfadd.vf
     * @param vs2 源操作向量基地址
     * @param rs1 源标量操作数
     * @param vd 目的向量基地址
     * @param num 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vadd_vf(Type *vs2, Type rs1, Type *vd, int num)
    {
        VaddVecMap vector_vs2(vs2, num);
        VaddVecMap vector_vd(vd, num);
        
        vector_vd = vector_vs2.array() + rs1;

        if (debug) {
            cout << "vs2:" << endl << vector_vs2 << endl;
            cout << "rs1:" << endl << rs1 << endl;
            cout << "vd" << endl << vector_vd << endl;
        }

        return 0;
    }

    /**
     * vadd_vv() vfadd.vv
     * @param vs2 源操作向量二基地址
     * @param vs1 源操作向量一基地址
     * @param vd 目的向量基地址
     * @param num 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vadd_vv(Type *vs2, Type *vs1, Type *vd, int num)
    {
        VaddVecMap vector_vs2(vs2, num);
        VaddVecMap vector_vs1(vs1, num);
        VaddVecMap vector_vd(vd, num);
        
        vector_vd = vector_vs2.array() + vector_vs1.array();

        if (debug) {
            cout << "vs2:" << endl << vector_vs2 << endl;
            cout << "vs1:" << endl << vector_vs1 << endl;
            cout << "vd" << endl << vector_vd << endl;
        }
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
template <typename Type>
class Vsub
{
  public:
    int debug;

    Vsub(): debug(GLOBAL_DBG)
    {
    }

    typedef Map<Matrix<Type, 1, Dynamic>> VsubVecMap;
    
    /**
     * vsub_vf() vfsub.vf
     * @param vs2 源操作向量基地址
     * @param rs1 源标量操作数
     * @param vd 目的向量基地址
     * @param num 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vsub_vf(Type *vs2, Type rs1, Type *vd, int num)
    {
        VsubVecMap vector_vs2(vs2, num);
        VsubVecMap vector_vd(vd, num);
        
        vector_vd = vector_vs2.array() + -rs1;

        if (debug) {
            cout << "vs2:" << endl << vector_vs2 << endl;
            cout << "rs1:" << endl << rs1 << endl;
            cout << "vd" << endl << vector_vd << endl;
        }

        return 0;
    }

    /**
     * vsub_vv() vfsub.vv
     * @param vs2 源操作向量二基地址
     * @param vs1 源操作向量一基地址
     * @param vd 目的向量基地址
     * @param num 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vsub_vv(Type *vs2, Type *vs1, Type *vd, int num)
    {
        VsubVecMap vector_vs2(vs2, num);
        VsubVecMap vector_vs1(vs1, num);
        VsubVecMap vector_vd(vd, num);
        
        vector_vd = vector_vs2.array() + -vector_vs1.array();

        if (debug) {
            cout << "vs2:" << endl << vector_vs2 << endl;
            cout << "vs1:" << endl << vector_vs1 << endl;
            cout << "vd" << endl << vector_vd << endl;
        }

        return 0;
    }
};


/**
 * @brief 单宽度向量乘法指令
 * 
 * 目的元素的宽度和源操作数中的元素宽度保持一致， 可以通过Type指定数据类型
 */
template <typename Type>
class Vmul
{
  public:
    int debug;

    Vmul(): debug(GLOBAL_DBG)
    {

    }

    typedef Map<Matrix<Type, 1, Dynamic>> VmulVecMap;
    
    /**
     * vmul_vf() vfmul.vf
     * @param vs2 源操作向量基地址
     * @param rs1 源标量操作数
     * @param vd 目的向量基地址
     * @param num 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vmul_vf(Type *vs2, Type rs1, Type *vd, int num)
    {
        VmulVecMap vector_vs2(vs2, num);
        VmulVecMap vector_vd(vd, num);
        
        vector_vd = vector_vs2.array() * rs1;

        if (debug) {
            cout << "vs2:" << endl << vector_vs2 << endl;
            cout << "rs1:" << endl << rs1 << endl;
            cout << "vd" << endl << vector_vd << endl;
        }

        return 0;
    }

    /**
     * vmul_vv() vfmul.vv
     * @param vs2 源操作向量二基地址
     * @param vs1 源操作向量一基地址
     * @param vd 目的向量基地址
     * @param num 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vmul_vv(Type *vs2, Type *vs1, Type *vd, int num)
    {
        VmulVecMap vector_vs2(vs2, num);
        VmulVecMap vector_vs1(vs1, num);
        VmulVecMap vector_vd(vd, num);
        
        vector_vd = vector_vs2.array() * vector_vs1.array();

        if (debug) {
            cout << "vs2:" << endl << vector_vs2 << endl;
            cout << "vs1:" << endl << vector_vs1 << endl;
            cout << "vd" << endl << vector_vd << endl;
        }

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
 * TypeData 输入向量，输出向量，输入标量的数据类型
 * TypeMask mask向量的数据类型
 */
template <typename TypeData, typename TypeMask>
class Vmerge
{
  public:
    int debug;

    Vmerge(): debug(GLOBAL_DBG)
    {

    }

    typedef Map<Matrix<TypeData, 1, Dynamic>> VmergeDataVecMap;
    typedef Map<Matrix<TypeMask, 1, Dynamic>> VmergeMaskVecMap;

    /**
     * vmerge_vf() vfmerge.vf
     * @param vs2 源操作向量基地址
     * @param rs1 源标量操作数
     * @param vd 目的向量基地址
     * @param vm 不可屏蔽标识， vm=0 可屏蔽， vm=1不可屏蔽
     * @param v0 mask向量基地址
     * @param num 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vmerge_vf(TypeData *vs2, TypeData rs1, TypeData *vd, int vm, TypeMask *v0, int num)
    {
        VmergeDataVecMap vector_vs2(vs2, num);
        VmergeDataVecMap vector_vd(vd, num);
        VmergeMaskVecMap vector_v0(v0, num);
        Matrix<TypeMask, 1, Dynamic> new_v0(num);

        switch (vm) {
        case 0:
            for (int i = 0; i < num; i++)
                new_v0(i) = vector_v0(i) & 0x1;
            vector_vd = new_v0.array().select(rs1, vector_vs2);
            break;
        case 1:
            vector_vd = vector_vd.Constant(1, num, rs1);
            break;
        default:
            cout << "invalid vm" << endl;
            return -BR_EPARAM;
        }

        if (debug) {
            cout << "vs2:" << endl << vector_vs2 << endl;
            cout << "rs1:" << endl << rs1 << endl;
            cout << "vm:" << endl << vm << endl;
            cout << "v0:" << endl << new_v0 << endl;
            cout << "vd" << endl << vector_vd << endl;
        }

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
     * @param num 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vext_x_v(Type *vs2, Type *rd, uint16_t rs1, int num)
    {
        VextVecMap vector_vs2(vs2, num);
        
        if (rs1 >= num)
            *rd = 0;
        else
            *rd = vector_vs2(rs1);
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
template <typename Type>
class Vma
{
  public:
    int debug;

    Vma(): debug(GLOBAL_DBG)
    {

    }

    typedef Map<Matrix<Type, 1, Dynamic>> VmaVecMap;
    
    /**
     * vmacc_vf() vfmacc.vf
     * @param vs2 源操作向量基地址
     * @param rs1 源标量操作数
     * @param vd 目的向量基地址
     * @param num 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vmacc_vf(Type *vs2, Type rs1, Type *vd, int num)
    {
        VmaVecMap vector_vs2(vs2, num);
        VmaVecMap vector_vd(vd, num);

        vector_vd = vector_vs2.array() * rs1 + vector_vd.array();

        if (debug) {
            cout << "vs2:" << endl << vector_vs2 << endl;
            cout << "rs1:" << endl << rs1 << endl;
            cout << "vd" << endl << vector_vd << endl;
        }

        return 0;
    }

    /**
     * vmacc_vv() vfmacc.vv
     * @param vs2 源操作向量二基地址
     * @param vs1 源操作向量一基地址
     * @param vd 目的向量基地址
     * @param num 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vmacc_vv(Type *vs2, Type *vs1, Type *vd, int num)
    {
        VmaVecMap vector_vs2(vs2, num);
        VmaVecMap vector_vs1(vs1, num);
        VmaVecMap vector_vd(vd, num);

        vector_vd = vector_vs2.array() * vector_vs1.array() + vector_vd.array();

        if (debug) {
            cout << "vs2:" << endl << vector_vs2 << endl;
            cout << "vs1:" << endl << vector_vs1 << endl;
            cout << "vd" << endl << vector_vd << endl;
        }

        return 0;
    }

    /**
     * vnmacc_vf() vfnmacc.vf
     * 
     * vd[i] = -(f[rs1] * vs2[i]) - vd[i]
     * @param vs2 源操作向量基地址
     * @param rs1 源标量操作数
     * @param vd 目的向量基地址
     * @param num 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vnmacc_vf(Type *vs2, Type rs1, Type *vd, int num)
    {
        VmaVecMap vector_vs2(vs2, num);
        VmaVecMap vector_vd(vd, num);

        vector_vd = -(vector_vs2.array() * rs1) + -vector_vd.array();

        if (debug) {
            cout << "vs2:" << endl << vector_vs2 << endl;
            cout << "rs1:" << endl << rs1 << endl;
            cout << "vd" << endl << vector_vd << endl;
        }

        return 0;
    }

    /**
     * vnmacc_vv() vfnmacc.vv
     * 
     * vd[i] = -(vs1[i] * vs2[i]) - vd[i]
     * @param vs2 源操作向量二基地址
     * @param vs1 源操作向量一基地址
     * @param vd 目的向量基地址
     * @param num 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vnmacc_vv(Type *vs2, Type *vs1, Type *vd, int num)
    {
        VmaVecMap vector_vs2(vs2, num);
        VmaVecMap vector_vs1(vs1, num);
        VmaVecMap vector_vd(vd, num);

        vector_vd = -(vector_vs2.array() * vector_vs1.array()) + -vector_vd.array();

        if (debug) {
            cout << "vs2:" << endl << vector_vs2 << endl;
            cout << "vs1:" << endl << vector_vs1 << endl;
            cout << "vd" << endl << vector_vd << endl;
        }

        return 0;
    }

    /**
     * vmsac_vf() vfmsac.vf
     * 
     * vd[i] = +(f[rs1] * vs2[i]) - vd[i]
     * @param vs2 源操作向量基地址
     * @param rs1 源标量操作数
     * @param vd 目的向量基地址
     * @param num 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vmsac_vf(Type *vs2, Type rs1, Type *vd, int num)
    {
        VmaVecMap vector_vs2(vs2, num);
        VmaVecMap vector_vd(vd, num);

        vector_vd = vector_vs2.array() * rs1 + -vector_vd.array();

        if (debug) {
            cout << "vs2:" << endl << vector_vs2 << endl;
            cout << "rs1:" << endl << rs1 << endl;
            cout << "vd" << endl << vector_vd << endl;
        }

        return 0;
    }

    /**
     * vmsac_vv() vfmsac.vv
     * 
     * vd[i] = +(vs1[i] * vs2[i]) - vd[i]
     * @param vs2 源操作向量二基地址
     * @param vs1 源操作向量一基地址
     * @param vd 目的向量基地址
     * @param num 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vmsac_vv(Type *vs2, Type *vs1, Type *vd, int num)
    {
        VmaVecMap vector_vs2(vs2, num);
        VmaVecMap vector_vs1(vs1, num);
        VmaVecMap vector_vd(vd, num);

        vector_vd = vector_vs2.array() * vector_vs1.array() + -vector_vd.array();

        if (debug) {
            cout << "vs2:" << endl << vector_vs2 << endl;
            cout << "vs1:" << endl << vector_vs1 << endl;
            cout << "vd" << endl << vector_vd << endl;
        }

        return 0;
    }

    /**
     * vnmsac_vf() vfnmsac.vf
     * 
     * vd[i] = -(f[rs1] * vs2[i]) + vd[i]
     * @param vs2 源操作向量基地址
     * @param rs1 源标量操作数
     * @param vd 目的向量基地址
     * @param num 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vnmsac_vf(Type *vs2, Type rs1, Type *vd, int num)
    {
        VmaVecMap vector_vs2(vs2, num);
        VmaVecMap vector_vd(vd, num);

        vector_vd = -(vector_vs2.array() * rs1) + vector_vd.array();

        if (debug) {
            cout << "vs2:" << endl << vector_vs2 << endl;
            cout << "rs1:" << endl << rs1 << endl;
            cout << "vd" << endl << vector_vd << endl;
        }

        return 0;
    }

    /**
     * vnmsac_vv() vfnmsac.vv
     * 
     * d[i] = -(vs1[i] * vs2[i]) + vd[i]
     * @param vs2 源操作向量二基地址
     * @param vs1 源操作向量一基地址
     * @param vd 目的向量基地址
     * @param num 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vnmsac_vv(Type *vs2, Type *vs1, Type *vd, int num)
    {
        VmaVecMap vector_vs2(vs2, num);
        VmaVecMap vector_vs1(vs1, num);
        VmaVecMap vector_vd(vd, num);

        vector_vd = -(vector_vs2.array() * vector_vs1.array()) + vector_vd.array();
        
        if (debug) {
            cout << "vs2:" << endl << vector_vs2 << endl;
            cout << "vs1:" << endl << vector_vs1 << endl;
            cout << "vd" << endl << vector_vd << endl;
        }

        return 0;
    }

    /**
     * vmadd_vf() vfmadd.vf
     * 
     * vfmadd.vf vd, rs1, vs2, vm # vd[i] = +(vd[i] * f[rs1]) + vs2[i]
     * @param vs2 源操作向量基地址
     * @param rs1 源标量操作数
     * @param vd 目的向量基地址
     * @param num 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vmadd_vf(Type *vs2, Type rs1, Type *vd, int num)
    {
        VmaVecMap vector_vs2(vs2, num);
        VmaVecMap vector_vd(vd, num);

        vector_vd = vector_vd.array() * rs1 + vector_vs2.array();

        if (debug) {
            cout << "vs2:" << endl << vector_vs2 << endl;
            cout << "rs1:" << endl << rs1 << endl;
            cout << "vd" << endl << vector_vd << endl;
        }

        return 0;
    }

    /**
     * vmadd_vv() vfmadd.vv
     * 
     * vfmadd.vv vd, vs1, vs2, vm # vd[i] = +(vd[i] * vs1[i]) + vs2[i]
     * @param vs2 源操作向量二基地址
     * @param vs1 源操作向量一基地址
     * @param vd 目的向量基地址
     * @param num 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vmadd_vv(Type *vs2, Type *vs1, Type *vd, int num)
    {
        VmaVecMap vector_vs2(vs2, num);
        VmaVecMap vector_vs1(vs1, num);
        VmaVecMap vector_vd(vd, num);

        vector_vd = vector_vd.array() * vector_vs1.array() + vector_vs2.array();

        if (debug) {
            cout << "vs2:" << endl << vector_vs2 << endl;
            cout << "vs1:" << endl << vector_vs1 << endl;
            cout << "vd" << endl << vector_vd << endl;
        }

        return 0;
    }

    /**
     * vnmadd_vf() vfnmadd.vf
     * 
     * vfnmadd.vf vd, rs1, vs2, vm # vd[i] = -(vd[i] * f[rs1]) - vs2[i]
     * @param vs2 源操作向量基地址
     * @param rs1 源标量操作数
     * @param vd 目的向量基地址
     * @param num 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vnmadd_vf(Type *vs2, Type rs1, Type *vd, int num)
    {
        VmaVecMap vector_vs2(vs2, num);
        VmaVecMap vector_vd(vd, num);

        vector_vd = -(vector_vd.array() * rs1) + -vector_vs2.array();

        if (debug) {
            cout << "vs2:" << endl << vector_vs2 << endl;
            cout << "rs1:" << endl << rs1 << endl;
            cout << "vd" << endl << vector_vd << endl;
        }

        return 0;
    }

    /**
     * vnmadd_vv() vfnmadd.vv
     * 
     * vfnmadd.vv vd, vs1, vs2, vm # vd[i] = -(vd[i] * vs1[i]) - vs2[i]
     * @param vs2 源操作向量二基地址
     * @param vs1 源操作向量一基地址
     * @param vd 目的向量基地址
     * @param num 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vnmadd_vv(Type *vs2, Type *vs1, Type *vd, int num)
    {
        VmaVecMap vector_vs2(vs2, num);
        VmaVecMap vector_vs1(vs1, num);
        VmaVecMap vector_vd(vd, num);

        vector_vd = -(vector_vd.array() * vector_vs1.array()) + -vector_vs2.array();

        if (debug) {
            cout << "vs2:" << endl << vector_vs2 << endl;
            cout << "vs1:" << endl << vector_vs1 << endl;
            cout << "vd" << endl << vector_vd << endl;
        }

        return 0;
    }

    /**
     * vmsub_vf() vfmsub.vf
     * 
     * vfmsub.vf vd, rs1, vs2, vm # vd[i] = +(vd[i] * f[rs1]) - vs2[i]
     * @param vs2 源操作向量基地址
     * @param rs1 源标量操作数
     * @param vd 目的向量基地址
     * @param num 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vmsub_vf(Type *vs2, Type rs1, Type *vd, int num)
    {
        VmaVecMap vector_vs2(vs2, num);
        VmaVecMap vector_vd(vd, num);

        vector_vd = vector_vd.array() * rs1 + -vector_vs2.array();

        if (debug) {
            cout << "vs2:" << endl << vector_vs2 << endl;
            cout << "rs1:" << endl << rs1 << endl;
            cout << "vd" << endl << vector_vd << endl;
        }

        return 0;
    }

    /**
     * vmsub_vv() vfmsub.vv
     * 
     * vfmsub.vv vd, vs1, vs2, vm # vd[i] = +(vd[i] * vs1[i]) - vs2[i]
     * @param vs2 源操作向量二基地址
     * @param vs1 源操作向量一基地址
     * @param vd 目的向量基地址
     * @param num 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vmsub_vv(Type *vs2, Type *vs1, Type *vd, int num)
    {
        VmaVecMap vector_vs2(vs2, num);
        VmaVecMap vector_vs1(vs1, num);
        VmaVecMap vector_vd(vd, num);

        vector_vd = vector_vd.array() * vector_vs1.array() + -vector_vs2.array();

        if (debug) {
            cout << "vs2:" << endl << vector_vs2 << endl;
            cout << "vs1:" << endl << vector_vs1 << endl;
            cout << "vd" << endl << vector_vd << endl;
        }

        return 0;
    }

    /**
     * vnmsub_vf() vfnmsub.vf
     * 
     * vfnmsub.vf vd, rs1, vs2, vm # vd[i] = -(vd[i] * f[rs1]) + vs2[i]
     * @param vs2 源操作向量基地址
     * @param rs1 源标量操作数
     * @param vd 目的向量基地址
     * @param num 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vnmsub_vf(Type *vs2, Type rs1, Type *vd, int num)
    {
        VmaVecMap vector_vs2(vs2, num);
        VmaVecMap vector_vd(vd, num);

        vector_vd = -(vector_vd.array() * rs1) + vector_vs2.array();

        if (debug) {
            cout << "vs2:" << endl << vector_vs2 << endl;
            cout << "rs1:" << endl << rs1 << endl;
            cout << "vd" << endl << vector_vd << endl;
        }

        return 0;
    }

    /**
     * vnmsub_vv() vfnmsub.vv
     * 
     * vfnmsub.vv vd, vs1, vs2, vm # vd[i] = -(vd[i] * vs1[i]) + vs2[i]
     * @param vs2 源操作向量二基地址
     * @param vs1 源操作向量一基地址
     * @param vd 目的向量基地址
     * @param num 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vnmsub_vv(Type *vs2, Type *vs1, Type *vd, int num)
    {
        VmaVecMap vector_vs2(vs2, num);
        VmaVecMap vector_vs1(vs1, num);
        VmaVecMap vector_vd(vd, num);

        vector_vd = -(vector_vd.array() * vector_vs1.array()) + vector_vs2.array();

        if (debug) {
            cout << "vs2:" << endl << vector_vs2 << endl;
            cout << "vs1:" << endl << vector_vs1 << endl;
            cout << "vd" << endl << vector_vd << endl;
        }

        return 0;
    }

    /**
     * vmax_vf() vfmax.vf
     * @param vs2 源操作向量基地址
     * @param rs1 源标量操作数
     * @param vd 目的向量基地址
     * @param num 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vmax_vf(Type *vs2, Type rs1, Type *vd, int num)
    {
        VmaVecMap vector_vs2(vs2, num);
        VmaVecMap vector_vd(vd, num);

        vector_vd = (vector_vs2.array() > rs1).select(vector_vs2, rs1);

        if (debug) {
            cout << "vs2:" << endl << vector_vs2 << endl;
            cout << "rs1:" << endl << rs1 << endl;
            cout << "vd" << endl << vector_vd << endl;
        }

        return 0;
    }

    /**
     * vmax_vv() vfmax.vv
     * @param vs2 源操作向量二基地址
     * @param vs1 源操作向量一基地址
     * @param vd 目的向量基地址
     * @param num 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vmax_vv(Type *vs2, Type *vs1, Type *vd, int num)
    {
        VmaVecMap vector_vs2(vs2, num);
        VmaVecMap vector_vs1(vs1, num);
        VmaVecMap vector_vd(vd, num);

        vector_vd = (vector_vs2.array() > vector_vs1.array()).select(vector_vs2, vector_vs1);

        if (debug) {
            cout << "vs2:" << endl << vector_vs2 << endl;
            cout << "vs1:" << endl << vector_vs1 << endl;
            cout << "vd" << endl << vector_vd << endl;
        }

        return 0;
    }

    /**
     * vmin_vf() vfmin.vf
     * @param vs2 源操作向量基地址
     * @param rs1 源标量操作数
     * @param vd 目的向量基地址
     * @param num 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vmin_vf(Type *vs2, Type rs1, Type *vd, int num)
    {
        VmaVecMap vector_vs2(vs2, num);
        VmaVecMap vector_vd(vd, num);

        vector_vd = (vector_vs2.array() < rs1).select(vector_vs2, rs1);

        if (debug) {
            cout << "vs2:" << endl << vector_vs2 << endl;
            cout << "rs1:" << endl << rs1 << endl;
            cout << "vd" << endl << vector_vd << endl;
        }

        return 0;
    }

    /**
     * vmin_vv() vfmin.vv
     * @param vs2 源操作向量二基地址
     * @param vs1 源操作向量一基地址
     * @param vd 目的向量基地址
     * @param num 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vmin_vv(Type *vs2, Type *vs1, Type *vd, int num)
    {
        VmaVecMap vector_vs2(vs2, num);
        VmaVecMap vector_vs1(vs1, num);
        VmaVecMap vector_vd(vd, num);

        vector_vd = (vector_vs2.array() < vector_vs1.array()).select(vector_vs2, vector_vs1);

        if (debug) {
            cout << "vs2:" << endl << vector_vs2 << endl;
            cout << "vs1:" << endl << vector_vs1 << endl;
            cout << "vd" << endl << vector_vd << endl;
        }

        return 0;
    }
};

/**
 * @brief 向量浮点符号注入指令
 * 
 * 向量浮点符号注入(Sign-Injection)指令的运算结果的指数和尾数由第一个源操作数 vs2 提供
 */
template <typename Type>
class Vsgnj
{
  public:
    int debug;

    Vsgnj(): debug(GLOBAL_DBG)
    {

    }

    typedef Map<Matrix<Type, 1, Dynamic>> VsgnjVecMap;

    /**
     * vsgnj_vv() vfsgnj.vv
     * @param vs2 源操作向量二基地址
     * @param vs1 源操作向量一基地址
     * @param vd 目的向量基地址
     * @param num 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vsgnj_vv(Type *vs2, Type *vs1, Type *vd, int num)
    {
        VsgnjVecMap vector_vs2(vs2, num);
        VsgnjVecMap vector_vs1(vs1, num);
        VsgnjVecMap vector_vd(vd, num);

        vector_vd = (vector_vs1.array() > (Type)0).select(
            vector_vs2.array().abs(), -vector_vs2.array().abs());

        if (debug) {
            cout << "vs2:" << endl << vector_vs2 << endl;
            cout << "vs1:" << endl << vector_vs1 << endl;
            cout << "vd" << endl << vector_vd << endl;
        }

        return 0;
    }

    /**
     * vsgnj_vf() vfsgnj.vf
     * @param vs2 源操作向量基地址
     * @param rs1 源标量操作数
     * @param vd 目的向量基地址
     * @param num 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vsgnj_vf(Type *vs2, Type rs1, Type *vd, int num)
    {
        VsgnjVecMap vector_vs2(vs2, num);
        VsgnjVecMap vector_vd(vd, num);

        if (rs1 > (Type)0)
            vector_vd = vector_vs2.array().abs();
        else
            vector_vd = -vector_vs2.array().abs();
        
        if (debug) {
            cout << "vs2:" << endl << vector_vs2 << endl;
            cout << "rs1:" << endl << rs1 << endl;
            cout << "vd" << endl << vector_vd << endl;
        }

        return 0;
    }
    
    /**
     * vsgnjn_vv() vfsgnjn.vv
     * @param vs2 源操作向量二基地址
     * @param vs1 源操作向量一基地址
     * @param vd 目的向量基地址
     * @param num 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vsgnjn_vv(Type *vs2, Type *vs1, Type *vd, int num)
    {
        VsgnjVecMap vector_vs2(vs2, num);
        VsgnjVecMap vector_vs1(vs1, num);
        VsgnjVecMap vector_vd(vd, num);

        vector_vd = (vector_vs1.array() < (Type)0).select(
            vector_vs2.array().abs(), -vector_vs2.array().abs());

        if (debug) {
            cout << "vs2:" << endl << vector_vs2 << endl;
            cout << "vs1:" << endl << vector_vs1 << endl;
            cout << "vd" << endl << vector_vd << endl;
        }

        return 0;
    }

    /**
     * vsgnjn_vf() vfsgnjn.vf
     * @param vs2 源操作向量基地址
     * @param rs1 源标量操作数
     * @param vd 目的向量基地址
     * @param num 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vsgnjn_vf(Type *vs2, Type rs1, Type *vd, int num)
    {
        VsgnjVecMap vector_vs2(vs2, num);
        VsgnjVecMap vector_vd(vd, num);

        if (rs1 < (Type)0)
            vector_vd = vector_vs2.array().abs();
        else
            vector_vd = -vector_vs2.array().abs();

        if (debug) {
            cout << "vs2:" << endl << vector_vs2 << endl;
            cout << "rs1:" << endl << rs1 << endl;
            cout << "vd" << endl << vector_vd << endl;
        }

        return 0;
    }
    
    /**
     * vsgnjx_vv() vfsgnjx.vv
     * @param vs2 源操作向量二基地址
     * @param vs1 源操作向量一基地址
     * @param vd 目的向量基地址
     * @param num 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vsgnjx_vv(Type *vs2, Type *vs1, Type *vd, int num)
    {
        VsgnjVecMap vector_vs2(vs2, num);
        VsgnjVecMap vector_vs1(vs1, num);
        VsgnjVecMap vector_vd(vd, num);

        vector_vd = ((vector_vs1.array() * vector_vs2.array()) > (Type)0).select(
            vector_vs2.array().abs(), -vector_vs2.array().abs());

        if (debug) {
            cout << "vs2:" << endl << vector_vs2 << endl;
            cout << "vs1:" << endl << vector_vs1 << endl;
            cout << "vd" << endl << vector_vd << endl;
        }

        return 0;
    }

    /**
     * vsgnjx_vf() vfsgnjx.vf
     * @param vs2 源操作向量基地址
     * @param rs1 源标量操作数
     * @param vd 目的向量基地址
     * @param num 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vsgnjx_vf(Type *vs2, Type rs1, Type *vd, int num)
    {
        VsgnjVecMap vector_vs2(vs2, num);
        VsgnjVecMap vector_vd(vd, num);

        vector_vd = ((vector_vs2.array() * rs1) > (Type)0).select(
            vector_vs2.array().abs(), -vector_vs2.array().abs());
        
        if (debug) {
            cout << "vs2:" << endl << vector_vs2 << endl;
            cout << "rs1:" << endl << rs1 << endl;
            cout << "vd" << endl << vector_vd << endl;
        }

        return 0;
    }
};

/**
 * @brief 向量比较指令
 * 
 * 比较指令的作用通常是为了产生屏蔽向量的值。比较指令包括相等(==),不相等(!=),
 * 大于(>),小于(<),大于等于(>=),小于等于(<=)等类型
 */
template <typename InType, typename OutType>
class Vcompare
{
  public:
    int debug;

    Vcompare(): debug(GLOBAL_DBG)
    {

    }

    typedef Map<Matrix<InType, 1, Dynamic>> VcompareInVecMap;
    typedef Map<Matrix<OutType, 1, Dynamic>> VcompareOutVecMap;

    /**
     * veq_vf()   vfeq.vf vd, vs2, rs1, vm   Compare equal(==)
     * @param vs2 源操作向量基地址
     * @param rs1 源标量操作数
     * @param vd 目的向量基地址
     * @param num 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int veq_vf(InType *vs2, InType rs1, OutType *vd, int num)
    {
        VcompareInVecMap vector_vs2(vs2, num);
        VcompareOutVecMap vector_vd(vd, num);

        for (int i = 0; i < num; i++) {
            if (vector_vs2(i) == rs1)
                vector_vd(i) = (OutType)1;
            else
                vector_vd(i) = (OutType)0;
        }
        
        if (debug) {
            cout << "vs2:" << endl << vector_vs2 << endl;
            cout << "rs1:" << endl << rs1 << endl;
            cout << "vd" << endl << vector_vd << endl;
        }

        return 0;
    }

    /**
     * veq_vv() vfeq.vv vd, vs2, vs1, vm    Compare equal(==)
     * @param vs2 源操作向量二基地址
     * @param vs1 源操作向量一基地址
     * @param vd 目的向量基地址
     * @param num 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int veq_vv(InType *vs2, InType *vs1, OutType *vd, int num)
    {
        VcompareInVecMap vector_vs2(vs2, num);
        VcompareInVecMap vector_vs1(vs1, num);
        VcompareOutVecMap vector_vd(vd, num);

        for (int i = 0; i < num; i++) {
            if (vector_vs2(i) == vector_vs1(i))
                vector_vd(i) = (OutType)1;
            else
                vector_vd(i) = (OutType)0;
        }

        if (debug) {
            cout << "vs2:" << endl << vector_vs2 << endl;
            cout << "vs1:" << endl << vector_vs1 << endl;
            cout << "vd" << endl << vector_vd << endl;
        }

        return 0;
    }

    /**
     * vne_vf()   vfne.vf vd, vs2, rs1, vm  Compare not equal(!=)
     * @param vs2 源操作向量基地址
     * @param rs1 源标量操作数
     * @param vd 目的向量基地址
     * @param num 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vne_vf(InType *vs2, InType rs1, OutType *vd, int num)
    {
        VcompareInVecMap vector_vs2(vs2, num);
        VcompareOutVecMap vector_vd(vd, num);

        for (int i = 0; i < num; i++) {
            if (vector_vs2(i) != rs1)
                vector_vd(i) = (OutType)1;
            else
                vector_vd(i) = (OutType)0;
        }
        
        if (debug) {
            cout << "vs2:" << endl << vector_vs2 << endl;
            cout << "rs1:" << endl << rs1 << endl;
            cout << "vd" << endl << vector_vd << endl;
        }

        return 0;
    }

    /**
     * vne_vv()    vfne.vv vd, vs2, vs1, vm   Compare not equal(!=)
     * @param vs2 源操作向量二基地址
     * @param vs1 源操作向量一基地址
     * @param vd 目的向量基地址
     * @param num 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vne_vv(InType *vs2, InType *vs1, OutType *vd, int num)
    {
        VcompareInVecMap vector_vs2(vs2, num);
        VcompareInVecMap vector_vs1(vs1, num);
        VcompareOutVecMap vector_vd(vd, num);

        for (int i = 0; i < num; i++) {
            if (vector_vs2(i) != vector_vs1(i))
                vector_vd(i) = (OutType)1;
            else
                vector_vd(i) = (OutType)0;
        }

        if (debug) {
            cout << "vs2:" << endl << vector_vs2 << endl;
            cout << "vs1:" << endl << vector_vs1 << endl;
            cout << "vd" << endl << vector_vd << endl;
        }

        return 0;
    }

    /**
     * vlt_vf()   vflt.vf vd, vs2, rs1, vm  Compare less than(<)
     * @param vs2 源操作向量基地址
     * @param rs1 源标量操作数
     * @param vd 目的向量基地址
     * @param num 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vlt_vf(InType *vs2, InType rs1, OutType *vd, int num)
    {
        VcompareInVecMap vector_vs2(vs2, num);
        VcompareOutVecMap vector_vd(vd, num);

        for (int i = 0; i < num; i++) {
            if (vector_vs2(i) < rs1)
                vector_vd(i) = (OutType)1;
            else
                vector_vd(i) = (OutType)0;
        }
        
        if (debug) {
            cout << "vs2:" << endl << vector_vs2 << endl;
            cout << "rs1:" << endl << rs1 << endl;
            cout << "vd" << endl << vector_vd << endl;
        }

        return 0;
    }

    /**
     * vlt_vv()    vflt.vv vd, vs2, vs1, vm   Compare less than(<)
     * @param vs2 源操作向量二基地址
     * @param vs1 源操作向量一基地址
     * @param vd 目的向量基地址
     * @param num 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vlt_vv(InType *vs2, InType *vs1, OutType *vd, int num)
    {
        VcompareInVecMap vector_vs2(vs2, num);
        VcompareInVecMap vector_vs1(vs1, num);
        VcompareOutVecMap vector_vd(vd, num);

        for (int i = 0; i < num; i++) {
            if (vector_vs2(i) < vector_vs1(i))
                vector_vd(i) = (OutType)1;
            else
                vector_vd(i) = (OutType)0;
        }

        if (debug) {
            cout << "vs2:" << endl << vector_vs2 << endl;
            cout << "vs1:" << endl << vector_vs1 << endl;
            cout << "vd" << endl << vector_vd << endl;
        }

        return 0;
    }

    /**
     * vle_vf()   vfle.vf vd, vs2, rs1, vm   Compare less than or equal(<=)
     * @param vs2 源操作向量基地址
     * @param rs1 源标量操作数
     * @param vd 目的向量基地址
     * @param num 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vle_vf(InType *vs2, InType rs1, OutType *vd, int num)
    {
        VcompareInVecMap vector_vs2(vs2, num);
        VcompareOutVecMap vector_vd(vd, num);

        for (int i = 0; i < num; i++) {
            if (vector_vs2(i) <= rs1)
                vector_vd(i) = (OutType)1;
            else
                vector_vd(i) = (OutType)0;
        }
        
        if (debug) {
            cout << "vs2:" << endl << vector_vs2 << endl;
            cout << "rs1:" << endl << rs1 << endl;
            cout << "vd" << endl << vector_vd << endl;
        }

        return 0;
    }

    /**
     * vle_vv()    vfle.vv vd, vs2, vs1, vm   Compare less than or equal(<=)
     * @param vs2 源操作向量二基地址
     * @param vs1 源操作向量一基地址
     * @param vd 目的向量基地址
     * @param num 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vle_vv(InType *vs2, InType *vs1, OutType *vd, int num)
    {
        VcompareInVecMap vector_vs2(vs2, num);
        VcompareInVecMap vector_vs1(vs1, num);
        VcompareOutVecMap vector_vd(vd, num);

        for (int i = 0; i < num; i++) {
            if (vector_vs2(i) <= vector_vs1(i))
                vector_vd(i) = (OutType)1;
            else
                vector_vd(i) = (OutType)0;
        }

        if (debug) {
            cout << "vs2:" << endl << vector_vs2 << endl;
            cout << "vs1:" << endl << vector_vs1 << endl;
            cout << "vd" << endl << vector_vd << endl;
        }

        return 0;
    }

    /**
     * vgt_vf()   vfgt.vf vd, vs2, rs1, vm   Compare greater than(>)
     * @param vs2 源操作向量基地址
     * @param rs1 源标量操作数
     * @param vd 目的向量基地址
     * @param num 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vgt_vf(InType *vs2, InType rs1, OutType *vd, int num)
    {
        VcompareInVecMap vector_vs2(vs2, num);
        VcompareOutVecMap vector_vd(vd, num);

        for (int i = 0; i < num; i++) {
            if (vector_vs2(i) > rs1)
                vector_vd(i) = (OutType)1;
            else
                vector_vd(i) = (OutType)0;
        }
        
        if (debug) {
            cout << "vs2:" << endl << vector_vs2 << endl;
            cout << "rs1:" << endl << rs1 << endl;
            cout << "vd" << endl << vector_vd << endl;
        }

        return 0;
    }

    /**
     * vgt_vv()    vfgt.vv vd, vs2, vs1, vm   Compare greater than(>)
     * @param vs2 源操作向量二基地址
     * @param vs1 源操作向量一基地址
     * @param vd 目的向量基地址
     * @param num 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vgt_vv(InType *vs2, InType *vs1, OutType *vd, int num)
    {
        VcompareInVecMap vector_vs2(vs2, num);
        VcompareInVecMap vector_vs1(vs1, num);
        VcompareOutVecMap vector_vd(vd, num);

        for (int i = 0; i < num; i++) {
            if (vector_vs2(i) > vector_vs1(i))
                vector_vd(i) = (OutType)1;
            else
                vector_vd(i) = (OutType)0;
        }

        if (debug) {
            cout << "vs2:" << endl << vector_vs2 << endl;
            cout << "vs1:" << endl << vector_vs1 << endl;
            cout << "vd" << endl << vector_vd << endl;
        }

        return 0;
    }

    /**
     * vge_vf()   vfge.vf vd, vs2, rs1, vm   Compare greater than or equal (>=)
     * @param vs2 源操作向量基地址
     * @param rs1 源标量操作数
     * @param vd 目的向量基地址
     * @param num 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vge_vf(InType *vs2, InType rs1, OutType *vd, int num)
    {
        VcompareInVecMap vector_vs2(vs2, num);
        VcompareOutVecMap vector_vd(vd, num);

        for (int i = 0; i < num; i++) {
            if (vector_vs2(i) >= rs1)
                vector_vd(i) = (OutType)1;
            else
                vector_vd(i) = (OutType)0;
        }
        
        if (debug) {
            cout << "vs2:" << endl << vector_vs2 << endl;
            cout << "rs1:" << endl << rs1 << endl;
            cout << "vd" << endl << vector_vd << endl;
        }

        return 0;
    }

    /**
     * vge_vv()    vfge.vv vd, vs2, vs1, vm   Compare greater than or equal (>=)
     * @param vs2 源操作向量二基地址
     * @param vs1 源操作向量一基地址
     * @param vd 目的向量基地址
     * @param num 向量长度(准确的说应该是个数)
     * @return 执行结果
     */
    int vge_vv(InType *vs2, InType *vs1, OutType *vd, int num)
    {
        VcompareInVecMap vector_vs2(vs2, num);
        VcompareInVecMap vector_vs1(vs1, num);
        VcompareOutVecMap vector_vd(vd, num);

        for (int i = 0; i < num; i++) {
            if (vector_vs2(i) >= vector_vs1(i))
                vector_vd(i) = (OutType)1;
            else
                vector_vd(i) = (OutType)0;
        }

        if (debug) {
            cout << "vs2:" << endl << vector_vs2 << endl;
            cout << "vs1:" << endl << vector_vs1 << endl;
            cout << "vd" << endl << vector_vd << endl;
        }

        return 0;
    }
};

#endif
