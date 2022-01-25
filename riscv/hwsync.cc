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

uint32_t hw_sync = 0, hw_pld = 0;
uint32_t hw_sync_num = 0xffffffff;
uint32_t hw_sync_cnt[32] = {0};
uint32_t mask_buf[16]= {'\0'};

hwsync_t::hwsync_t(size_t nprocs, size_t bank_id, char *hwsync_masks, uint32_t hwsync_timer_num,size_t board_id, size_t chip_id, size_t session_id) : group_count(16), core_num(32), board_id(board_id), chip_id(chip_id), session_id(session_id) {
    if (hwsync_masks[0] != 0) {
        uint8_t index = 0;
        char *p = NULL;
        const char *delim = ",";

        pthread_mutexattr_t attrmutex_pld;
        pthread_mutexattr_t attrmutex_sync;
        pthread_condattr_t attrcond_pld;
        pthread_condattr_t attrcond_sync;

        shm_name = "HWSYNC";
        shm_size = 512;
        char file_name[64];
        char hwsync_name[64];
        sprintf(file_name, "/dev/shm/%s_%lu_%lu_%lu", shm_name, session_id, board_id, chip_id);
        sprintf(hwsync_name, "%s_%lu_%lu_%lu", shm_name, session_id, board_id, chip_id);

        if (bank_id == 0) {
            chmod(file_name, 0666);
            munmap(shm_start, shm_size);
            shm_unlink(hwsync_name);
        }

        shm_id = shm_open(hwsync_name, O_CREAT | O_RDWR, 0666);
        if (shm_id == -1)
            throw std::runtime_error("hwsync shmget failed");

        chmod(file_name, 0666);
        ftruncate(shm_id, shm_size);
        shm_ptr = (char *)mmap(0, shm_size, PROT_WRITE, MAP_SHARED, shm_id, 0);
        if (shm_ptr == (void *)-1)
            throw std::runtime_error("hwsync mmap failed");

        shm_start = shm_ptr;
        req_sync = (uint32_t *)shm_ptr;

        shm_ptr += 4;
        req_pld = (uint32_t *)shm_ptr;

        shm_ptr += 4;
        masks = (uint32_t *)shm_ptr;

        shm_ptr += 64;
        pmutex_pld = (pthread_mutex_t *)shm_ptr;

        shm_ptr += sizeof(pthread_mutex_t);     /* __SIZEOF_PTHREAD_MUTEX_T==40 */
        pmutex_sync = (pthread_mutex_t *)shm_ptr;

        shm_ptr += sizeof(pthread_mutex_t);
        pcond_pld = (pthread_cond_t *)shm_ptr;

        shm_ptr += sizeof(pthread_cond_t);     /* __SIZEOF_PTHREAD_COND_T==48 */
        pcond_sync = (pthread_cond_t *)shm_ptr;
        shm_ptr += sizeof(pthread_cond_t);

        hs_sync_timer_num = (decltype(hs_sync_timer_num))shm_ptr;
        shm_ptr += sizeof(*hs_sync_timer_num);

        hs_sync_timer_cnt = (decltype(hs_sync_timer_cnt))shm_ptr;
        shm_ptr += sizeof(*hs_sync_timer_cnt) * core_num;

        uint32_t mask = (1 << nprocs) - 1;
        if (bank_id == 0) {
            // reset group masks
            *req_sync = ~0;
            *req_pld = ~0;
            if (0 == hwsync_timer_num)
                *hs_sync_timer_num = ~0;
            else 
                *hs_sync_timer_num = hwsync_timer_num;

            for (int i = 0 ; i < core_num ; i++) {
                hs_sync_timer_cnt[i] = 0;
            }

            for (int i = 0; i < group_count; i++)
                masks[i] = ~0;

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
        }

        // add all processors to group
        p = std::strtok(hwsync_masks, delim);
        if (!p) {
            masks[0] &= ~(mask << (bank_id * nprocs));
        } else {
            while (p) {
                masks[index++] = std::stoul(p, nullptr, 16);
                p = std::strtok(NULL, delim);
                if (index >= group_count)
                    break;
            }
        }
    }
    else {
        shm_start = NULL;
        req_sync = &hw_sync;
        req_pld = &hw_pld;
        masks = mask_buf;
        hs_sync_timer_num = &hw_sync_num;
        hs_sync_timer_cnt = (decltype(hs_sync_timer_cnt))hw_sync_cnt;
        if (0 == hwsync_timer_num)
            *hs_sync_timer_num = ~0;
        else 
            *hs_sync_timer_num = hwsync_timer_num;
        
        for (int i = 0 ; i < core_num ; i++) {
            hs_sync_timer_cnt[i] = 0;
        }

        for (int i=0; i<group_count; i++) {
            masks[i] = ~0;
        }
        *req_sync = ~0;
        *req_pld = ~0;

        // add all processors to group 0
        uint32_t mask = (1 << nprocs) - 1;
        masks[0] = ~(mask << (nprocs * bank_id));
    }
}

hwsync_t::~hwsync_t() {
    if (shm_start) {
        char file_name[64];
        char hwsync_name[64];
        sprintf(file_name, "/dev/shm/%s_%lu_%lu_%lu", shm_name, session_id, board_id, chip_id);
        sprintf(hwsync_name, "%s_%lu_%lu_%lu", shm_name, session_id, board_id, chip_id);
        *req_sync = ~0;
        pthread_cond_broadcast(pcond_sync);
        chmod(file_name, 0666);
        munmap(shm_start, shm_size);
        shm_unlink(hwsync_name);
    } else {
        *req_sync = ~0;
        cond_sync.notify_all();
    }
}

bool
hwsync_t::enter(unsigned core_id) {
#ifdef DEBUG
    std::cout << "core" << core_id << ": start sync" << std::endl;
#endif
    if (shm_start) {
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
    }
    else
    {
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
    }

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
    if (shm_start) {
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
    }
    else
    {
        std::unique_lock<std::mutex> lock(mutex_pld);
        *req_pld &= ~(1 << core_id);
        if ((*req_pld | ~coremap) == ~coremap) {
            // all enter, clear enter requests
            *req_pld |= coremap;
            cond_pld.notify_all();
        }
        cond_pld.wait(lock, [&]{ return (*req_pld & 1 << core_id) != 0; });
    }
    
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
    if (shm_start)
        pthread_cond_broadcast(pcond_sync);
    else
        cond_sync.notify_all();

    for (i = 0 ; i < core_num ; i++) {
        hwsync_timer_clear(i);
    }
}

bool hwsync_t::load(reg_t addr, size_t len, uint8_t* bytes)
{
  if (unlikely(!bytes || addr >= 4 * 16))
    return false;

  *((uint32_t*)bytes) = masks[addr / 4];

  return true;
}

bool hwsync_t::store(reg_t addr, size_t len, const uint8_t* bytes)
{
  if (unlikely(!bytes || addr >= 4 * 16))
    return false;

  masks[addr / 4] = *((uint32_t*)bytes);

  return true;
}