#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include "../include/menu_interface.h"
#include "../include/matrix_operations.h"
#include "../include/openmp_utils.h"
#include "../include/config.h"
#include "../include/process_management.h"
#include "../include/file_operations.h"
#include "../include/matrix_generator.h"

extern int use_openmp_flag;

void display_main_menu() {
    printf("\n╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                   MATRIX OPERATIONS SYSTEM                  ║\n");
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    
    if (global_config.custom_menu) {
        for (int i = 0; i < MENU_ITEMS; i++) {
            int choice_num = global_config.menu_order[i];
            const char* menu_text = get_menu_item_name(choice_num);
            
            if (choice_num == 17) {
                printf("║ %2d. %-52s [%s] ║\n", 
                       i + 1, menu_text, use_openmp_flag ? "ON " : "OFF");
            } else {
                printf("║ %2d. %-58s ║\n", i + 1, menu_text);
            }
        }
    } else {
        printf("║  1. Enter a matrix                                          ║\n");
        printf("║  2. Display a matrix                                        ║\n");
        printf("║  3. Delete a matrix                                         ║\n");
        printf("║  4. Modify a matrix                                         ║\n");
        printf("║  5. Read matrix from file                                   ║\n");
        printf("║  6. Read matrices from folder                               ║\n");
        printf("║  7. Save matrix to file                                     ║\n");
        printf("║  8. Save all matrices to folder                             ║\n");
        printf("║  9. Display all matrices                                    ║\n");
        printf("║ 10. Add two matrices                                        ║\n");
        printf("║ 11. Subtract two matrices                                   ║\n");
        printf("║ 12. Multiply two matrices                                   ║\n");
        printf("║ 13. Find determinant                                        ║\n");
        printf("║ 14. Find eigenvalues & eigenvectors of a matrix.            ║\n");
        printf("║ 15. Generate random matrices                                ║\n");
        printf("║ 16. Performance comparison                                  ║\n");
        printf("║ 17. Toggle OpenMP [%s]                                      ║\n", 
               use_openmp_flag ? "ON " : "OFF");
        printf("║ 18. Exit                                                    ║\n");
    }
    
    printf("╚══════════════════════════════════════════════════════════════╝\n");
}

