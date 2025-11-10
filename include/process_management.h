#ifndef PROCESS_MANAGEMENT_H
#define PROCESS_MANAGEMENT_H

#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include "config.h"
#include "matrix_operations.h"

typedef enum {
    OP_ADD_ELEMENT,
    OP_SUBTRACT_ELEMENT, 
    OP_MULTIPLY_ELEMENT,
    OP_DETERMINANT_PART
} operation_type_t;

typedef struct {
    pid_t pid;
    int pipe_in[2];
    int pipe_out[2];
    int busy;
    time_t last_used;
    operation_type_t current_op;
} child_process_t;

extern child_process_t process_pool[MAX_PROCESSES];
extern int active_processes;

void setup_signal_handlers();
void computation_start_handler(int sig);
void computation_done_handler(int sig);
void child_cleanup_handler(int sig);
void send_signal_to_children(int sig);

void initialize_process_pool();
void cleanup_process_pool();
child_process_t* get_available_process();
void return_process(child_process_t* process);
void cleanup_idle_processes();
void cleanup_old_processes();
int create_worker_process(child_process_t* process);
void worker_process_main(int pipe_in, int pipe_out);

matrix_t* add_matrices_parallel(const matrix_t* A, const matrix_t* B);
matrix_t* subtract_matrices_parallel(const matrix_t* A, const matrix_t* B);
matrix_t* multiply_matrices_parallel(const matrix_t* A, const matrix_t* B);
double matrix_determinant_parallel(const matrix_t* matrix);

#endif
