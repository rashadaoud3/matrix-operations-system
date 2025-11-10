#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "../include/matrix_operations.h"
#include "../include/config.h"

matrix_t* matrix_registry[MAX_MATRICES];
int matrix_count = 0;
int next_matrix_id = 1;

matrix_t* create_matrix(int rows, int cols, const char* name) {
    if (rows <= 0 || cols <= 0 || rows > MAX_MATRIX_SIZE || cols > MAX_MATRIX_SIZE) {
        printf("Invalid matrix dimensions: %dx%d\n", rows, cols);
        return NULL;
    }
    
    matrix_t* matrix = (matrix_t*)malloc(sizeof(matrix_t));
    if (!matrix) {
        printf("Memory allocation failed for matrix structure\n");
        return NULL;
    }
    
    matrix->rows = rows;
    matrix->cols = cols;
    matrix->id = next_matrix_id++;
    strncpy(matrix->name, name, sizeof(matrix->name) - 1);
    matrix->name[sizeof(matrix->name) - 1] = '\0';
    
    matrix->data = (double**)malloc(rows * sizeof(double*));
    if (!matrix->data) {
        printf("Memory allocation failed for matrix rows\n");
        free(matrix);
        return NULL;
    }
    
    for (int i = 0; i < rows; i++) {
        matrix->data[i] = (double*)malloc(cols * sizeof(double));
        if (!matrix->data[i]) {
            printf("Memory allocation failed for matrix columns\n");
            for (int j = 0; j < i; j++) {
                free(matrix->data[j]);
            }
            free(matrix->data);
            free(matrix);
            return NULL;
        }
        memset(matrix->data[i], 0, cols * sizeof(double));
    }
    
    return matrix;
}

void free_matrix(matrix_t* matrix) {
    if (!matrix) return;
    
    if (matrix->data) {
        for (int i = 0; i < matrix->rows; i++) {
            if (matrix->data[i]) {
                free(matrix->data[i]);
            }
        }
        free(matrix->data);
    }
    free(matrix);
}

matrix_t* copy_matrix(const matrix_t* original) {
    if (!original) return NULL;
    
    matrix_t* copy = create_matrix(original->rows, original->cols, original->name);
    if (!copy) return NULL;
    
    for (int i = 0; i < original->rows; i++) {
        for (int j = 0; j < original->cols; j++) {
            copy->data[i][j] = original->data[i][j];
        }
    }
    
    return copy;
}

matrix_t* create_random_matrix(int rows, int cols, const char* name) {
    matrix_t* matrix = create_matrix(rows, cols, name);
    if (!matrix) return NULL;
    
    srand(time(NULL));
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            matrix->data[i][j] = (double)rand() / RAND_MAX * 10.0;
        }
    }
    
    return matrix;
}

matrix_t* add_matrices_seq(const matrix_t* A, const matrix_t* B) {
    if (!A || !B) {
        printf("Invalid matrices for addition\n");
        return NULL;
    }
    
    if (A->rows != B->rows || A->cols != B->cols) {
        printf("Matrix dimensions don't match for addition: %dx%d vs %dx%d\n", 
               A->rows, A->cols, B->rows, B->cols);
        return NULL;
    }
    
    matrix_t* result = create_matrix(A->rows, A->cols, "Addition_Result");
    if (!result) return NULL;
    
    for (int i = 0; i < A->rows; i++) {
        for (int j = 0; j < A->cols; j++) {
            result->data[i][j] = A->data[i][j] + B->data[i][j];
        }
    }
    
    return result;
}

matrix_t* subtract_matrices_seq(const matrix_t* A, const matrix_t* B) {
    if (!A || !B) {
        printf("Invalid matrices for subtraction\n");
        return NULL;
    }
    
    if (A->rows != B->rows || A->cols != B->cols) {
        printf("Matrix dimensions don't match for subtraction: %dx%d vs %dx%d\n", 
               A->rows, A->cols, B->rows, B->cols);
        return NULL;
    }
    
    matrix_t* result = create_matrix(A->rows, A->cols, "Subtraction_Result");
    if (!result) return NULL;
    
    for (int i = 0; i < A->rows; i++) {
        for (int j = 0; j < A->cols; j++) {
            result->data[i][j] = A->data[i][j] - B->data[i][j];
        }
    }
    
    return result;
}