int handle_menu_choice(int choice) {
    if (choice < 1 || choice > MENU_ITEMS) {
        printf("Invalid choice! Please select 1-%d.\n", MENU_ITEMS);
        return 0;
    }
    
    int actual_choice = global_config.custom_menu ? global_config.menu_order[choice - 1] : choice;
    int should_exit = 0;
    
    switch (actual_choice) {
        case 1: handle_matrix_input(); break;
        case 2: handle_matrix_display(); break;
        case 3: handle_matrix_deletion(); break;
        case 4: handle_matrix_modification(); break;
        case 5: 
            printf("Enter filename: ");
            char filename[MAX_FILENAME];
            if (scanf("%255s", filename) == 1) {
                matrix_t* matrix = read_matrix_from_file(filename);
                if (matrix) {
                    if (add_matrix_to_registry(matrix) >= 0) {
                        printf("✓ Matrix loaded successfully (ID: %d)\n", matrix->id);
                    } else {
                        free_matrix(matrix);
                        printf("✗ Failed to add matrix to registry (registry full)\n");
                    }
                } else {
                    printf("✗ Failed to read matrix from file\n");
                }
            } else {
                printf("Invalid filename!\n");
                clear_input_buffer();
            }
            break;
        case 6: 
            {
                printf("Enter folder path [default: %s]: ", global_config.matrix_directory);
                char folder[MAX_FILENAME];
                clear_input_buffer();
                if (fgets(folder, sizeof(folder), stdin)) {
                    size_t len = strlen(folder);
                    if (len > 0 && folder[len-1] == '\n') {
                        folder[len-1] = '\0';
                    }
                    
                    char *trimmed_path = folder;
                    while (*trimmed_path == ' ') trimmed_path++; 
                    len = strlen(trimmed_path);
                    while (len > 0 && trimmed_path[len-1] == ' ') {
                        trimmed_path[len-1] = '\0'; 
                        len--;
                    }
                    
                    const char* actual_path;
                    if (strlen(trimmed_path) > 0) {
                        actual_path = trimmed_path;
                    } else {
                        actual_path = global_config.matrix_directory;
                    }
                    
                    int loaded = read_matrices_from_folder(actual_path);
                    if (loaded > 0) {
                        printf("✓ Successfully loaded %d matrices from '%s'\n", loaded, actual_path);
                    } else {
                        printf("✗ No matrices loaded from '%s'\n", actual_path);
                    }
                }
            }
            break;
        case 7:
            printf("Enter matrix ID to save: ");
            int save_id;
            if (scanf("%d", &save_id) == 1) {
                printf("Enter filename: ");
                char save_file[MAX_FILENAME];
                if (scanf("%255s", save_file) == 1) {
                    matrix_t* matrix = find_matrix_by_id(save_id);
                    if (matrix) {
                        if (write_matrix_to_file(matrix, save_file) == 0) {
                            printf("✓ Matrix saved successfully\n");
                        } else {
                            printf("✗ Failed to save matrix\n");
                        }
                    } else {
                        printf("Matrix not found!\n");
                    }
                } else {
                    printf("Invalid filename!\n");
                    clear_input_buffer();
                }
            } else {
                printf("Invalid matrix ID!\n");
                clear_input_buffer();
            }
            break;
        case 8:
            {
                printf("Enter folder path [default: %s]: ", global_config.matrix_directory);
                char folder[MAX_FILENAME];
                clear_input_buffer();
                if (fgets(folder, sizeof(folder), stdin)) {
                    folder[strcspn(folder, "\n")] = 0; 
                    
                    const char* path_to_use;
                    if (strlen(folder) > 0) {
                        path_to_use = folder;
                    } else {
                        path_to_use = global_config.matrix_directory;
                    }
                    
                    int saved = write_matrices_to_folder(path_to_use);
                    if (saved > 0) {
                        printf("✓ Successfully saved %d matrices to '%s'\n", saved, path_to_use);
                    } else {
                        printf("✗ No matrices saved to '%s'\n", path_to_use);
                    }
                }
            }
            break;
        case 9: 
            display_all_matrices(); 
            break;
        case 10: 
            handle_matrix_operations(1); // Addition
            break;
        case 11: 
            handle_matrix_operations(2); // Subtraction
            break;
        case 12: 
            handle_matrix_operations(3); // Multiplication
            break;
        case 13: 
            handle_determinant_calculation(); 
            break;
        case 14: 
            handle_eigen_calculation();  
            break; 
        case 15: 
            handle_random_matrix_generation(); 
            break;
        case 16: 
            handle_performance_comparison(); 
            break;
        case 17: 
            handle_openmp_toggle(); 
            break;
        case 18: 
            printf("Exiting program...\n");
            cleanup_process_pool();
            clear_matrix_registry();
            should_exit = 1;
            break;
        default: 
            printf("Invalid menu option!\n");
    }
    
    return should_exit;
}

