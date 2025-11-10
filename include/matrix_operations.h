#ifndef MATRIX_OPERATIONS_H
#define MATRIX_OPERATIONS_H

#include <time.h>

#define MAX_MATRICES 200
#define MAX_MATRIX_SIZE 100
#define EIGEN_MAX_ITER 1000
#define EIGEN_TOLERANCE 1e-10

typedef struct {
    int rows;
    int cols;
    char name[50];
    int id;
    double** data;
} matrix_t;

typedef struct {
    double start_time;
    double end_time;
} performance_timer_t;

typedef struct {
    double eigenvalue;
    double* eigenvector;
} eigen_t;

extern int use_openmp_flag;
extern matrix_t* matrix_registry[MAX_MATRICES];
extern int matrix_count;
extern int next_matrix_id;
double matrix_determinant_openmp(const matrix_t* matrix);
matrix_t* create_matrix(int rows, int cols, const char* name);
void free_matrix(matrix_t* matrix);
matrix_t* copy_matrix(const matrix_t* original);
matrix_t* create_random_matrix(int rows, int cols, const char* name);

matrix_t* add_matrices_seq(const matrix_t* A, const matrix_t* B);
matrix_t* subtract_matrices_seq(const matrix_t* A, const matrix_t* B);
matrix_t* multiply_matrices_seq(const matrix_t* A, const matrix_t* B);

matrix_t* add_matrices_openmp(const matrix_t* A, const matrix_t* B);
matrix_t* subtract_matrices_openmp(const matrix_t* A, const matrix_t* B);
matrix_t* multiply_matrices_openmp(const matrix_t* A, const matrix_t* B);

double matrix_determinant_seq(const matrix_t* matrix);
double matrix_determinant_lu(const matrix_t* matrix);

int find_eigenvalues_eigenvectors(const matrix_t* matrix, eigen_t** eigenvalues, int* count);
int find_eigenvalues_qr(const matrix_t* matrix, eigen_t** eigenvalues, int* count);
int find_dominant_eigenvalue(const matrix_t* matrix, double* eigenvalue, double* eigenvector);
void free_eigen_results(eigen_t* eigenvalues, int count);
void display_eigen_results(const eigen_t* eigenvalues, int count, int matrix_size);

matrix_t* matrix_transpose(const matrix_t* A);
matrix_t* matrix_multiply(const matrix_t* A, const matrix_t* B);
double vector_dot_product(const double* v1, const double* v2, int n);
double vector_norm(const double* v, int n);
void vector_normalize(double* v, int n);

int add_matrix_to_registry(matrix_t* matrix);
void remove_matrix_from_registry(int matrix_id);
matrix_t* find_matrix_by_id(int matrix_id);
void display_matrix(const matrix_t* matrix);
void display_all_matrices();
void clear_matrix_registry();
void debug_file_content(const char* filename);

int is_openmp_enabled(void);
void start_timer(performance_timer_t* timer);
void stop_timer(performance_timer_t* timer);
double get_elapsed_time(const performance_timer_t* timer);
void print_performance_stats(const performance_timer_t* sequential,
                           const performance_timer_t* parallel,
                           const performance_timer_t* openmp);
                           

#endif
