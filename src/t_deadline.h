#ifndef T_DEADLINE_H
#define T_DEADLINE_H

#include <assert.h>
#include <immintrin.h>
#include <memory.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>

#include "config.h"
#include "h_sched.h"
#include "h_time.h"

#define DL_RUNTIME 10 * NSEC_PER_MSEC
#define DL_PERIOD 20 * NSEC_PER_MSEC
#define DL_DEADLINE 11 * NSEC_PER_MSEC
#define DL_DELTA_WARN_THRESHOLD 500 * USEC_PER_MSEC

enum dl_flags {
    DL_FLAG_NONE = 0,
    DL_FLAG_RAND,
    DL_FLAG_RAND_HEAVY,
    DL_FLAG_AMT  // Amount
};

void* TDeadline_Entry(void* p);

#endif  // T_DEADLINE_H