void handle_matrix_operations(int op_type) {
    printf("\n=== MATRIX OPERATIONS ===\n");
    display_all_matrices();
    
    if (matrix_count < 2) {
        printf("Need at least 2 matrices for operations!\n");
        return;
    }
    
    const char* operation_name = "";
    switch (op_type) {
        case 1: operation_name = "addition"; break;
        case 2: operation_name = "subtraction"; break; 
        case 3: operation_name = "multiplication"; break;
        default: 
            printf("Invalid operation type!\n");
            return;
    }
    
    printf("Performing matrix %s...\n", operation_name);
    
    int matrix1_id = get_user_choice("Enter first matrix ID", 1, next_matrix_id - 1);
    int matrix2_id = get_user_choice("Enter second matrix ID", 1, next_matrix_id - 1);
    
    matrix_t* A = find_matrix_by_id(matrix1_id);
    matrix_t* B = find_matrix_by_id(matrix2_id);
    
    if (!A || !B) {
        printf("One or both matrices not found!\n");
        return;
    }
    
    printf("\nComputation method:\n");
    printf("1. Sequential\n");
    printf("2. Parallel (Processes)\n"); 
    printf("3. OpenMP\n");
    int method = get_user_choice("Select method", 1, 3);
    
    performance_timer_t timer;
    start_timer(&timer);
    
    matrix_t* result = NULL;
    const char* method_name = "";
    
    switch (op_type) {
        case 1:
            if (method == 1) {
                result = add_matrices_seq(A, B);
                method_name = "sequential";
            } else if (method == 2) {
                result = add_matrices_parallel(A, B);
                method_name = "parallel processes";
            } else {
                result = add_matrices_openmp(A, B);
                method_name = "OpenMP";
            }
            break;
        case 2:
            if (method == 1) {
                result = subtract_matrices_seq(A, B);
                method_name = "sequential";
            } else if (method == 2) {
                result = subtract_matrices_parallel(A, B);
                method_name = "parallel processes";
            } else {
                result = subtract_matrices_openmp(A, B);
                method_name = "OpenMP";
            }
            break;
        case 3:
            if (method == 1) {
                result = multiply_matrices_seq(A, B);
                method_name = "sequential";
            } else if (method == 2) {
                result = multiply_matrices_parallel(A, B);
                method_name = "parallel processes";
            } else {
                result = multiply_matrices_openmp(A, B);
                method_name = "OpenMP";
            }
            break;
    }
    
    stop_timer(&timer);
    
    if (result) {
        char result_name[50];
        snprintf(result_name, sizeof(result_name), "Result_%s_%d_%d", operation_name, matrix1_id, matrix2_id);
        strcpy(result->name, result_name);
        
        if (add_matrix_to_registry(result) >= 0) {
            printf("✓ %s completed in %.6f seconds using %s\n", 
                   operation_name, get_elapsed_time(&timer), method_name);
            printf("Result matrix (ID: %d):\n", result->id);
            display_matrix(result);
        } else {
            free_matrix(result);
            printf("✗ Failed to add result matrix to registry\n");
        }
    } else {
        printf("✗ Operation failed! Check matrix dimensions.\n");
    }
}

