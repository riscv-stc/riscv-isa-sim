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
        core_num(32), hwsync_mem_size(512) {

    hwsync_shm_ptr = (char *)malloc(hwsync_mem_size);
    if (hwsync_shm_ptr == nullptr)
        throw std::runtime_error("hwsync mmap failed");
    memset(hwsync_shm_ptr,0,hwsync_mem_size);

    req_sync = (uint32_t *)hwsync_shm_ptr;
    hwsync_shm_ptr += 4;

    req_pld = (uint32_t *)hwsync_shm_ptr;
    hwsync_shm_ptr += 4;

    masks = (uint32_t *)hwsync_shm_ptr;
    hwsync_shm_ptr += 64;
#ifdef HWSYNC_USED_PMUTEX
    pmutex_pld = (pthread_mutex_t *)hwsync_shm_ptr;
    hwsync_shm_ptr += sizeof(pthread_mutex_t);     /* __SIZEOF_PTHREAD_MUTEX_T==40 */

    pmutex_pld = (pthread_mutex_t *)hwsync_shm_ptr;
    hwsync_shm_ptr += sizeof(pthread_mutex_t);     /* __SIZEOF_PTHREAD_MUTEX_T==40 */

    pmutex_sync = (pthread_mutex_t *)hwsync_shm_ptr;
    hwsync_shm_ptr += sizeof(pthread_mutex_t);

    pcond_pld = (pthread_cond_t *)hwsync_shm_ptr;
    hwsync_shm_ptr += sizeof(pthread_cond_t);     /* __SIZEOF_PTHREAD_COND_T==48 */

    pcond_sync = (pthread_cond_t *)hwsync_shm_ptr;
    hwsync_shm_ptr += sizeof(pthread_cond_t);
#endif
    hs_sync_timer_num = (decltype(hs_sync_timer_num))hwsync_shm_ptr;
    hwsync_shm_ptr += sizeof(*hs_sync_timer_num);

    hs_sync_timer_cnt = (decltype(hs_sync_timer_cnt))hwsync_shm_ptr;
    hwsync_shm_ptr += sizeof(*hs_sync_timer_cnt) * core_num;

    if (0 == hwsync_timer_num)
        *hs_sync_timer_num = ~0;
    else 
        *hs_sync_timer_num = hwsync_timer_num;

    for (int i = 0 ; i < core_num ; i++) {
        hs_sync_timer_cnt[i] = 0;
    }

    for (int i = 0; i < group_count; i++)
        masks[i] = ~0;

    *req_sync = ~0;
    *req_pld = ~0;

    if (hwsync_masks[0] != 0) {
        uint8_t index = 0;
        char *p = NULL;
        const char *delim = ",";
    #ifdef HWSYNC_USED_PMUTEX
        pthread_mutexattr_t attrmutex_pld;
        pthread_mutexattr_t attrmutex_sync;
        pthread_condattr_t attrcond_pld;
        pthread_condattr_t attrcond_sync;

        pthread_mutexattr_init(&attrmutex_pld);
        pthread_mutexattr_setpshared(&attrmutex_pld, PTHREAD_PROCESS_SHARED);
        pthread_mutex_init(pmutex_pld, &attrmutex_pld);

        pthread_mutexattr_init(&attrmutex_sync);
        pthread_mutexattr_setpshared(&attrmutex_sync, PTHREAD_PROCESS_SHARED);
        pthread_mutex_init(pmutex_sync, &attrmutex_sync);

        pthread_condattr_init(&attrcond_pld);
        pthread_condattr_setpshared(&attrcond_pld, PTHREAD_PROCESS_SHARED);
        pthread_cond_init(pcond_pld, &attrcond_pld);

        pthread_condattr_init(&attrcond_sync);
        pthread_condattr_setpshared(&attrcond_sync, PTHREAD_PROCESS_SHARED);
        pthread_cond_init(pcond_sync, &attrcond_sync);
    #endif
        // add all processors to group
        p = std::strtok(hwsync_masks, delim);
        if (p) {
            while (p) {
                masks[index++] = std::stoul(p, nullptr, 16);
                p = std::strtok(NULL, delim);
                if (index >= group_count)
                    break;
            }
        } else {
            throw std::runtime_error("hwsync_masks error");
        }
    } else {
        throw std::runtime_error("hwsync_masks error");
    }
}

hwsync_t::~hwsync_t() {
    *req_sync = ~0;
#ifdef HWSYNC_USED_PMUTEX
    pthread_cond_broadcast(pcond_sync);
#else
    cond_sync.notify_all();
#endif
}

