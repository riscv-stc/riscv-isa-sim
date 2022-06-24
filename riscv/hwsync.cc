#include <iostream>
#include <assert.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "hwsync.h"
#include "processor.h"

//#define DEBUG

hwsync_t::hwsync_t(char *hwsync_masks, uint32_t hwsync_timer_num) : group_count(16),
                                                                    core_num(32), hwsync_mem_size(512)
{
    hwsync_base_addr = new uint8_t[HWSYNC_SIZE];
    memset(hwsync_base_addr, 0, HWSYNC_SIZE);
    sync_masks = (uint32_t *)(hwsync_base_addr + GROUP_MASK_OFFSET);
    group_done = (uint32_t *)(hwsync_base_addr + GROUP_DONE_OFFSET);
    group_valid = (uint32_t *)(hwsync_base_addr + GROUP_VALID_OFFSET);
    sync_status = (uint32_t *)(hwsync_base_addr + SYNC_STATUS_OFFSET);
    group_locks = new std::condition_variable_any[group_count];

    hwsync_ptr = hwsync_base_addr;
    req_pld = new uint32_t(~0);

    hs_sync_timer_num = new uint32_t(0);
    if (0 == hwsync_timer_num)
        *hs_sync_timer_num = ~0;
    else
        *hs_sync_timer_num = hwsync_timer_num;

    hs_sync_timer_cnt = new uint32_t[core_num];
    memset(hs_sync_timer_cnt, 0, core_num);
    memset(sync_masks, MASK_INITIAL_VALUE, group_count * sizeof(uint32_t));

    if (hwsync_masks[0] != 0)
    {
        uint8_t index = 0;
        char *p = NULL;
        const char *delim = ",";
        // add all processors to group
        p = std::strtok(hwsync_masks, delim);
        if (p)
        {
            while (p)
            {
                sync_masks[index++] = std::stoul(p, nullptr, 16);
                p = std::strtok(NULL, delim);
                if (index >= group_count)
                    break;
            }
        }
        else
        {
            throw std::runtime_error("hwsync_masks error");
        }
    }
    else
    {
        throw std::runtime_error("hwsync_masks error");
    }
}

hwsync_t::~hwsync_t()
{
    *sync_status = 0;
    cond_sync.notify_all();
}

bool hwsync_t::enter(unsigned core_id)
{
#ifdef DEBUG
    std::cout << "core" << core_id << ": start sync" << std::endl;
#endif

    std::unique_lock<std::mutex> lock(mutex_sync);
    setBitValue(*sync_status, core_id, 1);
    for (int idx = 0; idx < group_count; idx++)
    {
        bool valid = getBitValue(*group_valid, idx);
        if (valid && (sync_masks[idx] != MASK_INITIAL_VALUE) && is_group_all_sync(idx))
        {
            // all enter, clear enter requests
            *sync_status &= sync_masks[idx];
            group_locks[idx].notify_all();
            break;
        }
    }

    int group_id = get_groupID_from_coreID(core_id);
    bool valid = getBitValue(*group_valid, group_id);
    // if the core is not assigned to any group or register GROUP_VALID is fasle,just jump
    if (group_id >= 0 && valid)
    {
        // if the the number <group_id> bit of register sync_status is 1,just wait,if it is 0,just pass
        group_locks[group_id].wait(lock, [&]
                                   { return getBitValue(*sync_status, core_id) == 0; });
    }

#ifdef DEBUG
    std::cout << "core" << core_id << ": end sync" << std::endl;
#endif
    hwsync_timer_clear(core_id);
    return true;
}

bool hwsync_t::enter(unsigned core_id, uint32_t coremap)
{
#ifdef DEBUG
    std::cout << "core" << core_id << ": start pld, coremap=" << coremap << std::endl;
#endif
    std::unique_lock<std::mutex> lock(mutex_pld);
    *req_pld &= ~(1 << core_id);
    if ((*req_pld | ~coremap) == ~coremap)
    {
        // all enter, clear enter requests
        *req_pld |= coremap;
        cond_pld.notify_all();
    }
    cond_pld.wait(lock, [&]
                  { return (*req_pld & 1 << core_id) != 0; });

#ifdef DEBUG
    std::cout << "core" << core_id << ": end pld" << std::endl;
#endif

    return true;
}

bool hwsync_t::is_group_all_sync(int group_id)
{
    if ((~(*sync_status) | sync_masks[group_id]) == sync_masks[group_id])
        return true;
    return false;
}

bool hwsync_t::is_hs_group_sync(int coreid)
{
    int grp = 0;
    int coren = 0;

    for (grp = 0; grp < group_count; grp++)
    {
        if (0 == ((1 << coreid) & sync_masks[grp]))
        {
            for (coren = 0; coren < 32; coren++)
            {
                if (0 == ((1 << coren) & (sync_masks[grp] | ~(*sync_status))))
                    return true;
            }
        }
    }

    return false;
}