matrix_t* multiply_matrices_seq(const matrix_t* A, const matrix_t* B) {
    if (!A || !B) {
        printf("Invalid matrices for multiplication\n");
        return NULL;
    }
    
    if (A->cols != B->rows) {
        printf("Matrix dimensions incompatible for multiplication: %dx%d vs %dx%d\n", 
               A->rows, A->cols, B->rows, B->cols);
        return NULL;
    }
    
    matrix_t* result = create_matrix(A->rows, B->cols, "Multiplication_Result");
    if (!result) return NULL;
    
    for (int i = 0; i < A->rows; i++) {
        for (int j = 0; j < B->cols; j++) {
            result->data[i][j] = 0.0;
            for (int k = 0; k < A->cols; k++) {
                result->data[i][j] += A->data[i][k] * B->data[k][j];
            }
        }
    }
    
    return result;
}

matrix_t* add_matrices_openmp(const matrix_t* A, const matrix_t* B) {
    if (!A || !B) return NULL;
    
    if (A->rows != B->rows || A->cols != B->cols) {
        printf("Matrix dimensions don't match for addition\n");
        return NULL;
    }
    
    matrix_t* result = create_matrix(A->rows, A->cols, "Addition_Result_OpenMP");
    if (!result) return NULL;
    
    #ifdef _OPENMP
    #pragma omp parallel for collapse(2)
    #endif
    for (int i = 0; i < A->rows; i++) {
        for (int j = 0; j < A->cols; j++) {
            result->data[i][j] = A->data[i][j] + B->data[i][j];
        }
    }
    
    return result;
}

matrix_t* subtract_matrices_openmp(const matrix_t* A, const matrix_t* B) {
    if (!A || !B) return NULL;
    
    if (A->rows != B->rows || A->cols != B->cols) {
        printf("Matrix dimensions don't match for subtraction\n");
        return NULL;
    }
    
    matrix_t* result = create_matrix(A->rows, A->cols, "Subtraction_Result_OpenMP");
    if (!result) return NULL;
    
    #ifdef _OPENMP
    #pragma omp parallel for collapse(2)
    #endif
    for (int i = 0; i < A->rows; i++) {
        for (int j = 0; j < A->cols; j++) {
            result->data[i][j] = A->data[i][j] - B->data[i][j];
        }
    }
    
    return result;
}

matrix_t* multiply_matrices_openmp(const matrix_t* A, const matrix_t* B) {
    if (!A || !B) return NULL;
    
    if (A->cols != B->rows) {
        printf("Matrix dimensions incompatible for multiplication\n");
        return NULL;
    }
    
    matrix_t* result = create_matrix(A->rows, B->cols, "Multiplication_Result_OpenMP");
    if (!result) return NULL;
    
    #ifdef _OPENMP
    #pragma omp parallel for collapse(2)
    #endif
    for (int i = 0; i < A->rows; i++) {
        for (int j = 0; j < B->cols; j++) {
            result->data[i][j] = 0.0;
            for (int k = 0; k < A->cols; k++) {
                result->data[i][j] += A->data[i][k] * B->data[k][j];
            }
        }
    }
    
    return result;
}



double matrix_determinant_lu(const matrix_t* matrix) {
    if (!matrix || matrix->rows == 0 || matrix->cols == 0) {
        printf("Error: Matrix is empty or invalid.\n");
        return 0.0;
    }

    if (matrix->rows != matrix->cols) {
        printf("Error: Determinant is only defined for square matrices.\n");
        return 0.0;
    }
    
    int n = matrix->rows;
    
    if (n == 1) {
        return matrix->data[0][0];
    }
    
    if (n == 2) {
        return matrix->data[0][0] * matrix->data[1][1] - 
               matrix->data[0][1] * matrix->data[1][0];
    }
    
    if (n == 3) {
        return matrix->data[0][0] * (matrix->data[1][1] * matrix->data[2][2] - matrix->data[1][2] * matrix->data[2][1]) -
               matrix->data[0][1] * (matrix->data[1][0] * matrix->data[2][2] - matrix->data[1][2] * matrix->data[2][0]) +
               matrix->data[0][2] * (matrix->data[1][0] * matrix->data[2][1] - matrix->data[1][1] * matrix->data[2][0]);
    }
    
    matrix_t* LU = copy_matrix(matrix);
    if (!LU) return 0.0;
    
    double det = 1.0;
    int* pivot = (int*)malloc(n * sizeof(int));
    
    for (int i = 0; i < n; i++) {
        pivot[i] = i;
    }
    
    for (int j = 0; j < n; j++) {
        int max_row = j;
        for (int i = j + 1; i < n; i++) {
            if (fabs(LU->data[i][j]) > fabs(LU->data[max_row][j])) {
                max_row = i;
            }
        }
        
        if (max_row != j) {
            for (int k = 0; k < n; k++) {
                double temp = LU->data[j][k];
                LU->data[j][k] = LU->data[max_row][k];
                LU->data[max_row][k] = temp;
            }
            int temp_pivot = pivot[j];
            pivot[j] = pivot[max_row];
            pivot[max_row] = temp_pivot;
            det *= -1;
        }
        
        if (fabs(LU->data[j][j]) < 1e-12) {
            free_matrix(LU);
            free(pivot);
            return 0.0;
        }
        
        det *= LU->data[j][j];
        
        for (int i = j + 1; i < n; i++) {
            LU->data[i][j] /= LU->data[j][j];
            for (int k = j + 1; k < n; k++) {
                LU->data[i][k] -= LU->data[i][j] * LU->data[j][k];
            }
        }
    }
    
    free_matrix(LU);
    free(pivot);
    return det;
}

