#include "t_deadline.h"

void TDeadline_DlMissHandler(int sig) {
    assert(sig != 0);
    syslog(LOG_WARNING, "%s", "(T_DEADLINE) Deadline miss detected");
    (void)signal(SIGXCPU, SIG_DFL);
}

static inline void TDeadline_Payload(struct timespec* gts, stats_t* stats) {
    nsec_t curTime = 0, prevTime = 0, deltaTime = 0;
    printf(
        "\nSCHED_DEADLINE Test w/o payload (Period: 20ms; Runtime/WCET: "
        "10ms; Deadline: 11ms)\nIteration,Jitter\n");
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
    unsigned int dieRoll = 0;
    printf(
        "\nSCHED_DEADLINE Test w/ rand() dice demo (Period: 20ms; "
        "Runtime/WCET: 10ms; Deadline: 11ms)\nIteration,Jitter,DieRoll\n");
    for (;;) {
        dieRoll = ((rand() % 6) + 1);
        prevTime = curTime;
        curTime = HTime_GetNsDelta(gts);
        if (curTime != 0 && prevTime != 0) {
            deltaTime = abs((int)(curTime - prevTime - (DL_PERIOD)));
        }
        if (deltaTime >= DL_DELTA_WARN_THRESHOLD) {
            syslog(LOG_WARNING, "%s%lluns",
                   "(T_DEADLINE_RAND) Abnormal time delta detected between "
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
    unsigned int dieRollTotal = 0;
    printf(
        "\nSCHED_DEADLINE Test w/ rand() dice demo (100 rolls/cycle) "
        "(Period: 20ms; Runtime/WCET: 10ms; Deadline: 11ms)\n"
        "Iteration,Jitter,DieRollTotal\n");
    for (;;) {
        dieRollTotal = 0;
        for (int i = 0; i < 100; i++) {
            dieRollTotal += ((rand() % 6) + 1);
        }
        prevTime = curTime;
        curTime = HTime_GetNsDelta(gts); // TODO move this at the beginning of the payload we don't care how much it takes to execute the dierolls
        if (curTime != 0 && prevTime != 0) {
            deltaTime = abs((int)(curTime - prevTime - (DL_PERIOD)));
        }
        if (deltaTime >= DL_DELTA_WARN_THRESHOLD) {
            syslog(LOG_WARNING, "%s%lluns",
                   "(T_DEADLINE_RAND_HVY) Abnormal time delta detected "
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
        case DL_FLAG_RAND:
            TDeadline_Payload_RDRand(&gts, &stats);
            break;
        case DL_FLAG_RAND_HEAVY:
            TDeadline_Payload_RDRandHvy(&gts, &stats);
            break;
        default:
            assert(0);
    }
    pthread_exit(EXIT_SUCCESS);
    return (NULL);
}
