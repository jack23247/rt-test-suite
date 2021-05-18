#ifndef H_SCHED_H
#define H_SCHED_H

#include "config.h"

#include <linux/sched.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <stdint.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

/**
 * @brief Struct used by the task to communicate parameters to the scheduler
 * https://man7.org/linux/man-pages/man2/sched_getattr.2.html
 * https://www.i-programmer.info/programming/cc/13002-applying-c-deadline-scheduling.html?start=1
 */
struct sched_attr {
    uint32_t size;           /* Size of this structure */
    uint32_t sched_policy;   /* Policy (SCHED_*) */
    uint64_t sched_flags;    /* Flags */
    int32_t sched_nice;      /* Nice value (SCHED_OTHER, SCHED_BATCH) */
    uint32_t sched_priority; /* Static priority (SCHED_FIFO, SCHED_RR) */
    /* Remaining fields are for SCHED_DEADLINE */
    uint64_t sched_runtime;
    uint64_t sched_deadline;
    uint64_t sched_period;
};

/**
 * @brief Wrapper for the __NR_sched_setattr kernel call
 *
 * @param pid The PID of the process whose parameters are going to be modified
 * @param attr A pointer to a sched_attr structure used to specify attributes
 * @param flags ???
 *
 * @return On success, 0; on failure -1. The error code is stored in errno
 */
static inline int sched_setattr(
    pid_t pid, const struct sched_attr* attr,
    unsigned int flags) {  // TODO who knows what "flags" does? Is
    // it the same as "sched_attr.flags"?
    return syscall(__NR_sched_setattr, pid, attr, flags);
}

#endif  // H_SCHED_H