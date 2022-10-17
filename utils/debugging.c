#include <time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include "debugging.h"

static clock_t benchmark_clock;

void jsn_print_memory_usage(const char *message) {
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    printf("%s: %ld MB!\n", message, (usage.ru_maxrss / 1024));
}

void jsn_benchmark_start() {
    benchmark_clock = clock();
    printf("Benchmarking:\n");
}

void jsn_benchmark_end(const char *log_prefix) {
    double delta = (double)(clock() - benchmark_clock);
    double elapsed_time = delta / CLOCKS_PER_SEC;

    // Print out benchmark results.
    printf("Benchmark Result:\n");
    printf("---------------------------------------------------------------\n");
    printf("It took %f, seconds!\n", elapsed_time);
    printf("---------------------------------------------------------------\n");

    // Write the result to a file.
    FILE *f = fopen("../benchmarking.txt", "a");

    if (f == NULL) {
        printf("Error opening file!\n");
        exit(1);
    }

    // get the time.
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    fprintf(f, "%d-%02d-%02d %02d:%02d:%02d | ", tm.tm_year + 1900,
            tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    fprintf(f, "%s | %f seconds.\n", log_prefix, elapsed_time);
    fclose(f);
}

