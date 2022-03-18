#ifndef __HWSYNC_H__
#define __HWSYNC_H__

#include <vector>
#include <mutex>
#include <condition_variable>
#include "decode.h"
#include <semaphore.h>
#include <pthread.h>

#include "devices.h"

//#define HWSYNC_USED_PMUTEX

class hwsync_t: public abstract_device_t {
  private:
    uint32_t *masks;      /* masks[0]-masks[15]共16个sync组, 为0的bit对应的核在同一个sync组， 默认值{0xffffffff} */
    uint32_t *req_sync;   /* core_n开始sync时第n位清零，sync结束重新置位, 初始值0xffffffff */
    uint32_t *req_pld;
    int group_count;  /* 支持16个sync组 */
    int core_num;     /* 支持32个核 */
    int hwsync_mem_size;
    char *hwsync_shm_ptr;
#ifdef HWSYNC_USED_PMUTEX
    pthread_mutex_t * pmutex_pld;
    pthread_mutex_t * pmutex_sync;
    pthread_cond_t * pcond_pld;
    pthread_cond_t * pcond_sync;
#else
    std::mutex mutex_sync;
    std::mutex mutex_pld;
    std::condition_variable_any cond_sync;
    std::condition_variable_any cond_pld;
#endif
    
    uint32_t *hs_sync_timer_num;  /* timeout阈值 HS_TIME_OUT_CNT，所有grp公用一个阈值 */
    uint32_t *hs_sync_timer_cnt;  /* 为每个核分配一个sync timer(硬件只有一个timer) */

  public:
    hwsync_t(char *hwsync_masks, uint32_t hwsync_timer_num);
    virtual ~hwsync_t();

    bool enter(unsigned core_id);
    bool enter(unsigned core_id, uint32_t coremap);

    bool load(reg_t addr, size_t len, uint8_t* bytes);
    bool store(reg_t addr, size_t len, const uint8_t* bytes);
    bool is_hwsync_done(void);
    bool is_hs_group_sync(int coreid);   /* coreid所在的sync组是否处于sync状态 */
    bool is_hwsync_timeout(int coreid);
    void hwsync_timer_cnts_add(int coreid, uint32_t clks);
    void hwsync_timer_clear(int coreid);
    void hwsync_clear(void);
    uint32_t get_hwsync_timer_cnts(int coreid);
    uint32_t get_hwsync() { return *req_sync;}
    uint32_t get_hwsync_timer_thresh(void) {
      return (hs_sync_timer_num) ? *hs_sync_timer_num : 0xffffffff;
    };
    void reset(uint32_t id) {
      *req_pld |= (0x1 << id);
    #ifdef HWSYNC_USED_PMUTEX
        pthread_cond_broadcast(pcond_pld);
    #else
        cond_pld.notify_all();
    #endif
    }
    size_t size(void) {return 0x80000;};
};

#endif // __HWSYNC_H__
