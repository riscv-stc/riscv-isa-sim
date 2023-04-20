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

#define MASK_INITIAL_VALUE (~0)

#define GROUP_MASK_OFFSET    (0x00000000)
#define GROUP_DONE_OFFSET    (0x00000044)
#define GROUP_VALID_OFFSET   (0x00000048)
#define GROUP_VALID_3_0      (0x00000100)
#define GROUP_VALID_7_4      (0x00000200)
#define GROUP_VALID_11_8     (0x00000300)
#define GROUP_VALID_15_11    (0x00000400)
#define SYNC_STATUS_OFFSET   (0x00000050)
#define HS_SW_SYNC_REQ_CLR_OFFSET (0x00000068)

#define setBitValue(var, idx, value) (var) = (value > 0) ? ((var) | (1 << (idx))) : ((var) & ~(1 << (idx)))
#define getBitValue(var, idx) ((var >> (idx)) & 1)

template<typename T>
void free_set(T* point)
{
  if(point != nullptr)
  {
    free(point);
    point=nullptr;
  }
}

class hwsync_t : public abstract_device_t
{
private:
  simif_t *sim;
  uint32_t *sync_masks = nullptr;  /* sync_masks[0]-sync_masks[15]共16个sync组, 为0的bit对应的核在同一个sync组， 默认值{0xffffffff} */
  uint32_t *sync_status = nullptr; /* core_n开始sync时第n位清零，sync结束重新, 初始值0x0000 */
  uint32_t *group_valid = nullptr;
  uint32_t *req_pld = nullptr;
  uint8_t *hwsync_ptr = nullptr;
  uint8_t *hwsync_base_addr = nullptr;

  int group_count; /* 支持16个sync组 */
  int core_num;    /* 支持32个核 */
  int hwsync_mem_size;

  std::condition_variable_any *group_locks;

  void cpyBits(uint8_t *src, uint8_t *dst, int src_start, int dst_start, int len)
  {
    for (int i = 0; i < len; i++)
    {
      setBitValue(*dst, dst_start + i, getBitValue(*src, src_start + i));
    }
  }

  std::mutex mutex_sync;
  std::mutex mutex_pld;

  std::condition_variable_any cond_pld;

  uint32_t *hs_sync_timer_num; /* timeout阈值 HS_TIME_OUT_CNT，所有grp公用一个阈值 */
  uint32_t *hs_sync_timer_cnt; /* 为每个核分配一个sync timer(硬件只有一个timer) */

public:
  hwsync_t(simif_t *sim, char *hwsync_masks, uint32_t hwsync_timer_num);
  virtual ~hwsync_t();

  bool enter(unsigned core_id);
  bool enter(unsigned core_id, uint32_t coremap);

  bool load(reg_t addr, size_t len, uint8_t *bytes);
  bool store(reg_t addr, size_t len, const uint8_t *bytes);
  bool is_hwsync_done(void);
  bool is_hs_group_sync(int coreid);    /* coreid所在的sync组是否处于sync状态 */
  bool is_group_all_sync(int group_id); // Determines whether a group is fully synchronized
  bool is_hwsync_timeout(int coreid);
  void hwsync_timer_cnts_add(int coreid, uint32_t clks);
  void hwsync_timer_clear(int coreid);
  void hwsync_clear(void);
  void pld_clr(unsigned core_id);

  uint32_t get_hwsync_timer_cnts(int coreid);
  uint32_t get_hwsync() { return ~(*sync_status); }
  uint32_t get_pld() {return *req_pld;}
  uint32_t get_hwsync_timer_thresh(void)
  {
    return (hs_sync_timer_num) ? *hs_sync_timer_num : 0xffffffff;
  };
  uint32_t get_groupID_from_coreID(uint32_t core_id);
  size_t size(void) { return HWSYNC_SIZE; };
};

#endif // __HWSYNC_H__
