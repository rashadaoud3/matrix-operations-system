#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <ctype.h>
#include "../include/file_operations.h"
#include "../include/config.h"

int parse_matrix_line(const char* line, double* values, int max_values) {
    char buffer[512];
    strncpy(buffer, line, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';
    
    char* token = strtok(buffer, " \t\n,;[](){}");
    int count = 0;
    
    while (token && count < max_values) {
        if (strlen(token) == 1 && 
            (token[0] == '[' || token[0] == ']' || token[0] == '(' || token[0] == ')' || 
             token[0] == '{' || token[0] == '}' || token[0] == ',' || token[0] == ';')) {
            token = strtok(NULL, " \t\n,;[](){}");
            continue;
        }
        
        char* endptr;
        double value = strtod(token, &endptr);
        
        if (endptr != token) {
            values[count++] = value;
        } else {
            char cleaned_token[100];
            strncpy(cleaned_token, token, sizeof(cleaned_token) - 1);
            cleaned_token[sizeof(cleaned_token) - 1] = '\0';
            
            char* start = cleaned_token;
            char* end = start + strlen(cleaned_token) - 1;
            
            while (start <= end && strchr("[](){};, \t", *start)) start++;
            while (end > start && strchr("[](){};, \t", *end)) *end-- = '\0';
            
            if (start < end) {
                value = strtod(start, &endptr);
                if (endptr != start) {
                    values[count++] = value;
                }
            }
        }
        
        token = strtok(NULL, " \t\n,;[](){}");
    }
    
    return count;
}

void extract_matrix_name(const char* filename, char* name) {
    const char* basename = strrchr(filename, '/');
    if (basename) basename++;
    else basename = filename;
    
    strncpy(name, basename, 49);
    name[49] = '\0';
    
    char* dot = strrchr(name, '.');
    if (dot) *dot = '\0';
    
    for (int i = 0; name[i]; i++) {
        if (!isalnum(name[i]) && name[i] != '_' && name[i] != '-') {
            name[i] = '_';
        }
    }
}

void debug_file_content(const char* filename) {
    printf("=== DEBUG FILE CONTENT: %s ===\n", filename);
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Cannot open file for debugging\n");
        return;
    }
    
    char line[512];
    int line_num = 0;
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;
        printf("Line %2d: [%s]\n", ++line_num, line);
        
        double values[100];
        int num_values = parse_matrix_line(line, values, 100);
        printf("       -> Parsed %2d values: ", num_values);
        for (int i = 0; i < num_values && i < 8; i++) {
            printf("%6.2f ", values[i]);
        }
        if (num_values > 8 ) printf("... (total: %d)", num_values);
        printf("\n");
    }
    fclose(file);
    printf("=== END DEBUG ===\n\n");
}

matrix_t* read_matrix_from_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("ERROR: Cannot open file: %s\n", filename);
        return NULL;
    }

    printf("Reading matrix from: %s\n", filename);
    
    char line[512];
    int cols = 0;
    double** temp_data = NULL;
    int max_rows = 0;
    char name[50];
    
    extract_matrix_name(filename, name);
    
    int first_row_cols = 0, second_row_cols = 0;
    int determined_cols = 0;
    
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;
        
        double values[100];
        int num_values = parse_matrix_line(line, values, 100);
        
        if (num_values > 0) {
            if (first_row_cols == 0) {
                first_row_cols = num_values;
                printf("  First row has %d columns\n", first_row_cols);
            } else if (second_row_cols == 0) {
                second_row_cols = num_values;
                printf("  Second row has %d columns\n", second_row_cols);
                
                if (first_row_cols == second_row_cols) {
                    cols = first_row_cols;
                } else {
                    cols = (first_row_cols + second_row_cols) / 2;
                    printf("  WARNING: Column count mismatch! Using average: %d\n", cols);
                }
                determined_cols = 1;
                break;
            }
        }
    }
    
    if (!determined_cols && first_row_cols > 0) {
        cols = first_row_cols;
        determined_cols = 1;
        printf("  Using single row column count: %d\n", cols);
    }
    
    if (!determined_cols) {
        printf("ERROR: Cannot determine matrix dimensions from file: %s\n", filename);
        fclose(file);
        return NULL;
    }
    
    rewind(file);
    
    int valid_rows = 0;
    int skipped_rows = 0;
    
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;
        
        double values[100];
        int num_values = parse_matrix_line(line, values, 100);
        
        if (num_values > 0) {
            if (abs(num_values - cols) > cols / 2) {
                printf("  WARNING: Skipping row %d - has %d values, expected ~%d\n", 
                       valid_rows + 1, num_values, cols);
                skipped_rows++;
                continue;
            }
            
            if (valid_rows >= max_rows) {
                max_rows = (max_rows == 0) ? 10 : max_rows * 2;
                temp_data = realloc(temp_data, max_rows * sizeof(double*));
                for (int i = valid_rows; i < max_rows; i++) {
                    temp_data[i] = malloc(cols * sizeof(double));
                    for (int j = 0; j < cols; j++) {
                        temp_data[i][j] = 0.0;
                    }
                }
            }
            
            int copy_count = (num_values < cols) ? num_values : cols;
            for (int j = 0; j < copy_count; j++) {
                temp_data[valid_rows][j] = values[j];
            }
            
            for (int j = copy_count; j < cols; j++) {
                temp_data[valid_rows][j] = 0.0;
            }
            
            valid_rows++;
        }
    }
    
    fclose(file);
    
    if (valid_rows == 0) {
        printf("ERROR: No valid data found in file: %s\n", filename);
        if (temp_data) {
            for (int i = 0; i < max_rows; i++) free(temp_data[i]);
            free(temp_data);
        }
        return NULL;
    }
    
    matrix_t* matrix = create_matrix(valid_rows, cols, name);
    if (!matrix) {
        printf("ERROR: Failed to create matrix structure for: %s\n", filename);
        for (int i = 0; i < max_rows; i++) free(temp_data[i]);
        free(temp_data);
        return NULL;
    }
    
    for (int i = 0; i < valid_rows; i++) {
        for (int j = 0; j < cols; j++) {
            matrix->data[i][j] = temp_data[i][j];
        }
        free(temp_data[i]);
    }
    free(temp_data);
    
    printf("SUCCESS: Loaded matrix '%s' (%dx%d) from '%s'", name, valid_rows, cols, filename);
    if (skipped_rows > 0) {
        printf(" (skipped %d rows)", skipped_rows);
    }
    printf("\n");
    
    return matrix;
}