void handle_eigen_calculation() {
    printf("\n=== EIGENVALUES & EIGENVECTORS CALCULATION ===\n");
    display_all_matrices();
    
    if (matrix_count == 0) {
        printf("No matrices available!\n");
        return;
    }
    
    int matrix_id = get_user_choice("Enter matrix ID", 1, next_matrix_id - 1);
    matrix_t* matrix = find_matrix_by_id(matrix_id);
    
    if (!matrix) {
        printf("Matrix not found!\n");
        return;
    }
    
    if (matrix->rows != matrix->cols) {
        printf("Eigen calculation requires square matrix!\n");
        return;
    }
    
    printf("\nCalculation methods:\n");
    printf("1. Quick (QR Algorithm - for small matrices)\n");
    printf("2. Accurate (All eigenvalues for 2x2, dominant for larger)\n");
    printf("3. Dominant Only (Power Method - for large matrices)\n");
    int method = get_user_choice("Select method", 1, 3);
    
    printf("Calculating eigenvalues and eigenvectors for matrix %d (%dx%d)...\n", 
           matrix_id, matrix->rows, matrix->rows);
    
    performance_timer_t timer;
    start_timer(&timer);
    
    eigen_t* eigenvalues = NULL;
    int eigen_count = 0;
    int result = -1;
    
    switch (method) {
        case 1:
            if (matrix->rows <= 10) {
                result = find_eigenvalues_qr(matrix, &eigenvalues, &eigen_count);
            } else {
                printf("Matrix too large for QR, using Power Method instead.\n");
                result = find_eigenvalues_eigenvectors(matrix, &eigenvalues, &eigen_count);
            }
            break;
        case 2:
            result = find_eigenvalues_eigenvectors(matrix, &eigenvalues, &eigen_count);
            break;
        case 3:
            if (matrix->rows > 2) {
                result = find_eigenvalues_eigenvectors(matrix, &eigenvalues, &eigen_count);
            } else {
                printf("Power Method is for larger matrices. Using QR instead.\n");
                result = find_eigenvalues_qr(matrix, &eigenvalues, &eigen_count);
            }
            break;
    }
    
    stop_timer(&timer);
    
    if (result == 0 && eigenvalues) {
        display_eigen_results(eigenvalues, eigen_count, matrix->rows);
        printf("Calculation time: %.6f seconds\n", get_elapsed_time(&timer));
        
        const char* method_name = "";
        switch (method) {
            case 1: method_name = "Quick (QR)"; break;
            case 2: method_name = "Accurate"; break;
            case 3: method_name = "Dominant (Power Method)"; break;
        }
        printf("Method: %s\n", method_name);
        
        free_eigen_results(eigenvalues, eigen_count);
    } else {
        printf("✗ Failed to calculate eigenvalues and eigenvectors\n");
    }
}
void handle_matrix_input() {
    printf("\n=== ENTER NEW MATRIX ===\n");
    
    char name[50];
    int rows, cols;
    
    printf("Matrix name: ");
    if (scanf("%49s", name) != 1) {
        printf("Invalid name!\n");
        clear_input_buffer();
        return;
    }
    
    printf("Rows: ");
    if (scanf("%d", &rows) != 1 || rows <= 0 || rows > MAX_MATRIX_SIZE) {
        printf("Invalid rows! Must be between 1 and %d\n", MAX_MATRIX_SIZE);
        clear_input_buffer();
        return;
    }
    
    printf("Columns: ");
    if (scanf("%d", &cols) != 1 || cols <= 0 || cols > MAX_MATRIX_SIZE) {
        printf("Invalid columns! Must be between 1 and %d\n", MAX_MATRIX_SIZE);
        clear_input_buffer();
        return;
    }
    
    matrix_t* matrix = create_matrix(rows, cols, name);
    if (!matrix) {
        printf("Failed to create matrix!\n");
        return;
    }
    
    printf("Enter matrix elements (%dx%d):\n", rows, cols);
    for (int i = 0; i < rows; i++) {
        printf("Row %d: ", i + 1);
        for (int j = 0; j < cols; j++) {
            if (scanf("%lf", &matrix->data[i][j]) != 1) {
                printf("Invalid element [%d][%d]!\n", i, j);
                free_matrix(matrix);
                clear_input_buffer();
                return;
            }
        }
    }
    
    if (add_matrix_to_registry(matrix) >= 0) {
        printf("✓ Matrix '%s' created successfully (ID: %d)\n", name, matrix->id);
    } else {
        free_matrix(matrix);
        printf("✗ Failed to add matrix to registry (registry full)!\n");
    }
}

void handle_matrix_display() {
    printf("\n=== DISPLAY MATRIX ===\n");
    display_all_matrices();
    
    if (matrix_count == 0) {
        printf("No matrices available!\n");
        return;
    }
    
    int matrix_id = get_user_choice("Enter matrix ID", 1, next_matrix_id - 1);
    matrix_t* matrix = find_matrix_by_id(matrix_id);
    
    if (matrix) {
        display_matrix(matrix);
    } else {
        printf("Matrix ID %d not found!\n", matrix_id);
    }
}

void list_files_in_directory(const char* path) {
    DIR* dir = opendir(path);
    if (!dir) {
        printf("Cannot open directory: %s\n", path);
        return;
    }
    
    struct dirent* entry;
    int file_count = 0;
    printf("Files in %s:\n", path);
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            printf("  - %s\n", entry->d_name);
            file_count++;
        }
    }
    
    if (file_count == 0) {
        printf("  (No files found)\n");
    }
    
    closedir(dir);
}

