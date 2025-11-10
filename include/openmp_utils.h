#ifndef OPENMP_UTILS_H
#define OPENMP_UTILS_H

#include "config.h"

void set_openmp_threads(int num_threads);
int get_optimal_thread_count();
void enable_openmp();
void disable_openmp();
int is_openmp_enabled();

double measure_openmp_performance(void (*func)(void), int iterations);

#endif
