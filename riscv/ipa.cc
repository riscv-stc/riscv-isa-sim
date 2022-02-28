// See LICENSE for license details.

#include <iostream>
#include <iomanip>
#include "ipa.h"

using namespace std;

#define IPA_DEBUG

ipa_t::ipa_t(const char *ipaini,int procid) : procid(procid)
{
    at_reg_base = (uint32_t *)new uint8_t[len];
    memset(at_reg_base, 0, len);
    
    int entryid = 0;
    struct ipa_at_t at = {};

    // entryid = 2;
    // at.pa_base = 0x00000000;
    // at.ipa_start = 0x00000000;
    // at.ipa_end = 0xc0000000;
    // reg_add_at(&at, entryid, at_reg_base);
    // reg_at_entry_enable(entryid, true, at_reg_base);

    // entryid = 6;
    // at.pa_base = 0xd9000000;
    // at.ipa_start = 0xd9000000;
    // at.ipa_end = 0xdb800000;
    // reg_add_at(&at, entryid, at_reg_base);
    // reg_at_entry_enable(entryid, true, at_reg_base);

    if (nullptr != ipaini) {
        atini = iniparser_load(ipaini);
    }
}

ipa_t::~ipa_t()
{
    if (atini)
        iniparser_freedict(atini);

    if (at_reg_base)
        delete at_reg_base;
}

/* 启用了ipa但地址不在映射范围内则报trap(0xc0000000的8MB空间除外) */
bool ipa_t:: pmp_ok(reg_t addr, reg_t len) const
{
    reg_t check_addr = addr;
    reg_t check_end = addr + len;

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

reg_t ipa_t::translate(reg_t addr, reg_t len) const
{
    reg_t paddr = 0;

    if (!is_ipa_enabled()) {
        return addr;
    }

    if (IS_NPC_LOCAL_REGION(addr)) {
        return addr;
    }

    for (int i = 0 ; i < (int)ipa_at.size() ; i++) {
        if ((addr>=ipa_at[i].ipa_start) && (addr<ipa_at[i].ipa_end)) {
            paddr = addr - ipa_at[i].ipa_start + ipa_at[i].pa_base;
        }
    }

    return paddr;
}

bool ipa_t::load(reg_t addr, size_t len, uint8_t* bytes)
{
    int offset = addr - NP_IOV_ATU_START;

    if ((nullptr==at_reg_base) || (nullptr==bytes)) {
        return false;
    }
    if ((offset<0) || (offset+len>=NP_IOV_ATU_SIZE)) {
        return false;
    }
    memcpy(bytes, (char *)at_reg_base + offset, len);
    return true;
}

bool ipa_t::store(reg_t addr, size_t len, const uint8_t* bytes)
{
    int offset = addr - NP_IOV_ATU_START;

    if ((nullptr==at_reg_base) || (nullptr==bytes)) {
        return false;
    }
    if ((offset<0) || (offset+len>=NP_IOV_ATU_SIZE)) {
        return false;
    }
    memcpy((char *)at_reg_base + offset, bytes, len);
    return true;
}

/* 根据寄存器配置更新 ipa vt 映射表, 成功返回0 */
int ipa_t::at_update(uint32_t *at_base)
{
    struct ipa_at_t at = {};

    if (nullptr == at_base) {
        return -1;
    }

    ipa_at.clear();

    if (!is_ipa_enabled()) {
        at_enabled = false;
        return 0;
    }
    at_enabled = true;

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

    return 0;
}

/* 根据配置文件更新 ipa vt 映射表 */
int ipa_t::at_update(dictionary *ini, int procid)
{
    int nkeys = 0;
    int nsec = 0;
    const char *secname = nullptr;
    string section_name = "";

    if (nullptr == ini)
        return -1;

    section_name.assign("ipa-trans-core");
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
                cerr << section_name << ":entry" << i
                    << "  pa.base: 0x" << hex << setw(10) << left << at.pa_base
                    << "  ipa.start: 0x" << hex << setw(10) << left << at.ipa_start
                    << "  ipa.end: 0x" << hex << setw(10) << left << at.ipa_end << endl;
                throw std::range_error("ipa range error");
            } else {
                #ifdef IPA_DEBUG
                cout << section_name << ":entry" << i
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
            throw std::invalid_argument("ipaini parse error");
        }
    }

    at_enabled = true;
    return 0;
}

int ipa_t::reset(void)
{
    if (atini) {
        at_update(atini,procid);
    } else {
        at_update(at_reg_base);
    }
    return 0;
}

/* 编辑寄存器区域添加 at 表项 */
int ipa_t::reg_add_at(struct ipa_at_t *at, int entry_id, uint32_t *at_base)
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

int ipa_t::reg_at_enable(bool enabled, uint32_t *at_base)
{
    if (nullptr == at_base)
        return -1;
    
    *(uint32_t *)((uint8_t *)at_base + AT_CTL_REG_OFFSET) = (enabled) ? 1 : 0;
}

int ipa_t::reg_at_entry_enable(int entry_id, bool enabled, uint32_t *at_base)
{
    if((nullptr==at_base) || (0>=entry_id) || (IPA_ENTRY_TOTAL<=entry_id)) {
        return -1;
    }

    if (enabled)
        *(uint32_t *)((uint8_t *)at_base + ENTRY_IPA_EN_OFFSET) |= (1<<entry_id);
    else
        *(uint32_t *)((uint8_t *)at_base + ENTRY_IPA_EN_OFFSET) &= ~(1<<entry_id);
}