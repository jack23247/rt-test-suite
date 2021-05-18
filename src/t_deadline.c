#include "t_deadline.h"

void TDeadline_DlMissHandler(int sig) {
    assert(sig != 0);
    syslog(LOG_WARNING, "%s", "(T_DEADLINE) Deadline miss detected");
    (void)signal(SIGXCPU, SIG_DFL);
}

static inline void TDeadline_Payload(struct timespec* gts, stats_t* stats) {
    nsec_t curTime = 0, prevTime = 0, deltaTime = 0;
    printf(
        "#\n# SCHED_DEADLINE Test w/o payload (Period: 20ms, Runtime/WCET: "
        "10ms, Deadline: 11ms)\n# iter,deltat\n");
    for (;;) {
        // doNothing();
        prevTime = curTime;
        curTime = HTime_GetNsDelta(gts);
        if (curTime != 0 && prevTime != 0) {
            deltaTime = abs((int)(curTime - prevTime - (DL_PERIOD)));
        }
        if (deltaTime >= DL_DELTA_WARN_THRESHOLD) {
            syslog(
                LOG_WARNING, "%s%lluns",
                "(T_DEADLINE) Abnormal time delta detected between cycles: ",
                deltaTime);
        }
        stats->totalIterations++;
        if (stats->totalIterations == stats->targetIterations) break;
        printf("%u,%llu\n", stats->totalIterations, deltaTime);
        sched_yield();
    };
}

static inline void TDeadline_Payload_RDRand(struct timespec* gts,
                                            stats_t* stats) {
    nsec_t curTime = 0, prevTime = 0, deltaTime = 0;
    unsigned int randomness = 0, dieRoll = 0;
    int rc = 1;
    printf(
        "#\n# SCHED_DEADLINE Test w/ RDRAND dice demo (Period: 20ms, "
        "Runtime/WCET: 10ms, Deadline: 11ms)\n# iter,deltat,dieroll\n");
    for (;;) {
        rc &= _rdrand32_step(&randomness);
        assert(rc != 0);
        dieRoll = ((randomness % 6) + 1);
        prevTime = curTime;
        curTime = HTime_GetNsDelta(gts);
        if (curTime != 0 && prevTime != 0) {
            deltaTime = abs((int)(curTime - prevTime - (DL_PERIOD)));
        }
        if (deltaTime >= DL_DELTA_WARN_THRESHOLD) {
            syslog(LOG_WARNING, "%s%lluns",
                   "(T_DEADLINE_RDRAND) Abnormal time delta detected between "
                   "cycles: ",
                   deltaTime);
        }
        stats->totalIterations++;
        if (stats->totalIterations == stats->targetIterations) break;
        printf("%u,%llu,%u\n", stats->totalIterations, deltaTime, dieRoll);
        sched_yield();
    };
}

static inline void TDeadline_Payload_RDRandHvy(struct timespec* gts,
                                               stats_t* stats) {
    nsec_t curTime = 0, prevTime = 0, deltaTime = 0;
    unsigned int randomness = 0, dieRollTotal = 0;
    int rc = 1;
    printf(
        "#\n# SCHED_DEADLINE Test w/ RDRAND dice demo (100 rolls/cycle) "
        "(Period: 20ms, Runtime/WCET: 10ms, Deadline: 11ms)\n# "
        "iter,deltat,dieroll\n");
    for (;;) {
        for (int i = 0; i < 100; i++) {
            rc &= _rdrand32_step(&randomness);
            assert(rc != 0);
            dieRollTotal += ((randomness % 6) + 1);
        }
        prevTime = curTime;
        curTime = HTime_GetNsDelta(gts);
        if (curTime != 0 && prevTime != 0) {
            deltaTime = abs((int)(curTime - prevTime - (DL_PERIOD)));
        }
        if (deltaTime >= DL_DELTA_WARN_THRESHOLD) {
            syslog(LOG_WARNING, "%s%lluns",
                   "(T_DEADLINE_RDRAND_HVY) Abnormal time delta detected "
                   "between cycles: ",
                   deltaTime);
        }
        stats->totalIterations++;
        if (stats->totalIterations == stats->targetIterations) break;
        printf("%u,%llu,%u\n", stats->totalIterations, deltaTime,
               dieRollTotal);
        sched_yield();
    };
}

void* TDeadline_Entry(void* args) {
    int payloadSelect = *(int*)args;
    struct sched_attr attr = {.size = sizeof(attr),
                              .sched_flags = 0 | SCHED_FLAG_DL_OVERRUN,
                              .sched_policy = SCHED_DEADLINE,
                              .sched_runtime = DL_RUNTIME,     // 10ms
                              .sched_period = DL_PERIOD,       // 20ms
                              .sched_deadline = DL_DEADLINE};  // 11ms
    (void)signal(SIGXCPU, TDeadline_DlMissHandler);
    if (sched_setattr(0, &attr, 0)) {
        perror("TDeadline_Entry: sched_setattr");
        exit(EXIT_FAILURE);
    }
    stats_t stats = {.targetIterations = CONFIG_MAX_ITERS,
                     .totalIterations = 0,
                     .clockMisses = 0};
    struct timespec gts;
    HTime_InitBase();
    switch (payloadSelect) {
        case DL_FLAG_NONE:
            TDeadline_Payload(&gts, &stats);
            break;
        case DL_FLAG_RDRAND:
            TDeadline_Payload_RDRand(&gts, &stats);
            break;
        case DL_FLAG_RDRAND_HEAVY:
            TDeadline_Payload_RDRandHvy(&gts, &stats);
            break;
        default:
            assert(0);
    }
    pthread_exit(EXIT_SUCCESS);
    return (NULL);
}
