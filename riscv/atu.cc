// See LICENSE for license details.

#include <iostream>
#include <iomanip>
#include "atu.h"

using namespace std;

#define IPA_DEBUG

/* npc atu， 每个proc包含一个,根据 procid 解析 ini 配置 */
atu_t::atu_t(const char *atuini,int procid, enum atu_type_t type) : procid(procid)
{
    switch(type) {
    case NP_ATU:
    case MTE_ATU:
        atu_type = type;
        break;
    case SYSDMA_ATU:
    default:
        throw std::runtime_error("error atu type");
    }
    at_reg_base = (uint8_t *)new uint8_t[len];
    memset(at_reg_base, 0, len);
    
    if (nullptr != atuini) {
        atini = iniparser_load(atuini);
    }
}

/**
 * 说明: sysdma atu，每个sysdma包含2个atu，通过sysdma id核ch id解析ini
 *      另外寄存器地址空间由外部传给构造函数
 * 参数: dma_id 0-7，4个bank共8个sysdma控制器
 * 参数: ch_id 0-1, 每个控制器2个channel
 */
atu_t::atu_t(const char *atuini, int dma_id, int ch_id, uint8_t *reg_base) : 
        at_reg_base((uint8_t *)reg_base), dma_id(dma_id), ch_id(ch_id)
{
    atu_type = SYSDMA_ATU;
    memset(at_reg_base, 0, len);
    
    if (nullptr != atuini) {
        atini = iniparser_load(atuini);
    }
}

atu_t::~atu_t()
{
    if (atini)
        iniparser_freedict(atini);

    switch(atu_type) {
    case NP_ATU:
    case MTE_ATU:
        if (at_reg_base) {
            delete at_reg_base;
        }
        break;
    case SYSDMA_ATU:
    default:
        break;
    }
}

/* 启用了ipa但地址不在映射范围内则报trap(0xc0000000的8MB空间除外) */
bool atu_t:: pmp_ok(reg_t addr, reg_t len) const
{
    reg_t check_addr = addr;
    reg_t check_end = addr + len - 1;

    if (!is_ipa_enabled()) {
        return true;
    }
    if (IS_NPC_LOCAL_REGION(check_addr)) {
        if (IS_NPC_LOCAL_REGION(check_end)) {
            return true;
        } else {
            check_addr = NPC_LOCAL_ADDR_START+NPC_LOCAL_REGIN_SIZE;
        }
    }

    for (int i = 0 ; i < (int)ipa_at.size() ; i++) {
        if ((check_addr>=ipa_at[i].ipa_start) && (check_end<ipa_at[i].ipa_end)) {
            return true;
        }
    }
    return false;
}

reg_t atu_t::translate(reg_t addr, reg_t len) const
{
    reg_t paddr = IPA_INVALID_ADDR;

    if (!is_ipa_enabled()) {
        return addr;
    }

    if (IS_NPC_LOCAL_REGION(addr)) {
        return addr;
    }

    for (int i = 0 ; i < (int)ipa_at.size() ; i++) {
        if ((addr>=ipa_at[i].ipa_start) && (addr<ipa_at[i].ipa_end)) {
            paddr = addr - ipa_at[i].ipa_start + ipa_at[i].pa_base;
            return paddr;
        }
    }

    return IPA_INVALID_ADDR;
}

bool atu_t::load(reg_t addr, size_t len, uint8_t* bytes)
{
    if ((nullptr==at_reg_base) || (nullptr==bytes)) {
        return false;
    }
    if (/* (addr<0) ||  */(addr+len>=size())) {
        return false;
    }
    memcpy(bytes, (char *)at_reg_base + addr, len);
    return true;
}

bool atu_t::store(reg_t addr, size_t len, const uint8_t* bytes)
{
    if ((nullptr==at_reg_base) || (nullptr==bytes)) {
        return false;
    }
    if (/* (addr<0) ||  */(addr+len>=size())) {
        return false;
    }
    memcpy((char *)at_reg_base + addr, bytes, len);
    return true;
}

