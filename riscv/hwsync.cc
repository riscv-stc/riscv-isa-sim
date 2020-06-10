#include <iostream>
#include <assert.h>

#include "hwsync.h"

//#define DEBUG

hwsync_t::hwsync_t(size_t nprocs, size_t bank_id) : group_count(16) {
    // reset group masks
    for (int i=0; i<group_count; i++) {
        masks.push_back(~0);
    }
    req_sync = ~0;
    req_pld = ~0;

    // add all processors to group 0
    uint32_t mask = (1 << nprocs) - 1;
    masks[0] = ~(mask << (nprocs * bank_id));
}

hwsync_t::~hwsync_t() {
}

bool
hwsync_t::enter(unsigned core_id) {
    std::unique_lock<std::mutex> lock(mutex_sync);

#ifdef DEBUG
    std::cout << "core" << core_id << ": start sync" << std::endl;
#endif

    req_sync &= ~(1 << core_id);
    for (int i=0; i< group_count; i++) {
        if ((req_sync | masks[i]) == masks[i] && ~masks[i] != 0) {
            // all enter, clear enter requests
            req_sync |= ~masks[i];
            cond_sync.notify_all();
            break;
        }
    }

    cond_sync.wait(lock, [&]{ return (req_sync & 1 << core_id) != 0; });

#ifdef DEBUG
    std::cout << "core" << core_id << ": end sync" << std::endl;
#endif

    return true;
}

bool
hwsync_t::enter(unsigned core_id, uint32_t coremap) {
    std::unique_lock<std::mutex> lock(mutex_pld);

#ifdef DEBUG
    std::cout << "core" << core_id << ": start pld, coremap=" << coremap << std::endl;
#endif

    req_pld &= ~(1 << core_id);
    if ((req_pld | ~coremap) == ~coremap) {
        // all enter, clear enter requests
        req_pld |= coremap;
        cond_pld.notify_all();
    }
    cond_pld.wait(lock, [&]{ return (req_pld & 1 << core_id) != 0; });

#ifdef DEBUG
    std::cout << "core" << core_id << ": end pld" << std::endl;
#endif

    return true;
}