bool
hwsync_t::enter(unsigned core_id) {
#ifdef DEBUG
    std::cout << "core" << core_id << ": start sync" << std::endl;
#endif
#ifdef HWSYNC_USED_PMUTEX
        pthread_mutex_lock(pmutex_sync);
        *req_sync &= ~(1 << core_id);
        for (int i = 0; i < group_count; i++) {
            if (((*req_sync | masks[i]) == masks[i]) && (~masks[i] != 0)) {
                // all enter, clear enter requests
                *req_sync |= ~masks[i];
                pthread_cond_broadcast(pcond_sync);
                break;
            }
        }
        pthread_mutex_unlock(pmutex_sync);

        pthread_mutex_lock(pmutex_sync);
        while ((*req_sync & 1 << core_id) == 0)
            pthread_cond_wait(pcond_sync, pmutex_sync);
        pthread_mutex_unlock(pmutex_sync);
#else
        std::unique_lock<std::mutex> lock(mutex_sync);
        *req_sync &= ~(1 << core_id);
        for (int i=0; i< group_count; i++) {
            if (((*req_sync | masks[i]) == masks[i]) && (~masks[i] != 0)) {
                // all enter, clear enter requests
                *req_sync |= ~masks[i];
                cond_sync.notify_all();
                break;
            }
        }

        cond_sync.wait(lock, [&]{ return (*req_sync & 1 << core_id) != 0; });
#endif

#ifdef DEBUG
    std::cout << "core" << core_id << ": end sync" << std::endl;
#endif
    hwsync_timer_clear(core_id);
    return true;
}

bool
hwsync_t::enter(unsigned core_id, uint32_t coremap) {
#ifdef DEBUG
    std::cout << "core" << core_id << ": start pld, coremap=" << coremap << std::endl;
#endif
#ifdef HWSYNC_USED_PMUTEX
        pthread_mutex_lock(pmutex_pld);
        *req_pld &= ~(1 << core_id);

        if ((*req_pld | ~coremap) == ~coremap) {
            // all enter, clear enter requests
            *req_pld |= coremap;
            pthread_cond_broadcast(pcond_pld);
        }
        pthread_mutex_unlock(pmutex_pld);

        pthread_mutex_lock(pmutex_pld);
        while ((*req_pld & 1 << core_id) == 0)
            pthread_cond_wait(pcond_pld, pmutex_pld);
        pthread_mutex_unlock(pmutex_pld);
#else
        std::unique_lock<std::mutex> lock(mutex_pld);
        *req_pld &= ~(1 << core_id);
        if ((*req_pld | ~coremap) == ~coremap) {
            // all enter, clear enter requests
            *req_pld |= coremap;
            cond_pld.notify_all();
        }
        cond_pld.wait(lock, [&]{ return (*req_pld & 1 << core_id) != 0; });
#endif
    
#ifdef DEBUG
    std::cout << "core" << core_id << ": end pld" << std::endl;
#endif

    return true;
}

bool hwsync_t::is_hs_group_sync(int coreid)
{
    int grp = 0;
    int coren = 0;

    for (grp = 0 ; grp < group_count; grp++) {
        if (0 == ((1<<coreid) & masks[grp])) {
            for (coren = 0; coren < 32 ; coren++) {
                if (0 == ((1<<coren) & (masks[grp] | *req_sync)))
                    return true;
            }
        }
    }

    return false;
}

/* sync是否完成， *req_sync值为全f 或 grp内所有核都sync判定sync完成 */
bool hwsync_t::is_hwsync_done(void)
{
    int i = 0;

    if (likely(0xffffffff == *req_sync)) {
        return true;
    } else {
        for (i = 0 ; i < group_count; i++) {
            if (*req_sync == masks[i])
                return true;
        }
    }

    return false;
}

/* grp内的计数器超过阈值判定为timeout */
bool hwsync_t::is_hwsync_timeout(int coreid)
{
    int grpn = 0;
    int coren = 0;

    if ((core_num<=coreid) || (NULL==hs_sync_timer_cnt))
        return false;

    for (grpn = 0 ; grpn < group_count; grpn++) {
        if (0 == ((1<<coreid) & masks[grpn])) {
            for (coren = 0; coren < core_num ; coren++) {
                if ((get_hwsync_timer_cnts(coren) > get_hwsync_timer_thresh()) &&
                    (0 == ((1<<coren) & (masks[grpn])))) {
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

    if ((core_num<=coreid) || (NULL==hs_sync_timer_cnt))
        return;
    
    tmp = (uint64_t)(hs_sync_timer_cnt[coreid]) + clks;
    if (0xffffffff < tmp)
        hs_sync_timer_cnt[coreid] = 0xffffffff;
    else
        hs_sync_timer_cnt[coreid] = tmp;
}

uint32_t hwsync_t::get_hwsync_timer_cnts(int coreid)
{
    if ((core_num<=coreid) || (NULL==hs_sync_timer_cnt))
        return 0;

    return hs_sync_timer_cnt[coreid];
}

void hwsync_t::hwsync_timer_clear(int coreid)
{
    if ((core_num<=coreid) || (NULL==hs_sync_timer_cnt))
        return;
    hs_sync_timer_cnt[coreid] = 0;
}

void hwsync_t::hwsync_clear(void)
{
    int i = 0;

    *req_sync = ~0;
#ifdef HWSYNC_USED_PMUTEX
    pthread_cond_broadcast(pcond_sync);
#else
    cond_sync.notify_all();
#endif

    for (i = 0 ; i < core_num ; i++) {
        hwsync_timer_clear(i);
    }
}

bool hwsync_t::load(reg_t addr, size_t len, uint8_t* bytes)
{
  if (unlikely(!bytes || (addr+len>size())))
    return false;

  *((uint32_t*)bytes) = masks[addr / 4];

  return true;
}

bool hwsync_t::store(reg_t addr, size_t len, const uint8_t* bytes)
{
  if (unlikely(!bytes || (addr+len>size())))
    return false;

  masks[addr / 4] = *((uint32_t*)bytes);

  return true;
}