double matrix_determinant_seq(const matrix_t* matrix) {
    return matrix_determinant_lu(matrix);
}

matrix_t* matrix_multiply(const matrix_t* A, const matrix_t* B) {
    if (!A || !B || A->cols != B->rows) return NULL;
    
    matrix_t* result = create_matrix(A->rows, B->cols, "temp");
    if (!result) return NULL;
    
    for (int i = 0; i < A->rows; i++) {
        for (int j = 0; j < B->cols; j++) {
            result->data[i][j] = 0.0;
            for (int k = 0; k < A->cols; k++) {
                result->data[i][j] += A->data[i][k] * B->data[k][j];
            }
        }
    }
    return result;
}

matrix_t* matrix_transpose(const matrix_t* A) {
    if (!A) return NULL;
    
    matrix_t* result = create_matrix(A->cols, A->rows, "transpose");
    if (!result) return NULL;
    
    for (int i = 0; i < A->rows; i++) {
        for (int j = 0; j < A->cols; j++) {
            result->data[j][i] = A->data[i][j];
        }
    }
    return result;
}

double vector_dot_product(const double* v1, const double* v2, int n) {
    double result = 0.0;
    for (int i = 0; i < n; i++) {
        result += v1[i] * v2[i];
    }
    return result;
}

double vector_norm(const double* v, int n) {
    return sqrt(vector_dot_product(v, v, n));
}

void vector_normalize(double* v, int n) {
    double norm = vector_norm(v, n);
    if (norm > 1e-12) {
        for (int i = 0; i < n; i++) {
            v[i] /= norm;
        }
    }
}

int find_dominant_eigenvalue(const matrix_t* matrix, double* eigenvalue, double* eigenvector) {
    if (!matrix || matrix->rows != matrix->cols) return -1;
    
    int n = matrix->rows;
    
    static int seeded = 0;
    if (!seeded) {
        srand(12345);
        seeded = 1;
    }
    
    for (int i = 0; i < n; i++) {
        eigenvector[i] = 1.0;
    }
    vector_normalize(eigenvector, n);
    
    double* new_vector = (double*)malloc(n * sizeof(double));
    if (!new_vector) return -1;
    
    double old_eigenvalue = 0.0;
    *eigenvalue = 0.0;
    
    int converged = 0;
    for (int iter = 0; iter < EIGEN_MAX_ITER; iter++) {
        for (int i = 0; i < n; i++) {
            new_vector[i] = 0.0;
            for (int j = 0; j < n; j++) {
                new_vector[i] += matrix->data[i][j] * eigenvector[j];
            }
        }
        
        old_eigenvalue = *eigenvalue;
        *eigenvalue = vector_dot_product(eigenvector, new_vector, n);
        
        if (iter > 0 && fabs(*eigenvalue - old_eigenvalue) < EIGEN_TOLERANCE * fabs(*eigenvalue)) {
            converged = 1;
            break;
        }
        
        vector_normalize(new_vector, n);
        memcpy(eigenvector, new_vector, n * sizeof(double));
    }
    
    free(new_vector);
    
    if (!converged) {
        printf("Warning: Power method did not converge after %d iterations\n", EIGEN_MAX_ITER);
    }
    
    return converged ? 0 : -1;
}

