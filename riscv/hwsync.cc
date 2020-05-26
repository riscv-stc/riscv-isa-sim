#include <iostream>
#include <assert.h>

#include "hwsync.h"

//#define DEBUG

hwsync_t::hwsync_t(size_t nprocs) : group_count(16) {
    // reset group masks
    for (int i=0; i<group_count; i++) {
        masks.push_back(~0);
    }
    sync_enter = ~0;

    // add all processors to group 0
    masks[0] = ~((1 << nprocs) - 1);
}

hwsync_t::~hwsync_t() {
}

bool
hwsync_t::enter(unsigned core_id, uint32_t coremap) {
    std::unique_lock<std::mutex> lock(sync_mutex);

#ifdef DEBUG
    std::cout << "start sync" << std::endl;
#endif

    if (coremap) {
        pld_enter &= ~(1 << core_id);
        if ((pld_enter | ~coremap) == ~coremap) {
            // all enter, clear enter requests
            pld_enter |= coremap;
            sync_cond.notify_all();
        }
        sync_cond.wait(lock, [&]{ return (pld_enter & 1 << core_id) != 0; });

        return true;
    }

    sync_enter &= ~(1 << core_id);
    for (int i=0; i< group_count; i++) {
        if ((sync_enter | masks[i]) == masks[i] && ~masks[i] != 0) {
            // all enter, clear enter requests
            sync_enter |= ~masks[i];
            sync_cond.notify_all();
            break;
        }
    }

    sync_cond.wait(lock, [&]{ return (sync_enter & 1 << core_id) != 0; });

#ifdef DEBUG
    std::cout << "end sync" << std::endl;
#endif

    return true;
}