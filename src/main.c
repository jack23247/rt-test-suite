#define _GNU_SOURCE

#include <assert.h>
#include <libcpuid/libcpuid.h>
#include <pthread.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>

#include "config.h"
#include "t_deadline.h"

typedef struct worker {
    pthread_t tid;
    void* (*entry)(void*);
} worker_t;

int main(void) {
    struct sysinfo systemInfo;
    struct utsname kernelInfo;
    struct cpu_raw_data_t raw;
    struct cpu_id_t data;
    if (!cpuid_present()) {
        perror("cpuid_present");
        exit(EXIT_FAILURE);
    }
    if (cpuid_get_raw_data(&raw) < 0) {
        perror("cpuid_get_raw_data");
        exit(EXIT_FAILURE);
    }
    if (cpu_identify(&raw, &data) < 0) {
        perror("cpuid_identify");
        exit(EXIT_FAILURE);
    }
    sysinfo(&systemInfo);
    uname(&kernelInfo);
    printf("# %lu\n", time(NULL));
    printf("# %s %s %s %s \n", kernelInfo.sysname, kernelInfo.release,
           kernelInfo.version, kernelInfo.machine);
    printf("# %s - Cores: %i, Threads: %i\n", data.brand_str, data.num_cores,
           data.num_logical_cpus);
    printf("# Memory (in megabytes) - Total: %.0f, Used: %.0f, Free: %.0f\n",
           systemInfo.totalram / 1000000.0,
           (systemInfo.totalram - systemInfo.freeram) / 1000000.0,
           systemInfo.freeram / 1000000.0);
    worker_t workers[DL_FLAG_AMT];
    for (int i = 0; i < DL_FLAG_AMT; i++) {  // Dispatch workers
        workers[i].entry = TDeadline_Entry;
        pthread_create(&(workers[i].tid), NULL, workers[i].entry, &i);
        pthread_join(workers[i].tid, NULL);
        printf("# End of test (%i of %i)\n", i + 1, DL_FLAG_AMT);
    }
    printf("# %lu EOF\n", time(NULL));
    pthread_exit(0);  // Failsafe, not really needed
    return (EXIT_SUCCESS);
}