/* grp内的计数器超过阈值判定为timeout */
bool hwsync_t::is_hwsync_timeout(int coreid)
{
    int grpn = 0;
    int coren = 0;

    if ((core_num <= coreid) || (NULL == hs_sync_timer_cnt))
        return false;

    for (grpn = 0; grpn < group_count; grpn++)
    {
        if (0 == ((1 << coreid) & sync_masks[grpn]))
        {
            for (coren = 0; coren < core_num; coren++)
            {
                if ((get_hwsync_timer_cnts(coren) > get_hwsync_timer_thresh()) &&
                    (0 == ((1 << coren) & (sync_masks[grpn]))))
                {
                    return true;
                }
            }
        }
    }

    return false;
}

void hwsync_t::hwsync_timer_cnts_add(int coreid, uint32_t clks)
{
    uint64_t tmp = 0;

    if ((core_num <= coreid) || (NULL == hs_sync_timer_cnt))
        return;

    tmp = (uint64_t)(hs_sync_timer_cnt[coreid]) + clks;
    if (0xffffffff < tmp)
        hs_sync_timer_cnt[coreid] = 0xffffffff;
    else
        hs_sync_timer_cnt[coreid] = tmp;
}

uint32_t hwsync_t::get_hwsync_timer_cnts(int coreid)
{
    if ((core_num <= coreid) || (NULL == hs_sync_timer_cnt))
        return 0;

    return hs_sync_timer_cnt[coreid];
}

void hwsync_t::hwsync_timer_clear(int coreid)
{
    if ((core_num <= coreid) || (NULL == hs_sync_timer_cnt))
        return;
    hs_sync_timer_cnt[coreid] = 0;
}

void hwsync_t::hwsync_clear(void)
{
    int i = 0;

    *sync_status = 0;
    cond_sync.notify_all();

    for(int i = 0; i < group_count; i++)
    {
        group_locks[i].notify_all();
    }

    for (i = 0; i < core_num; i++)
    {
        hwsync_timer_clear(i);
    }
}

bool hwsync_t::load(reg_t addr, size_t len, uint8_t *bytes)
{
    if (unlikely(!bytes || addr + len >= HWSYNC_SIZE))
        return false;

    uint8_t *now_addr = (uint8_t *)(hwsync_base_addr + addr);
    memcpy(bytes, now_addr, len);
    switch (addr)
    {
    // case GROUP_DONE_OFFSET:
    case GROUP_VALID_OFFSET:
        memset(bytes + 2, 0, 2); // set high 2 bytes as 0
        break;

    default:
        break;
    }

    return true;
}

bool hwsync_t::store(reg_t addr, size_t len, const uint8_t *bytes)
{
    if (unlikely(!bytes || addr + len >= HWSYNC_SIZE))
        return false;

    uint8_t *now_addr = (uint8_t *)(hwsync_base_addr + addr);
    switch (addr)
    {
    case SYNC_STATUS_OFFSET:
        break;
    // case GROUP_DONE_OFFSET:
    //     memcpy(hwsync_base_addr + addr, bytes, len > 2 ? 2 : len);
    //     break;
    case GROUP_VALID_OFFSET:
        memcpy(now_addr, bytes, len > 2 ? 2 : len);
        cpyBits(now_addr, hwsync_base_addr + GROUP_VALID_3_0, 0, 0, 4);
        cpyBits(now_addr, hwsync_base_addr + GROUP_VALID_7_4, 4, 0, 4);
        cpyBits(now_addr + 1, hwsync_base_addr + GROUP_VALID_11_8, 0, 0, 4);
        cpyBits(now_addr + 1, hwsync_base_addr + GROUP_VALID_15_11, 4, 0, 4);
        break;
    case GROUP_VALID_3_0:
        memcpy(hwsync_base_addr + addr, bytes, len);
        cpyBits(hwsync_base_addr + GROUP_VALID_3_0, now_addr, 0, 0, 4);
        break;
    case GROUP_VALID_7_4:
        memcpy(hwsync_base_addr + addr, bytes, len);
        cpyBits(hwsync_base_addr + GROUP_VALID_7_4, now_addr, 0, 4, 4);
        break;
    case GROUP_VALID_11_8:
        memcpy(hwsync_base_addr + addr, bytes, len);
        cpyBits(hwsync_base_addr + GROUP_VALID_11_8, now_addr + 1, 0, 0, 4);
        break;
    case GROUP_VALID_15_11:
        memcpy(hwsync_base_addr + addr, bytes, len);
        cpyBits(hwsync_base_addr + GROUP_VALID_15_11, now_addr + 1, 0, 4, 4);
        break;
    default:
        memcpy(hwsync_base_addr + addr, bytes, len);
        break;
    }

    return true;
}

uint32_t hwsync_t::get_groupID_from_coreID(uint32_t core_id)
{
    for (int i = 0; i < group_count; i++)
    {
        if (getBitValue(sync_masks[i], core_id) == 0)
            return i;
    }
    return -1;
}