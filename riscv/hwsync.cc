#include <iostream>
#include <assert.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "hwsync.h"
#include "processor.h"

// #define DEBUG

hwsync_t::hwsync_t(simif_t *sim, char *hwsync_masks, uint32_t hwsync_timer_num) 
    : sim(sim), group_count(16), core_num(32), hwsync_mem_size(512)
{
    hwsync_base_addr = new uint8_t[HWSYNC_SIZE];
    memset(hwsync_base_addr, 0, HWSYNC_SIZE);
    sync_masks = (uint32_t *)(hwsync_base_addr + GROUP_MASK_OFFSET);
    // group_done = (uint32_t *)(hwsync_base_addr + GROUP_DONE_OFFSET);
    group_valid = (uint32_t *)(hwsync_base_addr + GROUP_VALID_OFFSET);
    sync_status = (uint32_t *)(hwsync_base_addr + SYNC_STATUS_OFFSET);
    group_locks = new std::condition_variable_any[group_count];

    hwsync_ptr = hwsync_base_addr;

    /* 硬件上pld有专用的同步方法,和hs没有关联, 这里借用了hs的同步方法 */
    req_pld = new uint32_t(~0);

    hs_sync_timer_num = new uint32_t(0);
    if (0 == hwsync_timer_num)
        *hs_sync_timer_num = ~0;
    else
        *hs_sync_timer_num = hwsync_timer_num;

    hs_sync_timer_cnt = new uint32_t[core_num];
    memset(hs_sync_timer_cnt, 0, core_num);
    memset(sync_masks, MASK_INITIAL_VALUE, group_count * sizeof(uint32_t));

    // initial sync_masks register visa console
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
                sync_masks[index] = std::stoul(p, nullptr, 16);
                setBitValue(*group_valid, index, 1);
                index++;
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
    for (int i = 0; i < group_count; i++)
    {
        group_locks[i].notify_all();
    }
    /*TODO: when free those variables,there will be a error that
    the process is still load the variables.
    This problem should be solved.
    */
    // free_set(hwsync_base_addr);
    // free_set(group_locks);
    // free_set(req_pld);
    // free_set(hs_sync_timer_num);
    // free_set(hs_sync_timer_cnt);
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
    (*req_pld) &= (~(1 << core_id));
    if (((*req_pld) | (~coremap)) == ~coremap)
    {
        // all enter, clear enter requests
        *req_pld |= coremap;
        cond_pld.notify_all();
    }
    cond_pld.wait(lock, [&]
                  { return ((*req_pld) & (1 << core_id)) != 0; });

#ifdef DEBUG
    std::cout << "core" << core_id << ": end pld" << std::endl;
#endif

    return true;
}

void hwsync_t::pld_clr(uint32_t id)
{
    bool req_bit_clred = false;

    if (!sim->get_core_by_idxinsim(id)->is_pld_started()) {
        return ;
    }

    /* 等待执行pld的核进入 cond_pld.wait */
    while(sim->get_core_by_idxinsim(id)->is_pld_started()) {
        {
        std::unique_lock<std::mutex> lock(mutex_pld);
        req_bit_clred = (0 == (*req_pld & (1<<id)));
        }
        if (req_bit_clred) {
            break;
        } else {
            printf("npc%d wait pld clr step1 \n", id);
            usleep(1000);
        }
    }

    /* 释放 */
    {
        std::unique_lock<std::mutex> lock(mutex_pld);
        *req_pld |= (0x1 << id);
        cond_pld.notify_all();
    }

    /* 等待进入 PLD_FINISH 状态 */
    usleep(1);
    while(sim->get_core_by_idxinsim(id)->is_pld_started()) {
        printf("npc%d wait pld clr step2 \n", id);
        usleep(1000);
    }
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

    for (int i = 0; i < group_count; i++)
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
    if (unlikely(!bytes || addr + len >= size()))
        return false;

    uint8_t *now_addr = (uint8_t *)(hwsync_base_addr + addr);
    memcpy(bytes, now_addr, len);
    switch (addr)
    {
    // case GROUP_DONE_OFFSET:
    case GROUP_VALID_OFFSET:
        memset(bytes + 2, 0, 2); // set high 2 bytes as 0
        break;
    case HS_SW_SYNC_REQ_CLR_OFFSET:
        memset(bytes, 0, len);
        break;
    default:
        break;
    }

    return true;
}

bool hwsync_t::store(reg_t addr, size_t len, const uint8_t *bytes)
{
    if (unlikely(!bytes || addr + len >= size()))
        return false;

    uint8_t *now_addr = (uint8_t *)(hwsync_base_addr + addr);
    switch (addr)
    {
    case HS_SW_SYNC_REQ_CLR_OFFSET:
    {
        memcpy(hwsync_base_addr + addr, bytes, len);
        uint32_t data = *(uint32_t *)bytes;
        // printf("clr req data %x \n",data);
        if (getBitValue(data, 31) != 1) // sync clr disable
        {
            break;
        }

        for (int group_id = 0; group_id < 15; group_id++)
        {
            if (getBitValue(data, group_id) != 1)
            {
                continue;
            }

            if (0xffffffff == sync_masks[group_id]) {
                continue;
            }

            printf("sync clr grp%d \t", group_id);
            for (int i = 0 ; i < 32 ; i++) {
                if (0 != ((sync_masks[group_id]>>i)&0x01)) {
                    continue;
                }
                if (0 == i%8) {
                    printf("\n");
                }
                printf("%d:%x,%s ",i, sim->get_core_by_idxinsim(i)->get_state()->pc,
                    (sim->get_core_by_idxinsim(i)->is_suspend()) ? "||" : "..");
            }
            printf("\n");
            fflush(NULL);

            /* 清sync前, 确保已经发起sync的npc都处于 group_locks.wait(),而不是处于线程/函数上下文 */
            for (int i = 0 ; i < 32 ; i++) {
                if (0 == ((sync_masks[group_id]>>i)&0x01)) {
                    while(1) {
                        bool npc_sync_start = false;
                        bool sync_set_req  = false;
                        {
                        std::unique_lock<std::mutex> lock(mutex_sync);
                        npc_sync_start = sim->get_core_by_idxinsim(i)->is_sync_started();
                        sync_set_req = getBitValue(*sync_status, i);
                        }
                        if (npc_sync_start && !sync_set_req) {
                            usleep(1000);
                        } else {
                            break;
                        }
                    }
                }
            }
            *sync_status &= sync_masks[group_id];
            group_locks[group_id].notify_all();

            /*等待 执行sync的npc由SYNC_STARTED进入SYNC_FINISH状态 */
            usleep(1);
            for (int i = 0 ; i < 32 ; i++) {
                if (0 == ((sync_masks[group_id]>>i)&0x01)) {
                    while(1) {
                        if (sim->get_core_by_idxinsim(i)->is_sync_started()) {
                            usleep(1000);
                        } else {    /* SYNC_IDLE / SYNC_FINISH */
                            break;
                        }
                    }
                }
            }
        }
        break;
    }     
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