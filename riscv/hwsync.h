#ifndef __HWSYNC_H__
#define __HWSYNC_H__

#include <vector>
#include <mutex>
#include <condition_variable>

class hwsync_t {
  private:
    std::vector<uint32_t> masks;
    uint32_t req_sync;
    uint32_t req_pld;
    int group_count;

    std::mutex mutex_sync;
    std::mutex mutex_pld;
    std::condition_variable_any cond_sync;
    std::condition_variable_any cond_pld;

  public:
    hwsync_t(size_t nprocs, size_t bank_id);
    ~hwsync_t();

    bool enter(unsigned core_id);
    bool enter(unsigned core_id, uint32_t coremap);
};

#endif // __HWSYNC_H__
