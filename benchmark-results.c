/**
 * Author: Vernon Grant
 * Repository: https://github.com/VernonGrant/jsn.c
 * License: https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * Created: 2022-10-21
 **/

#include "jsn.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "benchmark/benchmark.h"

int main(int argc, char *argv[]) {
    // Create the main object.
    jsn_print_memory_usage("Memory at the start:");

    // Canada benchmark.
    jsn_benchmark_start();
    jsn_handle canada = jsn_from_file("./benchmark/canada.json");
    jsn_benchmark_end("Parsing of ./benchmark/canada.json      ");

    // Citm Catalog benchmark.
    jsn_benchmark_start();
    jsn_handle citm = jsn_from_file("./benchmark/citm_catalog.json");
    jsn_benchmark_end("Parsing of ./benchmark/citm_catalog.json");

    // Twitter benchmark.
    jsn_benchmark_start();
    jsn_handle twitter = jsn_from_file("./benchmark/twitter.json");
    jsn_benchmark_end("Parsing of ./benchmark/twitter.json     ");

    // Check memory usage.
    jsn_print_memory_usage("Memory at the end:");

    // free.
    jsn_free(canada);
    jsn_free(citm);
    jsn_free(twitter);

    return 0;
}
