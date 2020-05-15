#include <iostream>
#include <assert.h>

#include "hwsync.h"

//#define DEBUG

hwsync_t::hwsync_t() : num_total(), num_enter() {

}

hwsync_t::~hwsync_t() {
}

void
hwsync_t::join(unsigned group_id) {
    if (num_total.find(group_id) == num_total.end()) {
        num_total[group_id] = 0;
        idle[group_id] = true;
    } else if (!idle[group_id]) {
        std::cerr << "ERROR: join sync group while group is busy" << std::endl;
        exit(1);
    }

    num_total[group_id] ++;
}

void
hwsync_t::leave(unsigned group_id) {
    if (!idle[group_id]) {
        std::cerr << "ERROR: leave sync group while group is busy" << std::endl;
        exit(1);
    }

    if (num_total.find(group_id) == num_total.end()) {
        std::cerr << "ERROR: unregister on empty sync group " << group_id << std::endl;
        exit(1);
    }

    num_total[group_id] --;
    assert(num_total[group_id] >= 0);
}


bool
hwsync_t::enter(unsigned group_id) {
    if (num_exit[group_id] != num_total[group_id] && num_exit[group_id] != 0)
        return false;

#ifdef DEBUG
    std::cout << "enter sync" << std::endl;
#endif

    idle[group_id] = false;

    if (num_enter.find(group_id) == num_enter.end()) {
        num_enter[group_id] = 0;
    }

    num_enter[group_id] ++;
    assert(num_enter[group_id] <= num_total[group_id]);

    return true;
}

bool
hwsync_t::exit(unsigned group_id) {
    if (num_enter[group_id] != num_total[group_id])
        return false;

#ifdef DEBUG
    std::cout << "exit sync" << std::endl;
#endif

    if (num_exit.find(group_id) == num_exit.end()) {
        num_exit[group_id] = 0;
    }

    num_exit[group_id] ++;

    if (num_exit[group_id] == num_total[group_id]) {
        num_exit[group_id] = 0;
        num_enter[group_id] = 0;
        idle[group_id] = true;
    }

    return true;
}


bool
hwsync_t::done(unsigned group_id) {
    return num_enter[group_id] == num_total[group_id];
}

bool
hwsync_t::exited(unsigned group_id) {
    return num_exit[group_id] == num_total[group_id] || num_exit[group_id] == 0;
}