/* 根据寄存器配置更新 ipa vt 映射表, 成功返回0 */
int atu_t::at_update(uint8_t *at_base)
{
    struct ipa_at_t at = {};

    if (nullptr == at_base) {
        return -1;
    }

    ipa_at.clear();

    if (!IS_AT_ENABLE(at_base)) {
        at_enabled = false;
        return 0;
    }

    for (int i = 0 ; i < ipa_entry_max ; i++) {
        if (!IS_ENTRY_ENABLE(at_base, i)) {
            continue;
        }
        memset(&at, 0 , sizeof(at));
        at.pa_base = PA_BASE_ADDR(at_base, i);
        at.ipa_start = IPA_START_ADDR(at_base, i);
        at.ipa_end = IPA_START_END(at_base, i);
        ipa_at.push_back(at);
    }
    at_enabled = true;

    return 0;
}

/* npc atu 根据配置文件更新 ipa vt 映射表 */
int atu_t::at_update(dictionary *ini, int procid)
{
    int nkeys = 0;
    int nsec = 0;
    const char *secname = nullptr;
    string section_name = "";

    if (nullptr == ini)
        return -1;

    switch(atu_type) {
    case NP_ATU:
        section_name.assign("ipa-trans-core");
        break;
    case MTE_ATU:
        section_name.assign("ipa-trans-mte-core");
        break;
    case SYSDMA_ATU:
    default:
        throw std::runtime_error("at_update() atu type error");
    }
    section_name.append(to_string(procid));

    nkeys = iniparser_getsecnkeys(ini, section_name.c_str());
    if (0 >= nkeys) {
        return -2;
    }

    ipa_at.clear();
    string keys_name;
    struct ipa_at_t at = {};
    for (int i = 0 ; i < ipa_entry_max; i++) {
        memset(&at, 0 , sizeof(at));
#define LONGINT_INVALID       ((reg_t)(0)-1)
        keys_name = section_name + ":entry";
        keys_name.append(to_string(i));
        keys_name.append(".pa.base");
        at.pa_base = iniparser_getlongint(ini, keys_name.c_str(), LONGINT_INVALID);

        keys_name = section_name + ":entry";
        keys_name.append(to_string(i));
        keys_name.append(".ipa.start");
        at.ipa_start = iniparser_getlongint(ini, keys_name.c_str(), LONGINT_INVALID);

        keys_name = section_name + ":entry";
        keys_name.append(to_string(i));
        keys_name.append(".ipa.end");
        at.ipa_end = iniparser_getlongint(ini, keys_name.c_str(), LONGINT_INVALID);

        if ((LONGINT_INVALID!=at.pa_base) && (LONGINT_INVALID!=at.ipa_start) &&(LONGINT_INVALID!=at.ipa_end)) {
            if ((at.ipa_end < at.ipa_start)) {
                cerr << section_name << ":entry" << dec << i
                    << "  pa.base: 0x" << hex << setw(10) << left << at.pa_base
                    << "  ipa.start: 0x" << hex << setw(10) << left << at.ipa_start
                    << "  ipa.end: 0x" << hex << setw(10) << left << at.ipa_end << endl;
                throw std::range_error("ipa range error");
            } else {
                #ifdef IPA_DEBUG
                cout << section_name << ":entry" << dec << i
                    << "  pa.base: 0x" << hex << setw(10) << left << at.pa_base
                    << "  ipa.start: 0x" << hex << setw(10) << left << at.ipa_start
                    << "  ipa.end: 0x" << hex << setw(10) << left << at.ipa_end << endl;
                #endif
                ipa_at.push_back(at);
            }
        } else if ((LONGINT_INVALID==at.pa_base) && (LONGINT_INVALID==at.ipa_start) && (LONGINT_INVALID==at.ipa_end)) {
            continue;
        } else {
            cerr << "ini [" << section_name << "]:entry" << i << " invalid" <<endl;
            throw std::invalid_argument("atuini parse error");
        }
    }

    at_enabled = true;
    return 0;
}

