#ifndef BENCHMARK_H
#define BENCHMARK_H

/**
 * Author: Vernon Grant
 * Repository: https://github.com/VernonGrant/jsn.c
 * License: https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * Created: 2022-10-21
 **/

void jsn_init_allocation_counters();

void jsn_print_memory_usage(const char *message);

void jsn_benchmark_start();

void jsn_benchmark_end(const char *log_prefix);

#endif
