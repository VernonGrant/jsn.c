#include "jsn.h"
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

// Development use only.
#include "benchmark/benchmark.h"

int main(int argc, char *argv[]) {
    // Create the main object.
    jsn_print_memory_usage("Memory at the start:");

    // Benchmark parsing of large file.
    jsn_benchmark_start();
    jsn_handle root_25 = jsn_from_file("./benchmark/25mb.json");
    jsn_benchmark_end("Parsing 25MB JSON file");

    jsn_benchmark_start();
    jsn_handle root_20 = jsn_from_file("./benchmark/20mb.json");
    jsn_benchmark_end("Parsing 20MB JSON file");

    // Check memory usage.
    jsn_print_memory_usage("Memory at the end:");

    // free.
    jsn_free(root_25);
    jsn_free(root_20);

    return 0;
}
