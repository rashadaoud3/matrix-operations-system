#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include "../include/process_management.h"

child_process_t process_pool[MAX_PROCESSES];
int active_processes = 0;

volatile sig_atomic_t computation_started = 0;
volatile sig_atomic_t computation_completed = 0;
volatile sig_atomic_t child_ready_count = 0;
volatile sig_atomic_t child_done_count = 0;

void computation_start_handler(int sig) {
    (void)sig;
    computation_started = 1;
    child_ready_count++;
}

void computation_done_handler(int sig) {
    (void)sig;
    computation_completed = 1;
    child_done_count++;
}

void child_cleanup_handler(int sig) {
    (void)sig;
    int status;
    pid_t pid;
    
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        for (int i = 0; i < global_config.max_processes; i++) {
            if (process_pool[i].pid == pid) {
                process_pool[i].pid = -1;
                active_processes--;
                break;
            }
        }
    }
}

void setup_signal_handlers() {
    struct sigaction sa;
    
    sa.sa_handler = computation_start_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGUSR1, &sa, NULL);
    
    sa.sa_handler = computation_done_handler;
    sigaction(SIGUSR2, &sa, NULL);
    
    sa.sa_handler = child_cleanup_handler;
    sa.sa_flags = SA_NOCLDSTOP;
    sigaction(SIGCHLD, &sa, NULL);
    
    signal(SIGPIPE, SIG_IGN);
}

void send_signal_to_children(int sig) {
    for (int i = 0; i < global_config.max_processes; i++) {
        if (process_pool[i].pid > 0) {
            kill(process_pool[i].pid, sig);
        }
    }
}

void initialize_process_pool() {
    printf("Initializing process pool with %d processes\n", global_config.max_processes);
    for (int i = 0; i < global_config.max_processes; i++) {
        process_pool[i].pid = -1;
        process_pool[i].busy = 0;
        process_pool[i].last_used = 0;
        process_pool[i].pipe_in[0] = process_pool[i].pipe_in[1] = -1;
        process_pool[i].pipe_out[0] = process_pool[i].pipe_out[1] = -1;
    }
    active_processes = 0;
}

void cleanup_process_pool() {
    printf("Cleaning up process pool...\n");
    for (int i = 0; i < global_config.max_processes; i++) {
        if (process_pool[i].pid > 0) {
            kill(process_pool[i].pid, SIGTERM);
            waitpid(process_pool[i].pid, NULL, 0);
            
            if (process_pool[i].pipe_in[0] != -1) close(process_pool[i].pipe_in[0]);
            if (process_pool[i].pipe_in[1] != -1) close(process_pool[i].pipe_in[1]);
            if (process_pool[i].pipe_out[0] != -1) close(process_pool[i].pipe_out[0]);
            if (process_pool[i].pipe_out[1] != -1) close(process_pool[i].pipe_out[1]);
            
            process_pool[i].pid = -1;
        }
    }
    active_processes = 0;
}

void worker_simple_calculation(int pipe_in, int pipe_out) {
    while (1) {
        operation_type_t op_type;
        double value1, value2;
        double result;
        

        if (read(pipe_in, &op_type, sizeof(op_type)) != sizeof(op_type)) break;
        if (read(pipe_in, &value1, sizeof(value1)) != sizeof(value1)) break;
        if (read(pipe_in, &value2, sizeof(value2)) != sizeof(value2)) break;
        

        switch (op_type) {
            case OP_ADD_ELEMENT:
                result = value1 + value2;
                break;
            case OP_SUBTRACT_ELEMENT:
                result = value1 - value2;
                break;
            case OP_MULTIPLY_ELEMENT:
                result = value1 * value2;
                break;
            default:
                result = 0.0;
        }
        

        if (write(pipe_out, &result, sizeof(result)) != sizeof(result)) break;
    }
    exit(0);
}