int write_matrix_to_file(const matrix_t* matrix, const char* filename) {
    if (!matrix) {
        printf("ERROR: Invalid matrix for writing\n");
        return -1;
    }

    FILE* file = fopen(filename, "w");
    if (!file) {
        printf("ERROR: Cannot create file: %s\n", filename);
        return -1;
    }

    fprintf(file, "# Matrix: %s %dx%d\n", matrix->name, matrix->rows, matrix->cols);
    fprintf(file, "# Generated by Matrix Operations System\n");

    for (int i = 0; i < matrix->rows; i++) {
        for (int j = 0; j < matrix->cols; j++) {
            fprintf(file, "%.6f", matrix->data[i][j]);
            if (j < matrix->cols - 1) fprintf(file, " ");
        }
        fprintf(file, "\n");
    }

    fclose(file);
    printf("SUCCESS: Saved matrix '%s' to '%s'\n", matrix->name, filename);
    return 0;
}

void get_matrix_files(const char* folder_path, char files[][MAX_FILENAME], int* count) {
    DIR* dir = opendir(folder_path);
    if (!dir) {
        printf("ERROR: Cannot open directory: %s\n", folder_path);
        *count = 0;
        return;
    }

    struct dirent* entry;
    *count = 0;

    while ((entry = readdir(dir)) != NULL && *count < MAX_MATRICES) {
        char* ext = strrchr(entry->d_name, '.');
        if (ext && (strcmp(ext, ".txt") == 0 || strcmp(ext, ".mat") == 0)) {

            int path_len = snprintf(files[*count], MAX_FILENAME - 1, "%s/%s", folder_path, entry->d_name);
            if (path_len >= MAX_FILENAME) {

                strncpy(files[*count], entry->d_name, MAX_FILENAME - 1);
            }
            files[*count][MAX_FILENAME - 1] = '\0';
            (*count)++;
        }
    }

    closedir(dir);
}

int read_matrices_from_folder(const char* folder_path) {
    printf("\n=== LOADING MATRICES FROM: %s ===\n", folder_path);
    
    char files[MAX_MATRICES][MAX_FILENAME];
    int file_count = 0;

    get_matrix_files(folder_path, files, &file_count);

    if (file_count == 0) {
        printf("No matrix files found in '%s'\n", folder_path);
        return 0;
    }

    printf("Found %d matrix files\n", file_count);

    int loaded_count = 0;
    int failed_count = 0;
    
    for (int i = 0; i < file_count; i++) {
        printf("\n[%d/%d] ", i + 1, file_count);
        
        matrix_t* matrix = read_matrix_from_file(files[i]);
        if (matrix) {
            if (add_matrix_to_registry(matrix) >= 0) {
                loaded_count++;
            } else {
                printf("ERROR: Matrix registry full! Cannot add: %s\n", matrix->name);
                free_matrix(matrix);
                failed_count++;
            }
        } else {
            failed_count++;
        }
    }

    printf("\n=== LOADING SUMMARY ===\n");
    printf("Successfully loaded: %d matrices\n", loaded_count);
    printf("Failed to load: %d matrices\n", failed_count);
    printf("Total files processed: %d\n", file_count);
    printf("========================\n");
    
    return loaded_count;
}

int write_matrices_to_folder(const char* folder_path) {
    struct stat st = {0};
    if (stat(folder_path, &st) == -1) {
        if (mkdir(folder_path, 0755) == -1) {
            printf("ERROR: Cannot create directory: %s\n", folder_path);
            return -1;
        }
        printf("Created directory: %s\n", folder_path);
    }

    printf("\n=== SAVING MATRICES TO: %s ===\n", folder_path);

    int saved_count = 0;
    int total_count = 0;
    
    for (int i = 0; i < MAX_MATRICES; i++) {
        if (matrix_registry[i]) {
            total_count++;
            char filename[MAX_FILENAME];
            snprintf(filename, sizeof(filename), "%s/%s.txt", 
                    folder_path, matrix_registry[i]->name);
            
            if (write_matrix_to_file(matrix_registry[i], filename) == 0) {
                saved_count++;
            }
        }
    }

    printf("=== SAVING SUMMARY ===\n");
    printf("Successfully saved: %d matrices\n", saved_count);
    printf("Total matrices: %d\n", total_count);
    printf("=====================\n");

    return saved_count;
}

int load_matrix_directory() {
    return read_matrices_from_folder(global_config.matrix_directory);
}

void save_matrix_directory() {
    write_matrices_to_folder(global_config.matrix_directory);
}