int find_eigenvalues_2x2(const matrix_t* matrix, eigen_t** eigenvalues, int* count) {
    if (matrix->rows != 2) return -1;
    
    *eigenvalues = (eigen_t*)malloc(2 * sizeof(eigen_t));
    (*eigenvalues)[0].eigenvector = (double*)malloc(2 * sizeof(double));
    (*eigenvalues)[1].eigenvector = (double*)malloc(2 * sizeof(double));
    
    double a = matrix->data[0][0], b = matrix->data[0][1];
    double c = matrix->data[1][0], d = matrix->data[1][1];
    
    double trace = a + d;
    double det = a * d - b * c;
    double discriminant = trace * trace - 4 * det;
    
    if (discriminant >= 0) {
        double sqrt_disc = sqrt(discriminant);
        (*eigenvalues)[0].eigenvalue = (trace + sqrt_disc) / 2.0;
        (*eigenvalues)[1].eigenvalue = (trace - sqrt_disc) / 2.0;
        
        if (fabs(b) > 1e-10) {
            (*eigenvalues)[0].eigenvector[0] = (*eigenvalues)[0].eigenvalue - d;
            (*eigenvalues)[0].eigenvector[1] = c;
            (*eigenvalues)[1].eigenvector[0] = (*eigenvalues)[1].eigenvalue - d;
            (*eigenvalues)[1].eigenvector[1] = c;
        } else if (fabs(c) > 1e-10) {
            (*eigenvalues)[0].eigenvector[0] = b;
            (*eigenvalues)[0].eigenvector[1] = (*eigenvalues)[0].eigenvalue - a;
            (*eigenvalues)[1].eigenvector[0] = b;
            (*eigenvalues)[1].eigenvector[1] = (*eigenvalues)[1].eigenvalue - a;
        } else {
            (*eigenvalues)[0].eigenvector[0] = 1.0;
            (*eigenvalues)[0].eigenvector[1] = 0.0;
            (*eigenvalues)[1].eigenvector[0] = 0.0;
            (*eigenvalues)[1].eigenvector[1] = 1.0;
        }
        
        vector_normalize((*eigenvalues)[0].eigenvector, 2);
        vector_normalize((*eigenvalues)[1].eigenvector, 2);
    } else {
        (*eigenvalues)[0].eigenvalue = trace / 2.0;
        (*eigenvalues)[1].eigenvalue = trace / 2.0;
        (*eigenvalues)[0].eigenvector[0] = 1.0;
        (*eigenvalues)[0].eigenvector[1] = 0.0;
        (*eigenvalues)[1].eigenvector[0] = 0.0;
        (*eigenvalues)[1].eigenvector[1] = 1.0;
    }
    
    *count = 2;
    return 0;
}

int find_eigenvalues_qr(const matrix_t* matrix, eigen_t** eigenvalues, int* count) {
    if (!matrix || matrix->rows != matrix->cols) return -1;
    
    int n = matrix->rows;
    
    if (n == 1) {
        *eigenvalues = (eigen_t*)malloc(sizeof(eigen_t));
        (*eigenvalues)[0].eigenvector = (double*)malloc(sizeof(double));
        (*eigenvalues)[0].eigenvalue = matrix->data[0][0];
        (*eigenvalues)[0].eigenvector[0] = 1.0;
        *count = 1;
        return 0;
    }
    else if (n == 2) {
        return find_eigenvalues_2x2(matrix, eigenvalues, count);
    }
    else {
        printf("Matrix size %dx%d - Using Power Method for dominant eigenvalue\n", n, n);
        *eigenvalues = (eigen_t*)malloc(sizeof(eigen_t));
        (*eigenvalues)[0].eigenvector = (double*)malloc(n * sizeof(double));
        
        if (find_dominant_eigenvalue(matrix, &(*eigenvalues)[0].eigenvalue, 
                                   (*eigenvalues)[0].eigenvector) == 0) {
            *count = 1;
            return 0;
        } else {
            free((*eigenvalues)[0].eigenvector);
            free(*eigenvalues);
            return -1;
        }
    }
}

int find_eigenvalues_eigenvectors(const matrix_t* matrix, eigen_t** eigenvalues, int* count) {
    return find_eigenvalues_qr(matrix, eigenvalues, count);
}

