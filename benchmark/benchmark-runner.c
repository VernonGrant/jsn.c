/**
 * Author: Vernon Grant
 * Repository: https://github.com/VernonGrant/jsn.c
 * License: https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * Created: 2022-10-21
 **/

#include "../jsn.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>


#include "./utils/benchmark.h"

int main(int argc, char *argv[]) {
    // Canada benchmark.
    jsn_benchmark_start();
    jsn_handle canada = jsn_from_file("./benchmark/data/canada.json");
    jsn_benchmark_end("Parsing of ./benchmark/data/canada.json      ");

    // Citm Catalog benchmark.
    jsn_benchmark_start();
    jsn_handle citm = jsn_from_file("./benchmark/data/citm_catalog.json");
    jsn_benchmark_end("Parsing of ./benchmark/data/citm_catalog.json");

    // Twitter benchmark.
    jsn_benchmark_start();
    jsn_handle twitter = jsn_from_file("./benchmark/data/twitter.json");
    jsn_benchmark_end("Parsing of ./benchmark/data/twitter.json     ");

    // free.
    jsn_free(canada);
    jsn_free(citm);
    jsn_free(twitter);

    return 0;
}
