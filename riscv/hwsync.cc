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
uint32_t mask_buf[16]= {'\0'};

hwsync_t::hwsync_t(size_t nprocs, size_t bank_id, char *hwsync_masks) : group_count(16) {
    if (hwsync_masks[0] != 0) {
        uint8_t index = 0;
        char *p = NULL;
        const char *delim = ",";

        pthread_mutexattr_t attrmutex_pld;
        pthread_mutexattr_t attrmutex_sync;
        pthread_condattr_t attrcond_pld;
        pthread_condattr_t attrcond_sync;

        shm_name = "HWSYNC";
        shm_size = 256;

        if (bank_id == 0) {
            munmap(shm_start, shm_size);
            shm_unlink(shm_name);
        }

        shm_id = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
        if (shm_id == -1)
            throw std::runtime_error("hwsync shmget failed");

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

        shm_ptr += sizeof(pthread_mutex_t);
        pmutex_sync = (pthread_mutex_t *)shm_ptr;

        shm_ptr += sizeof(pthread_mutex_t);
        pcond_pld = (pthread_cond_t *)shm_ptr;

        shm_ptr += sizeof(pthread_cond_t);
        pcond_sync = (pthread_cond_t *)shm_ptr;

        uint32_t mask = (1 << nprocs) - 1;
        if (bank_id == 0) {
            // reset group masks
            *req_sync = ~0;
            *req_pld = ~0;

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
        p = strtok(hwsync_masks, delim);
        if (!p) {
            masks[0] &= ~(mask << (bank_id * nprocs));
        } else {
            while (p) {
                masks[index++] = std::stoul(p, nullptr, 16);
                p = strtok(NULL, delim);
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
        *req_sync = ~0;
        pthread_cond_broadcast(pcond_sync);

        munmap(shm_start, shm_size);
        shm_unlink(shm_name);
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
            if ((*req_sync | masks[i]) == masks[i] && ~masks[i] != 0) {
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
            if ((*req_sync | masks[i]) == masks[i] && ~masks[i] != 0) {
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