void verify_eigen_results(const matrix_t* matrix, const eigen_t* eigenvalues, int count) {
    printf("\nVerification (A*v - lambda*v):\n");
    for (int i = 0; i < count; i++) {
        double* Av = (double*)malloc(matrix->rows * sizeof(double));
        for (int j = 0; j < matrix->rows; j++) {
            Av[j] = 0.0;
            for (int k = 0; k < matrix->rows; k++) {
                Av[j] += matrix->data[j][k] * eigenvalues[i].eigenvector[k];
            }
        }
        
        double max_error = 0.0;
        for (int j = 0; j < matrix->rows; j++) {
            double error = fabs(Av[j] - eigenvalues[i].eigenvalue * eigenvalues[i].eigenvector[j]);
            if (error > max_error) max_error = error;
        }
        
        printf("Eigenvalue %d: max error = %.2e %s\n", 
               i+1, max_error, max_error < 1e-8 ? "✓" : "✗");
        free(Av);
    }
}

void free_eigen_results(eigen_t* eigenvalues, int count) {
    if (!eigenvalues) return;
    
    for (int i = 0; i < count; i++) {
        if (eigenvalues[i].eigenvector) {
            free(eigenvalues[i].eigenvector);
        }
    }
    free(eigenvalues);
}

void display_eigen_results(const eigen_t* eigenvalues, int count, int matrix_size) {
    printf("\n=== EIGENVALUES & EIGENVECTORS ===\n");
    
    for (int i = 0; i < count; i++) {
        printf("Eigenvalue %d: %.6f\n", i + 1, eigenvalues[i].eigenvalue);
        printf("Eigenvector %d: [", i + 1);
        for (int j = 0; j < matrix_size; j++) {
            printf("%.4f", eigenvalues[i].eigenvector[j]);
            if (j < matrix_size - 1) printf(", ");
        }
        printf("]\n\n");
    }
    printf("===================================\n");
}

int add_matrix_to_registry(matrix_t* matrix) {
    if (!matrix) return -1;
    
    if (matrix_count >= global_config.max_matrices) {
        printf("Matrix registry full! Cannot add more matrices.\n");
        return -1;
    }
    
    for (int i = 0; i < MAX_MATRICES; i++) {
        if (matrix_registry[i] == NULL) {
            matrix_registry[i] = matrix;
            matrix_count++;
            return matrix->id;
        }
    }
    
    return -1;
}

void remove_matrix_from_registry(int matrix_id) {
    for (int i = 0; i < MAX_MATRICES; i++) {
        if (matrix_registry[i] && matrix_registry[i]->id == matrix_id) {
            printf("Deleting matrix: %s (ID: %d)\n", matrix_registry[i]->name, matrix_id);
            free_matrix(matrix_registry[i]);
            matrix_registry[i] = NULL;
            matrix_count--;
            return;
        }
    }
    printf("Matrix with ID %d not found in registry\n", matrix_id);
}

matrix_t* find_matrix_by_id(int matrix_id) {
    for (int i = 0; i < MAX_MATRICES; i++) {
        if (matrix_registry[i] && matrix_registry[i]->id == matrix_id) {
            return matrix_registry[i];
        }
    }
    return NULL;
}

void display_matrix(const matrix_t* matrix) {
    if (!matrix) {
        printf("Invalid matrix\n");
        return;
    }
    
    printf("\nMatrix: %s (ID: %d, %dx%d)\n", matrix->name, matrix->id, matrix->rows, matrix->cols);
    printf("┌");
    for (int j = 0; j < matrix->cols; j++) printf("──────────");
    printf("┐\n");
    
    for (int i = 0; i < matrix->rows; i++) {
        printf("│");
        for (int j = 0; j < matrix->cols; j++) {
            printf(" %8.2f ", matrix->data[i][j]);
        }
        printf("│\n");
    }
    
    printf("└");
    for (int j = 0; j < matrix->cols; j++) printf("──────────");
    printf("┘\n");
}

void display_all_matrices() {
    if (matrix_count == 0) {
        printf("\nNo matrices in registry.\n");
        return;
    }
    
    printf("\n=== MATRIX REGISTRY (%d matrices) ===\n", matrix_count);
    for (int i = 0; i < MAX_MATRICES; i++) {
        if (matrix_registry[i]) {
            printf("ID: %2d | %-20s | %2dx%-2d\n", 
                   matrix_registry[i]->id, 
                   matrix_registry[i]->name, 
                   matrix_registry[i]->rows, 
                   matrix_registry[i]->cols);
        }
    }
    printf("====================================\n");
}

void clear_matrix_registry() {
    printf("Clearing matrix registry...\n");
    for (int i = 0; i < MAX_MATRICES; i++) {
        if (matrix_registry[i]) {
            free_matrix(matrix_registry[i]);
            matrix_registry[i] = NULL;
        }
    }
    matrix_count = 0;
    next_matrix_id = 1;
}

