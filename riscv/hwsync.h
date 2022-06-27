#ifndef __HWSYNC_H__
#define __HWSYNC_H__

#include <vector>
#include <mutex>
#include <condition_variable>
#include "decode.h"
#include <semaphore.h>
#include <pthread.h>
#include <bitset>

#include "devices.h"

#define MASK_INITIAL_VALUE          (~0)

#define setBitValue(var, idx, value) (var) = (value > 0) ? ((var) | (1 << (idx))) : ((var) & ~(1 << (idx)))
#define clearBitMap(var, fromBit, toBit) (var) = ((var) & ~(((0x1 << ((toBit) + 1 - (fromBit))) - 1) << (fromBit)))
#define getBitValue(var, idx) ((var >> (idx)) & 1)

class hwsync_t: public abstract_device_t {
  private:
    uint32_t *sync_masks;      /* sync_masks[0]-sync_masks[15]共16个sync组, 为0的bit对应的核在同一个sync组， 默认值{0xffffffff} */
    uint32_t *sync_status;   /* core_n开始sync时第n位清零，sync结束重新, 初始值0x0000 */
    uint32_t* group_valid;
    uint32_t* group_done;
    uint32_t *req_pld;
    int group_count;  /* 支持16个sync组 */
    int core_num;     /* 支持32个核 */
    int hwsync_mem_size;

    uint8_t *hwsync_ptr;
    uint8_t* hwsync_base_addr;

    std::condition_variable_any *group_locks;
    
    void cpyBits(uint8_t *src, uint8_t *dst, int src_start, int dst_start, int len)
    {
      for (int i = 0; i < len; i++)
      {
          setBitValue(*dst, dst_start + i, getBitValue(*src, src_start + i));
      }
    }
    uint32_t get_groupID_from_coreID(uint32_t core_id);


    std::mutex mutex_sync;
    std::mutex mutex_pld;
    std::condition_variable_any cond_sync;
    
    std::condition_variable_any cond_pld;
    
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
    bool is_group_all_sync(int group_id); //Determines whether a group is fully synchronized
    bool is_hwsync_timeout(int coreid);
    void hwsync_timer_cnts_add(int coreid, uint32_t clks);
    void hwsync_timer_clear(int coreid);
    void hwsync_clear(void);
    uint32_t get_hwsync_timer_cnts(int coreid);
    uint32_t get_hwsync() { return ~(*sync_status);}
    uint32_t get_hwsync_timer_thresh(void) {
      return (hs_sync_timer_num) ? *hs_sync_timer_num : 0xffffffff;
    };
    void reset(uint32_t id) {
      *req_pld |= (0x1 << id);
      cond_pld.notify_all();
    }
    size_t size(void) {return 0x80000;};
    
   
};

#endif // __HWSYNC_H__