void delete_matrix_from_folder_helper(matrix_t* matrix, const char* folder_path, const char* folder_type) {
    if (!matrix || !folder_path) {
        printf("Invalid matrix or folder path!\n");
        return;
    }
    
    char filename[MAX_FILENAME];
    int deleted = 0;
    

    int path_len = snprintf(filename, MAX_FILENAME - 1, "%s/%s.txt", folder_path, matrix->name);
    if (path_len < MAX_FILENAME) {
        if (remove(filename) == 0) {
            printf("✓ Matrix deleted from %s: %s\n", folder_type, filename);
            deleted = 1;
        }
    }
    
    if (!deleted) {
        DIR* dir = opendir(folder_path);
        if (dir) {
            struct dirent* entry;
            while ((entry = readdir(dir)) != NULL) {
                if (entry->d_type == DT_REG && strstr(entry->d_name, matrix->name) != NULL) {
                    path_len = snprintf(filename, MAX_FILENAME - 1, "%s/%s", folder_path, entry->d_name);
                    if (path_len < MAX_FILENAME && remove(filename) == 0) {
                        printf("✓ Matrix deleted from %s: %s\n", folder_type, filename);
                        deleted = 1;
                        break;
                    }
                }
            }
            closedir(dir);
        }
    }
    
    if (!deleted) {
        printf("✗ Could not find file to delete for matrix '%s' in %s: %s\n", 
               matrix->name, folder_type, folder_path);
        printf("File may have been already deleted or has different name.\n");
        
        printf("Available files in folder:\n");
        list_files_in_directory(folder_path);
    }
}
void handle_matrix_deletion() {
    printf("\n=== DELETE MATRIX ===\n");
    display_all_matrices();
    
    if (matrix_count == 0) {
        printf("No matrices available!\n");
        return;
    }
    
    int matrix_id = get_user_choice("Enter matrix ID to delete", 1, next_matrix_id - 1);
    
    printf("\nDelete options:\n");
    printf("1. Delete from memory only\n");
    printf("2. Delete from default folder only\n");
    printf("3. Delete from both memory and default folder\n");
    printf("4. Delete from external folder\n");
    printf("5. Cancel deletion\n");
    
    int delete_choice = get_user_choice("Select deletion type", 1, 5);
    
    if (delete_choice == 5) {
        printf("Deletion cancelled.\n");
        return;
    }
    
    matrix_t* matrix = find_matrix_by_id(matrix_id);
    if (!matrix) {
        printf("Matrix not found!\n");
        return;
    }
    
    if (delete_choice == 1 || delete_choice == 3) {
        remove_matrix_from_registry(matrix_id);
        printf("✓ Matrix deleted from memory\n");
    }
    
    if (delete_choice == 2 || delete_choice == 3) {
        delete_matrix_from_folder_helper(matrix, global_config.matrix_directory, "default folder");
    }
    
    if (delete_choice == 4) {
        printf("Enter external folder path: ");
        char external_folder[MAX_FILENAME];
        clear_input_buffer();
        if (fgets(external_folder, sizeof(external_folder), stdin)) {
            external_folder[strcspn(external_folder, "\n")] = 0;
            
            DIR* dir = opendir(external_folder);
            if (!dir) {
                printf("Error: Cannot access folder '%s'\n", external_folder);
                printf("Please make sure the folder exists and you have permission to access it.\n");
                return;
            }
            closedir(dir);
            
            delete_matrix_from_folder_helper(matrix, external_folder, "external folder");
        }
    }
}