int create_single_process(child_process_t* process) {
    if (pipe(process->pipe_in) == -1 || pipe(process->pipe_out) == -1) {
        return -1;
    }
    
    pid_t pid = fork();
    if (pid == -1) {
        close(process->pipe_in[0]);
        close(process->pipe_in[1]);
        close(process->pipe_out[0]);
        close(process->pipe_out[1]);
        return -1;
    }
    
    if (pid == 0) {

        close(process->pipe_in[1]);
        close(process->pipe_out[0]);
        
        worker_simple_calculation(process->pipe_in[0], process->pipe_out[1]);
        
        close(process->pipe_in[0]);
        close(process->pipe_out[1]);
        exit(0);
    } else {

        process->pid = pid;
        close(process->pipe_in[0]);
        close(process->pipe_out[1]);
        return 0;
    }
}

matrix_t* add_matrices_parallel(const matrix_t* A, const matrix_t* B) {
    if (!A || !B) return NULL;
    
    if (A->rows != B->rows || A->cols != B->cols) {
        printf("Matrix dimensions don't match for parallel addition\n");
        return NULL;
    }
    
    matrix_t* result = create_matrix(A->rows, A->cols, "Addition_Result_Parallel");
    if (!result) return NULL;
    

    child_process_t temp_process;
    if (create_single_process(&temp_process) != 0) {
        printf("Failed to create temporary process, using sequential\n");
        free_matrix(result);
        return add_matrices_seq(A, B);
    }
    
    int success_count = 0;
    
    for (int i = 0; i < A->rows; i++) {
        for (int j = 0; j < A->cols; j++) {
            operation_type_t op_type = OP_ADD_ELEMENT;
            double value1 = A->data[i][j];
            double value2 = B->data[i][j];
            double element_result;
            

            if (write(temp_process.pipe_in[1], &op_type, sizeof(op_type)) != sizeof(op_type) ||
                write(temp_process.pipe_in[1], &value1, sizeof(value1)) != sizeof(value1) ||
                write(temp_process.pipe_in[1], &value2, sizeof(value2)) != sizeof(value2)) {
                continue;
            }
            

            if (read(temp_process.pipe_out[0], &element_result, sizeof(element_result)) == sizeof(element_result)) {
                result->data[i][j] = element_result;
                success_count++;
            } else {
                result->data[i][j] = value1 + value2; // fallback
            }
        }
    }
    

    kill(temp_process.pid, SIGTERM);
    waitpid(temp_process.pid, NULL, 0);
    close(temp_process.pipe_in[1]);
    close(temp_process.pipe_out[0]);
    
    printf("Parallel addition: %d/%d elements\n", success_count, A->rows * A->cols);
    return result;
}

matrix_t* subtract_matrices_parallel(const matrix_t* A, const matrix_t* B) {
    if (!A || !B) return NULL;
    
    if (A->rows != B->rows || A->cols != B->cols) {
        printf("Matrix dimensions don't match for parallel subtraction\n");
        return NULL;
    }
    
    matrix_t* result = create_matrix(A->rows, A->cols, "Subtraction_Result_Parallel");
    if (!result) return NULL;
    

    child_process_t temp_process;
    if (create_single_process(&temp_process) != 0) {
        printf("Failed to create temporary process, using sequential\n");
        free_matrix(result);
        return subtract_matrices_seq(A, B);
    }
    
    int success_count = 0;
    
    for (int i = 0; i < A->rows; i++) {
        for (int j = 0; j < A->cols; j++) {
            operation_type_t op_type = OP_SUBTRACT_ELEMENT;
            double value1 = A->data[i][j];
            double value2 = B->data[i][j];
            double element_result;
            
            if (write(temp_process.pipe_in[1], &op_type, sizeof(op_type)) != sizeof(op_type) ||
                write(temp_process.pipe_in[1], &value1, sizeof(value1)) != sizeof(value1) ||
                write(temp_process.pipe_in[1], &value2, sizeof(value2)) != sizeof(value2)) {
                continue;
            }
            
            if (read(temp_process.pipe_out[0], &element_result, sizeof(element_result)) == sizeof(element_result)) {
                result->data[i][j] = element_result;
                success_count++;
            } else {
                result->data[i][j] = value1 - value2;
            }
        }
    }
    
    kill(temp_process.pid, SIGTERM);
    waitpid(temp_process.pid, NULL, 0);
    close(temp_process.pipe_in[1]);
    close(temp_process.pipe_out[0]);
    
    printf("Parallel subtraction: %d/%d elements\n", success_count, A->rows * A->cols);
    return result;
}

