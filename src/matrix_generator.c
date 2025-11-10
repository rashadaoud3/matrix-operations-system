#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>
#include "../include/matrix_operations.h"
#include "../include/config.h"
#include "../include/matrix_generator.h"

void generate_random_matrices(int count, int min_size, int max_size) {
    if (count <= 0) {
        printf("Invalid matrix count: %d\n", count);
        return;
    }
    
    if (min_size <= 0 || max_size <= 0 || min_size > max_size) {
        printf("Invalid size range: %d-%d\n", min_size, max_size);
        return;
    }
    
    struct stat st = {0};
    if (stat(global_config.matrix_directory, &st) == -1) {
        if (mkdir(global_config.matrix_directory, 0755) == -1) {
            printf("Error: Cannot create directory '%s'\n", global_config.matrix_directory);
            return;
        }
        printf("Created directory: %s\n", global_config.matrix_directory);
    }
    
    printf("Generating %d random matrices (%dx%d to %dx%d) in '%s'\n", 
           count, min_size, min_size, max_size, max_size, global_config.matrix_directory);
    
    srand(time(NULL));
    int generated = 0;
    
    for (int i = 0; i < count; i++) {
        int rows = rand() % (max_size - min_size + 1) + min_size;
        int cols = rand() % (max_size - min_size + 1) + min_size;
        
        char matrix_name[50];
        snprintf(matrix_name, sizeof(matrix_name), "Random_%dx%d_%d", rows, cols, i+1);
        
        matrix_t* matrix = create_matrix(rows, cols, matrix_name);
        if (!matrix) {
            printf("Failed to create matrix %d\n", i+1);
            continue;
        }
        
        #ifdef _OPENMP
        #pragma omp parallel for collapse(2)
        #endif
        for (int r = 0; r < rows; r++) {
            for (int c = 0; c < cols; c++) {
                matrix->data[r][c] = (double)(rand() % 2000 - 1000) / 100.0;
            }
        }
        
        char filename[MAX_FILENAME];

        int path_len = snprintf(filename, MAX_FILENAME - 1, "%s/%s.txt", 
                global_config.matrix_directory, matrix_name);
        
        if (path_len >= MAX_FILENAME) {

            snprintf(filename, MAX_FILENAME - 1, "%s/matrix_%d.txt", 
                    global_config.matrix_directory, i+1);
        }
        
        FILE* file = fopen(filename, "w");
        if (file) {
            fprintf(file, "# Random Matrix %dx%d\n", rows, cols);
            fprintf(file, "# Generated automatically\n");
            for (int r = 0; r < rows; r++) {
                for (int c = 0; c < cols; c++) {
                    fprintf(file, "%.2f ", matrix->data[r][c]);
                }
                fprintf(file, "\n");
            }
            fclose(file);
            
            if (matrix_count < global_config.max_matrices) {
                if (add_matrix_to_registry(matrix) >= 0) {
                    generated++;
                    printf("✓ Generated: %s (%dx%d)\n", matrix_name, rows, cols);
                } else {
                    free_matrix(matrix);
                }
            } else {
                free_matrix(matrix);
                printf("Matrix registry full, saved to file only: %s\n", filename);
            }
        } else {
            printf("Error: Cannot save matrix to %s\n", filename);
            free_matrix(matrix);
        }
    }
    
    printf("Successfully generated %d random matrices\n", generated);
}

void handle_random_matrix_generation() {
    printf("\n=== RANDOM MATRIX GENERATION ===\n");
    
    int count, min_size, max_size;
    printf("Number of matrices to generate: ");
    if (scanf("%d", &count) != 1 || count <= 0) {
        printf("Invalid count!\n");
        return;
    }
    
    printf("Minimum matrix size: ");
    if (scanf("%d", &min_size) != 1 || min_size <= 0) {
        printf("Invalid minimum size!\n");
        return;
    }
    
    printf("Maximum matrix size: ");
    if (scanf("%d", &max_size) != 1 || max_size <= 0 || max_size < min_size) {
        printf("Invalid maximum size!\n");
        return;
    }
    
    printf("\nGenerating %d matrices of size %d-%d...\n", count, min_size, max_size);
    generate_random_matrices(count, min_size, max_size);
}