/* sysamd atu 根据配置文件更新 ipa vt 映射表 */
int atu_t::at_update(dictionary *ini, int dma_id, int ch_id)
{
    int nkeys = 0;
    int nsec = 0;
    const char *secname = nullptr;
    string section_name = "";

    if (nullptr == ini)
        return -1;

    section_name.assign("ipa-trans-sysdma");
    section_name.append(to_string(dma_id));
    section_name.append("-");
    section_name.append("ch");
    section_name.append(to_string(ch_id));

    nkeys = iniparser_getsecnkeys(ini, section_name.c_str());
    if (0 >= nkeys) {
        return -2;
    }

    ipa_at.clear();
    string keys_name;
    struct ipa_at_t at = {};
    for (int i = 0 ; i < ipa_entry_max; i++) {
        memset(&at, 0 , sizeof(at));
#define LONGINT_INVALID       ((reg_t)(0)-1)
        keys_name = section_name + ":entry";
        keys_name.append(to_string(i));
        keys_name.append(".pa.base");
        at.pa_base = iniparser_getlongint(ini, keys_name.c_str(), LONGINT_INVALID);

        keys_name = section_name + ":entry";
        keys_name.append(to_string(i));
        keys_name.append(".ipa.start");
        at.ipa_start = iniparser_getlongint(ini, keys_name.c_str(), LONGINT_INVALID);

        keys_name = section_name + ":entry";
        keys_name.append(to_string(i));
        keys_name.append(".ipa.end");
        at.ipa_end = iniparser_getlongint(ini, keys_name.c_str(), LONGINT_INVALID);

        if ((LONGINT_INVALID!=at.pa_base) && (LONGINT_INVALID!=at.ipa_start) &&(LONGINT_INVALID!=at.ipa_end)) {
            if ((at.ipa_end < at.ipa_start)) {
                cerr << section_name << ":entry" << dec << i
                    << "  pa.base: 0x" << hex << setw(10) << left << at.pa_base
                    << "  ipa.start: 0x" << hex << setw(10) << left << at.ipa_start
                    << "  ipa.end: 0x" << hex << setw(10) << left << at.ipa_end << endl;
                throw std::range_error("ipa range error");
            } else {
                #ifdef IPA_DEBUG
                cout << section_name << ":entry" << dec << i
                    << "  pa.base: 0x" << hex << setw(10) << left << at.pa_base
                    << "  ipa.start: 0x" << hex << setw(10) << left << at.ipa_start
                    << "  ipa.end: 0x" << hex << setw(10) << left << at.ipa_end << endl;
                #endif
                ipa_at.push_back(at);
            }
        } else if ((LONGINT_INVALID==at.pa_base) && (LONGINT_INVALID==at.ipa_start) && (LONGINT_INVALID==at.ipa_end)) {
            continue;
        } else {
            cerr << "ini [" << section_name << "]:entry" << i << " invalid" <<endl;
            throw std::invalid_argument("atuini parse error");
        }
    }

    at_enabled = true;
    return 0;
}

void atu_t::reset(void)
{
    if (atini) {
        switch(atu_type) {
        case NP_ATU:
        case MTE_ATU:
            at_update(atini,procid);
            break;
        case SYSDMA_ATU:
            at_update(atini, dma_id, ch_id);
            break;
        default:
            throw std::runtime_error("reset() atu type error");
        }
    } else {
        at_update(at_reg_base);
    }
}

/* 调试接口,编辑寄存器区域添加 at 表项. (at_update()后才能生效) */
int atu_t::reg_add_at(struct ipa_at_t *at, int entry_id, uint32_t *at_base)
{
    uint32_t *pa_l_addr = nullptr;
    uint32_t *pa_u_addr = nullptr;

    uint32_t *ipa_sl_addr = nullptr;
    uint32_t *ipa_su_addr = nullptr;

    uint32_t *ipa_el_addr = nullptr;
    uint32_t *ipa_eu_addr = nullptr;

    if((nullptr==at) || (nullptr==at_base) || (0>=entry_id) || (IPA_ENTRY_TOTAL<=entry_id)) {
        return -1;
    }

    pa_l_addr = (uint32_t *)((uint8_t *)at_base + entry_id*IPA_ENTRYX_SIZE + ENTRYX_PA_BASE_L_OFFSET);
    pa_u_addr = (uint32_t *)((uint8_t *)at_base + entry_id*IPA_ENTRYX_SIZE + ENTRYX_PA_BASE_U_OFFSET);
    ipa_sl_addr = (uint32_t *)((uint8_t *)at_base + entry_id*IPA_ENTRYX_SIZE + EXTRYX_IPA_START_L_OFFSET);
    ipa_su_addr = (uint32_t *)((uint8_t *)at_base + entry_id*IPA_ENTRYX_SIZE + EXTRYX_IPA_START_U_OFFSET);
    ipa_el_addr = (uint32_t *)((uint8_t *)at_base + entry_id*IPA_ENTRYX_SIZE + ENTRYX_IPA_END_L_OFFSET);
    ipa_eu_addr = (uint32_t *)((uint8_t *)at_base + entry_id*IPA_ENTRYX_SIZE + ENTRYX_IPA_END_U_OFFSET);

    
    *pa_l_addr = at->pa_base&0xffffffff;
    *pa_u_addr = (at->pa_base>>32)&0xff;

    *ipa_sl_addr = at->ipa_start&0xffffffff;
    *ipa_su_addr = (at->ipa_start>>32)&0xff;

    *ipa_el_addr = at->ipa_end&0xffffffff;
    *ipa_eu_addr = (at->ipa_end>>32)&0xff;

    return 0;
}

