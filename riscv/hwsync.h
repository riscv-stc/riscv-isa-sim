#ifndef __HWSYNC_H__
#define __HWSYNC_H__

#include <map>
#include <mutex>

class hwsync_t {
  private:
    std::map<unsigned, int> num_total;
    std::map<unsigned, int> num_enter;
    std::map<unsigned, int> num_exit;
    std::map<unsigned, bool> idle;
    std::mutex mutex;

  public:
    hwsync_t();
    ~hwsync_t();

    void join(unsigned group_id);
    void leave(unsigned group_id);
    bool enter(unsigned group_id);
    bool exit(unsigned group_id);
    bool done(unsigned group_id);
    bool exited(unsigned group_id);
};

#endif // __HWSYNC_H__