void handle_matrix_modification() {
    printf("\n=== MODIFY MATRIX ===\n");
    display_all_matrices();
    
    if (matrix_count == 0) {
        printf("No matrices available!\n");
        return;
    }
    
    int matrix_id = get_user_choice("Enter matrix ID to modify", 1, next_matrix_id - 1);
    matrix_t* matrix = find_matrix_by_id(matrix_id);
    
    if (!matrix) {
        printf("Matrix not found!\n");
        return;
    }
    
    display_matrix(matrix);
    
    printf("\nModification options:\n");
    printf("1. Modify single element\n");
    printf("2. Modify entire row\n");
    printf("3. Modify entire column\n");
    
    int choice = get_user_choice("Select modification type", 1, 3);
    
    switch (choice) {
        case 1: {
            int row = get_user_choice("Enter row number", 1, matrix->rows) - 1;
            int col = get_user_choice("Enter column number", 1, matrix->cols) - 1;
            printf("Enter new value: ");
            double value;
            if (scanf("%lf", &value) == 1) {
                matrix->data[row][col] = value;
                printf("✓ Element [%d][%d] updated to %.2f\n", row+1, col+1, value);
            } else {
                printf("Invalid value!\n");
                clear_input_buffer();
            }
            break;
        }
        case 2: {
            int row = get_user_choice("Enter row number", 1, matrix->rows) - 1;
            printf("Enter %d new values for row %d: ", matrix->cols, row + 1);
            for (int j = 0; j < matrix->cols; j++) {
                if (scanf("%lf", &matrix->data[row][j]) != 1) {
                    printf("Invalid value for column %d!\n", j + 1);
                    clear_input_buffer();
                    return;
                }
            }
            printf("✓ Row %d updated successfully\n", row + 1);
            break;
        }
        case 3: {
            int col = get_user_choice("Enter column number", 1, matrix->cols) - 1;
            printf("Enter %d new values for column %d: ", matrix->rows, col + 1);
            for (int i = 0; i < matrix->rows; i++) {
                if (scanf("%lf", &matrix->data[i][col]) != 1) {
                    printf("Invalid value for row %d!\n", i + 1);
                    clear_input_buffer();
                    return;
                }
            }
            printf("✓ Column %d updated successfully\n", col + 1);
            break;
        }
    }
}


void handle_determinant_calculation() {
    printf("\n=== DETERMINANT CALCULATION ===\n");
    display_all_matrices();
    
    if (matrix_count == 0) {
        printf("No matrices available!\n");
        return;
    }
    
    int matrix_id = get_user_choice("Enter matrix ID", 1, next_matrix_id - 1);
    matrix_t* matrix = find_matrix_by_id(matrix_id);
    
    if (!matrix) {
        printf("Matrix not found!\n");
        return;
    }
    
    if (matrix->rows != matrix->cols) {
        printf("Determinant requires square matrix!\n");
        return;
    }
    
    printf("\nComputation method:\n");
    printf("1. Sequential\n");
    printf("2. Parallel (Processes)\n");
    printf("3. OpenMP\n");
    int method = get_user_choice("Select method", 1, 3);
    
    performance_timer_t timer;
    start_timer(&timer);
    
    double det;
    const char* method_name = "";
    
    if (method == 1) {
        det = matrix_determinant_seq(matrix);
        method_name = "sequential";
    } else if (method == 2) {
        det = matrix_determinant_parallel(matrix);
        method_name = "parallel processes";
    } else {
        det = matrix_determinant_openmp(matrix);
        method_name = "OpenMP";
    }
    
    stop_timer(&timer);
    
    printf("\n=== RESULT ===\n");
    printf("Matrix: %s (ID: %d, %dx%d)\n", matrix->name, matrix->id, matrix->rows, matrix->cols);
    printf("Determinant: %.6f\n", det);
    printf("Calculation time (%s): %.6f seconds\n", method_name, get_elapsed_time(&timer));
    printf("================\n");
}
void handle_performance_comparison() {
    printf("\n=== PERFORMANCE COMPARISON ===\n");
    

    int test_size = global_config.performance_test_size;
    printf("Using safe matrix size: %dx%d\n", test_size, test_size);
    printf("Change test size? (1=Yes, 0=No): ");
    int change_size;
    if (scanf("%d", &change_size) == 1 && change_size == 1) {
        printf("Enter new test size: ");
        if (scanf("%d", &test_size) == 1) {
            if (test_size <= 0 || test_size > MAX_MATRIX_SIZE) {
                printf("Invalid size! Using configured size: %d\n", global_config.performance_test_size);
                test_size = global_config.performance_test_size;
            }
        }
        clear_input_buffer();
    }
    clear_input_buffer();
    
    printf("Creating test matrices...\n");
    matrix_t* A = create_random_matrix(test_size, test_size, "Test_A");
    matrix_t* B = create_random_matrix(test_size, test_size, "Test_B");
    

    if (!A || !B) {
        printf("✗ Failed to create test matrices!\n");
        if (A) free_matrix(A);
        if (B) free_matrix(B);
        return;
    }
    
    printf("Testing matrix multiplication...\n");
    
    performance_timer_t sequential_timer, parallel_timer, openmp_timer;
    matrix_t* result_seq = NULL;
    matrix_t* result_par = NULL;
    matrix_t* result_omp = NULL;
    
    int success_seq = 0, success_par = 0, success_omp = 0;
    

    printf("1. Sequential execution...\n");
    start_timer(&sequential_timer);
    result_seq = multiply_matrices_seq(A, B);
    stop_timer(&sequential_timer);
    if (result_seq != NULL) {
        success_seq = 1;
        printf("   ✓ Completed successfully\n");
    } else {
        printf("   ✗ Failed!\n");
    }
    

    printf("2. Parallel execution (processes)...\n");
    start_timer(&parallel_timer);
    result_par = multiply_matrices_parallel(A, B);
    stop_timer(&parallel_timer);
    if (result_par != NULL) {
        success_par = 1;
        printf("   ✓ Completed successfully\n");
    } else {
        printf("   ✗ Failed!\n");
    }
    
    // 3. OpenMP
    printf("3. OpenMP execution...\n");
    start_timer(&openmp_timer);
    result_omp = multiply_matrices_openmp(A, B);
    stop_timer(&openmp_timer);
    if (result_omp != NULL) {
        success_omp = 1;
        printf("   ✓ Completed successfully\n");
    } else {
        printf("   ✗ Failed!\n");
    }
    

    printf("\n=== PERFORMANCE RESULTS ===\n");
    if (success_seq) {
        printf("Sequential:  %.6f seconds\n", get_elapsed_time(&sequential_timer));
    } else {
        printf("Sequential:  FAILED\n");
    }
    
    if (success_par) {
        printf("Parallel:    %.6f seconds", get_elapsed_time(&parallel_timer));
        if (success_seq) {
            double speedup = get_elapsed_time(&sequential_timer) / get_elapsed_time(&parallel_timer);
            printf(" (Speedup: %.2fx)", speedup);
        }
        printf("\n");
    } else {
        printf("Parallel:    FAILED\n");
    }
    
    if (success_omp) {
        printf("OpenMP:      %.6f seconds", get_elapsed_time(&openmp_timer));
        if (success_seq) {
            double speedup = get_elapsed_time(&sequential_timer) / get_elapsed_time(&openmp_timer);
            printf(" (Speedup: %.2fx)", speedup);
        }
        printf("\n");
    } else {
        printf("OpenMP:      FAILED\n");
    }
    printf("==========================\n");
    

    free_matrix(A);
    free_matrix(B);
    if (result_seq) free_matrix(result_seq);
    if (result_par) free_matrix(result_par);
    if (result_omp) free_matrix(result_omp);
}

