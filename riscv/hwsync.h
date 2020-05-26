#ifndef __HWSYNC_H__
#define __HWSYNC_H__

#include <vector>
#include <mutex>
#include <condition_variable>

class hwsync_t {
  private:
    std::vector<uint32_t> masks;
    uint32_t sync_enter;
    uint32_t pld_enter;
    int group_count;

    std::mutex sync_mutex;
    std::condition_variable_any sync_cond;

  public:
    hwsync_t(size_t nprocs);
    ~hwsync_t();

    bool enter(unsigned core_id, uint32_t coremap = 0);
};

#endif // __HWSYNC_H__