matrix_t* multiply_matrices_parallel(const matrix_t* A, const matrix_t* B) {
    if (!A || !B) return NULL;
    
    if (A->cols != B->rows) {
        printf("Matrix dimensions incompatible for parallel multiplication\n");
        return NULL;
    }
    
    matrix_t* result = create_matrix(A->rows, B->cols, "Multiplication_Result_Parallel");
    if (!result) return NULL;
    

    child_process_t temp_process;
    if (create_single_process(&temp_process) != 0) {
        printf("Failed to create temporary process, using sequential\n");
        free_matrix(result);
        return multiply_matrices_seq(A, B);
    }
    
    int success_count = 0;
    int total_operations = A->rows * B->cols * A->cols;
    
    for (int i = 0; i < A->rows; i++) {
        for (int j = 0; j < B->cols; j++) {
            double sum = 0.0;
            
            for (int k = 0; k < A->cols; k++) {
                operation_type_t op_type = OP_MULTIPLY_ELEMENT;
                double value1 = A->data[i][k];
                double value2 = B->data[k][j];
                double element_result;
                
                if (write(temp_process.pipe_in[1], &op_type, sizeof(op_type)) != sizeof(op_type) ||
                    write(temp_process.pipe_in[1], &value1, sizeof(value1)) != sizeof(value1) ||
                    write(temp_process.pipe_in[1], &value2, sizeof(value2)) != sizeof(value2)) {
                    sum += value1 * value2;
                    continue;
                }
                
                if (read(temp_process.pipe_out[0], &element_result, sizeof(element_result)) == sizeof(element_result)) {
                    sum += element_result;
                    success_count++;
                } else {
                    sum += value1 * value2;
                }
            }
            
            result->data[i][j] = sum;
        }
    }
    
    kill(temp_process.pid, SIGTERM);
    waitpid(temp_process.pid, NULL, 0);
    close(temp_process.pipe_in[1]);
    close(temp_process.pipe_out[0]);
    
    printf("Parallel multiplication: %d/%d operations\n", success_count, total_operations);
    return result;
}

double matrix_determinant_parallel(const matrix_t* matrix) {
    if (!matrix || matrix->rows == 0 || matrix->cols == 0) {
        printf("Error: Matrix is empty or invalid.\n");
        return 0.0;
    }

    if (matrix->rows != matrix->cols) {
        printf("Error: Determinant is only defined for square matrices.\n");
        return 0.0;
    }

    return matrix_determinant_seq(matrix);
}


child_process_t* get_available_process() { 
    return NULL; 
}

void return_process(child_process_t* process) { 
    (void)process; 
}

void cleanup_idle_processes() {

    time_t current_time = time(NULL);
    for (int i = 0; i < global_config.max_processes; i++) {
        if (process_pool[i].pid > 0 && !process_pool[i].busy) {
            if (current_time - process_pool[i].last_used > PROCESS_TIMEOUT) {
                kill(process_pool[i].pid, SIGTERM);
                waitpid(process_pool[i].pid, NULL, 0);
                process_pool[i].pid = -1;
                active_processes--;
            }
        }
    }
}

void cleanup_old_processes() {
    cleanup_idle_processes();
}

int create_worker_process(child_process_t* process) { 
    return create_single_process(process);
}

void worker_process_main(int pipe_in, int pipe_out) { 
    worker_simple_calculation(pipe_in, pipe_out);
}