void handle_openmp_toggle() {
    printf("\nOpenMP is currently: %s\n", use_openmp_flag ? "ENABLED" : "DISABLED");
    printf("Toggle OpenMP? (1=Yes, 0=No): ");
    int choice;
    if (scanf("%d", &choice) == 1) {
        if (choice == 1) {
            use_openmp_flag = !use_openmp_flag;
            global_config.use_openmp = use_openmp_flag;
            
            save_config(NULL);
            
            printf("✓ OpenMP %s\n", use_openmp_flag ? "enabled" : "disabled");
            printf("✓ Configuration saved\n");
        } else {
            printf("OpenMP toggle cancelled.\n");
        }
    } else {
        printf("Invalid input!\n");
    }
    clear_input_buffer();
}

int get_user_choice(const char* prompt, int min, int max) {
    int choice;
    while (1) {
        printf("%s (%d-%d): ", prompt, min, max);
        if (scanf("%d", &choice) == 1) {
            if (choice >= min && choice <= max) {
                clear_input_buffer();
                return choice;
            } else {
                printf("Number must be between %d and %d!\n", min, max);
            }
        } else {
            printf("Invalid input! Please enter a number.\n");
            clear_input_buffer();
        }
    }
}

void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void press_enter_to_continue() {
    printf("\nPress Enter to continue...");
    clear_input_buffer();
    getchar();
}