void start_timer(performance_timer_t* timer) {
    timer->start_time = (double)clock() / CLOCKS_PER_SEC;
}

void stop_timer(performance_timer_t* timer) {
    timer->end_time = (double)clock() / CLOCKS_PER_SEC;
}

double get_elapsed_time(const performance_timer_t* timer) {
    return timer->end_time - timer->start_time;
}

void print_performance_stats(const performance_timer_t* sequential,
                           const performance_timer_t* parallel,
                           const performance_timer_t* openmp) {
    printf("\n=== PERFORMANCE COMPARISON ===\n");
    
    if (sequential) {
        printf("Sequential:  %10.6f seconds\n", get_elapsed_time(sequential));
    }
    
    if (parallel) {
        printf("Parallel:    %10.6f seconds\n", get_elapsed_time(parallel));
        if (sequential) {
            double speedup = get_elapsed_time(sequential) / get_elapsed_time(parallel);
            printf("Speedup:     %10.2fx\n", speedup);
        }
    }
    
    if (openmp) {
        printf("OpenMP:      %10.6f seconds\n", get_elapsed_time(openmp));
        if (sequential) {
            double speedup = get_elapsed_time(sequential) / get_elapsed_time(openmp);
            printf("Speedup:     %10.2fx\n", speedup);
        }
    }
    
    printf("================================\n");
}




double matrix_determinant_openmp(const matrix_t* matrix) {

    if (!matrix || matrix->rows != matrix->cols) {
        printf("Invalid matrix for determinant calculation\n");
        return 0.0;
    }
    
    int n = matrix->rows;
    

    if (n == 1) {
        return matrix->data[0][0];
    }
    
    if (n == 2) {
        return matrix->data[0][0] * matrix->data[1][1] - 
               matrix->data[0][1] * matrix->data[1][0];
    }
    
    if (n == 3) {

        return matrix->data[0][0] * (matrix->data[1][1] * matrix->data[2][2] - matrix->data[1][2] * matrix->data[2][1]) -
               matrix->data[0][1] * (matrix->data[1][0] * matrix->data[2][2] - matrix->data[1][2] * matrix->data[2][0]) +
               matrix->data[0][2] * (matrix->data[1][0] * matrix->data[2][1] - matrix->data[1][1] * matrix->data[2][0]);
    }
    

    matrix_t* LU = copy_matrix(matrix);
    if (!LU) return 0.0;
    
    double det = 1.0;
    int* pivot = (int*)malloc(n * sizeof(int));
    if (!pivot) {
        free_matrix(LU);
        return 0.0;
    }
    

    for (int i = 0; i < n; i++) {
        pivot[i] = i;
    }
    

    for (int j = 0; j < n; j++) {
        int max_row = j;
        
        // Find pivot row with maximum element in current column
        #ifdef _OPENMP
        #pragma omp parallel for if(is_openmp_enabled())
        #endif
        for (int i = j + 1; i < n; i++) {
            if (fabs(LU->data[i][j]) > fabs(LU->data[max_row][j])) {
                #ifdef _OPENMP
                #pragma omp critical
                #endif
                {
                    if (fabs(LU->data[i][j]) > fabs(LU->data[max_row][j])) {
                        max_row = i;
                    }
                }
            }
        }
        
        // Swap rows if necessary
        if (max_row != j) {
            // Swap rows in parallel
            #ifdef _OPENMP
            #pragma omp parallel for if(is_openmp_enabled())
            #endif
            for (int k = 0; k < n; k++) {
                double temp = LU->data[j][k];
                LU->data[j][k] = LU->data[max_row][k];
                LU->data[max_row][k] = temp;
            }
            

            int temp_pivot = pivot[j];
            pivot[j] = pivot[max_row];
            pivot[max_row] = temp_pivot;
            det *= -1; 
        }
        
        // Check for singular matrix
        if (fabs(LU->data[j][j]) < 1e-12) {
            free_matrix(LU);
            free(pivot);
            return 0.0; // Matrix is singular
        }
        

        det *= LU->data[j][j];
        
        // Update lower rows in parallel
        #ifdef _OPENMP
        #pragma omp parallel for if(is_openmp_enabled())
        #endif
        for (int i = j + 1; i < n; i++) {
            LU->data[i][j] /= LU->data[j][j]; 
            for (int k = j + 1; k < n; k++) {
                LU->data[i][k] -= LU->data[i][j] * LU->data[j][k]; 
            }
        }
    }
    

    free_matrix(LU);
    free(pivot);
    
    return det;
}