/* 调试接口,编辑寄存器区域 写 AT_CTL_REG_ADDR. (at_update()后才能生效) */
int atu_t::reg_at_enable(bool enabled, uint32_t *at_base)
{
    if (nullptr == at_base)
        return -1;
    
    *(uint32_t *)((uint8_t *)at_base + AT_CTL_REG_OFFSET) = (enabled) ? (1<<VFCFG_BIT_AT_EN) : 0;
    return 0;
}

/* 调试接口,编辑寄存器区域 写 ENTRY_IPA_EN_ADDR. (at_update()后才能生效) */
int atu_t::reg_at_entry_enable(int entry_id, bool enabled, uint32_t *at_base)
{
    if((nullptr==at_base) || (0>=entry_id) || (IPA_ENTRY_TOTAL<=entry_id)) {
        return -1;
    }

    if (enabled)
        *(uint32_t *)((uint8_t *)at_base + ENTRY_IPA_EN_OFFSET) |= (1<<entry_id);
    else
        *(uint32_t *)((uint8_t *)at_base + ENTRY_IPA_EN_OFFSET) &= ~(1<<entry_id);
    return 0;
}

idtu_t::idtu_t(void)
{
    memset(reg_base, 0, sizeof(reg_base));
}

idtu_t::~idtu_t(void)
{
    ;
}

void idtu_t::reset(void)
{
    memset(reg_base, 0, sizeof(reg_base));
}

bool idtu_t::load(reg_t addr, size_t len, uint8_t* bytes)
{
    if ((nullptr==reg_base) || (nullptr==bytes) || (addr+len>=size())) {
        return false;
    }

    memcpy(bytes, (char *)reg_base + addr, len);
    return true;
}

bool idtu_t::store(reg_t addr, size_t len, const uint8_t* bytes)
{
    if ((nullptr==reg_base) || (nullptr==bytes) || (addr+len>=size())) {
        return false;
    }

    switch(addr) {
    case IDTU_VF_CTRL:
        memcpy((char *)reg_base + addr, bytes, len);
        break;
    default:
        memcpy((char *)reg_base + addr, bytes, len);
        break;
    }
    return true;
}

uint32_t idtu_t::idtu_coreid_trans(uint32_t logi_core_id)
{
    uint32_t cnt = 0;
    uint32_t base = 0;
    uint32_t phy_core_id = 0;

    if (IS_IDTU_ENABLE(reg_base)) {
        cnt = (*(uint32_t*)(reg_base+IDTU_VF_CTRL) & VFCFG_VF_NR_CLUSTERS_MASK) >> VFCFG_BIT_NR_CLUSTERS;
        base = (*(uint32_t*)(reg_base+IDTU_VF_CTRL) & VFCFG_VF_PHY_CID_BASE_MASK) >> VFCFG_BIT_PHY_CID_BASE;

        phy_core_id = logi_core_id + base * 8;
    } else {
        phy_core_id = logi_core_id;
    }
    return phy_core_id;
}

uint32_t idtu_t::idtu_coremap_trans(uint32_t logi_coremap)
{
    uint32_t cnt = 0;
    uint32_t base = 0;
    uint32_t phy_coremap = 0;

    if (IS_IDTU_ENABLE(reg_base)) {
        cnt = (*(uint32_t*)(reg_base+IDTU_VF_CTRL) & VFCFG_VF_NR_CLUSTERS_MASK) >> VFCFG_BIT_NR_CLUSTERS;
        base = (*(uint32_t*)(reg_base+IDTU_VF_CTRL) & VFCFG_VF_PHY_CID_BASE_MASK) >> VFCFG_BIT_PHY_CID_BASE;

        phy_coremap = logi_coremap << (base * 8);
    } else {
        phy_coremap = logi_coremap;
    }
    return phy_coremap;
}