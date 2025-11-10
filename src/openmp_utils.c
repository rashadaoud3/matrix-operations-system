#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include "../include/openmp_utils.h"

static int openmp_enabled = 1;

void set_openmp_threads(int num_threads) {
    #ifdef _OPENMP
    if (num_threads > 0) {
        omp_set_num_threads(num_threads);
    }
    #endif
}

int get_optimal_thread_count() {
    #ifdef _OPENMP
    return omp_get_max_threads();
    #else
    return 1;
    #endif
}

void enable_openmp() {
    openmp_enabled = 1;
    set_openmp_threads(global_config.openmp_threads);
}

void disable_openmp() {
    openmp_enabled = 0;
}

int is_openmp_enabled() {
    return openmp_enabled;
}

double measure_openmp_performance(void (*func)(void), int iterations) {
    #ifdef _OPENMP
    double start_time = omp_get_wtime();
    
    #pragma omp parallel for
    for (int i = 0; i < iterations; i++) {
        func();
    }
    
    double end_time = omp_get_wtime();
    return end_time - start_time;
    #else
    double start_time = (double)clock() / CLOCKS_PER_SEC;
    
    for (int i = 0; i < iterations; i++) {
        func();
    }
    
    double end_time = (double)clock() / CLOCKS_PER_SEC;
    return end_time - start_time;
    #endif
}
