#ifndef __HWSYNC_H__
#define __HWSYNC_H__

#include <vector>
#include <mutex>
#include <condition_variable>
#include "decode.h"
#include <semaphore.h>
#include <pthread.h>

#include "devices.h"

class hwsync_t: public abstract_device_t {
  private:
    uint32_t *masks;
    uint32_t *req_sync;
    uint32_t *req_pld;
    int group_count;
    int shm_id;
    int shm_size;
    char *shm_ptr;
    char *shm_start;
    char * shm_name;
    size_t board_id;
    size_t chip_id;

    std::mutex mutex_sync;
    std::mutex mutex_pld;
    std::condition_variable_any cond_sync;
    std::condition_variable_any cond_pld;
    pthread_mutex_t * pmutex_pld;
    pthread_mutex_t * pmutex_sync;
    pthread_cond_t * pcond_pld;
    pthread_cond_t * pcond_sync;

  public:
    hwsync_t(size_t nprocs, size_t bank_id, char *hwsync_masks, size_t board_id, size_t chip_id);
    virtual ~hwsync_t();

    bool enter(unsigned core_id);
    bool enter(unsigned core_id, uint32_t coremap);

    bool load(reg_t addr, size_t len, uint8_t* bytes);
    bool store(reg_t addr, size_t len, const uint8_t* bytes);
    uint32_t get_hwsync() { return *req_sync;}
    void reset(uint32_t id) {
      *req_pld |= (0x1 << id);
      if (shm_start)
        pthread_cond_broadcast(pcond_pld);
      else
        cond_pld.notify_all();
    }
};

#endif // __HWSYNC_H__
