#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/config.h"

config_t global_config;

void initialize_default_config() {

    strcpy(global_config.matrix_directory, "./matrices");
    global_config.max_matrices = MAX_MATRICES;
    global_config.process_pool_size = 5;
    global_config.use_openmp = 1;
    global_config.custom_menu = 0;
    global_config.max_processes = MAX_PROCESSES;  
    

    for (int i = 0; i < MENU_ITEMS; i++) {
        global_config.menu_order[i] = i + 1;
    }
    

    global_config.performance_test_size = 30;
    global_config.openmp_threads = 4;
    global_config.enable_process_pool = 1;
    global_config.show_timings = 1;
    global_config.auto_save_interval = 5;
    global_config.auto_load_on_startup = 1;
    global_config.create_backups = 1;
    global_config.enable_memory_check = 1;
    global_config.cache_size = 200;
    global_config.eigen_tolerance = 1e-12;
    global_config.eigen_max_iterations = 2000;
    global_config.determinant_method = 1;
    global_config.multiplication_method = 2;
}

void load_config(const char* filename) {
    initialize_default_config();
    
    const char* config_file = filename ? filename : "config/config.txt";
    FILE* file = fopen(config_file, "r");
    if (!file) {
        printf("Config file '%s' not found, using defaults.\n", config_file);
        return;
    }
    
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;
        

        if (strlen(line) == 0 || line[0] == '#') continue;
        
        char* key = strtok(line, "=");
        char* value = strtok(NULL, "=");
        
        if (!key || !value) continue;
        

        char* trimmed_key = key;
        while (*trimmed_key == ' ') trimmed_key++;
        char* end = trimmed_key + strlen(trimmed_key) - 1;
        while (end > trimmed_key && *end == ' ') *end-- = '\0';
        
        char* trimmed_value = value;
        while (*trimmed_value == ' ') trimmed_value++;
        end = trimmed_value + strlen(trimmed_value) - 1;
        while (end > trimmed_value && *end == ' ') *end-- = '\0';
        

        if (strcmp(trimmed_key, "matrix_directory") == 0) {
            strcpy(global_config.matrix_directory, trimmed_value);
        }
        else if (strcmp(trimmed_key, "use_openmp") == 0) {
            global_config.use_openmp = atoi(trimmed_value);
        }
        else if (strcmp(trimmed_key, "max_matrices") == 0) {
            global_config.max_matrices = atoi(trimmed_value);
        }
        else if (strcmp(trimmed_key, "process_pool_size") == 0) {
            global_config.process_pool_size = atoi(trimmed_value);
        }
        else if (strcmp(trimmed_key, "max_processes") == 0) {
            global_config.max_processes = atoi(trimmed_value);
        }
        else if (strcmp(trimmed_key, "performance_test_size") == 0) {
            global_config.performance_test_size = atoi(trimmed_value);
        }
        else if (strcmp(trimmed_key, "openmp_threads") == 0) {
            global_config.openmp_threads = atoi(trimmed_value);
        }
        else if (strcmp(trimmed_key, "enable_process_pool") == 0) {
            global_config.enable_process_pool = atoi(trimmed_value);
        }
        else if (strcmp(trimmed_key, "show_timings") == 0) {
            global_config.show_timings = atoi(trimmed_value);
        }
        else if (strcmp(trimmed_key, "auto_save_interval") == 0) {
            global_config.auto_save_interval = atoi(trimmed_value);
        }
        else if (strcmp(trimmed_key, "auto_load_on_startup") == 0) {
            global_config.auto_load_on_startup = atoi(trimmed_value);
        }
        else if (strcmp(trimmed_key, "create_backups") == 0) {
            global_config.create_backups = atoi(trimmed_value);
        }
        else if (strcmp(trimmed_key, "enable_memory_check") == 0) {
            global_config.enable_memory_check = atoi(trimmed_value);
        }
        else if (strcmp(trimmed_key, "cache_size") == 0) {
            global_config.cache_size = atoi(trimmed_value);
        }
        else if (strcmp(trimmed_key, "eigen_tolerance") == 0) {
            global_config.eigen_tolerance = atof(trimmed_value);
        }
        else if (strcmp(trimmed_key, "eigen_max_iterations") == 0) {
            global_config.eigen_max_iterations = atoi(trimmed_value);
        }
        else if (strcmp(trimmed_key, "determinant_method") == 0) {
            global_config.determinant_method = atoi(trimmed_value);
        }
        else if (strcmp(trimmed_key, "multiplication_method") == 0) {
            global_config.multiplication_method = atoi(trimmed_value);
        }
        else if (strcmp(trimmed_key, "reorder") == 0) {
            global_config.custom_menu = 1;
            int used_numbers[MENU_ITEMS] = {0};
            int index = 0;
            
            char* token = strtok(trimmed_value, ",");
            while (token && index < MENU_ITEMS) {
                int num = atoi(token);
                if (num >= 1 && num <= MENU_ITEMS) {
                    global_config.menu_order[index++] = num;
                    used_numbers[num-1] = 1;
                }
                token = strtok(NULL, ",");
            }
            
            for (int i = 1; i <= MENU_ITEMS && index < MENU_ITEMS; i++) {
                if (!used_numbers[i-1]) {
                    global_config.menu_order[index++] = i;
                }
            }
        }
        else {
            printf("Config: Unknown option '%s', ignoring.\n", trimmed_key);
        }
    }
    
    fclose(file);
    printf("Configuration loaded from '%s'\n", config_file);
}

void save_config(const char* filename) {
    const char* config_file = filename ? filename : "config/config.txt";
    FILE* file = fopen(config_file, "w");
    if (!file) {
        printf("Error: Could not save config file '%s'\n", config_file);
        return;
    }
    
    fprintf(file, "# Matrix Operations Configuration\n\n");
    
    fprintf(file, "# Basic Settings\n");
    fprintf(file, "matrix_directory=%s\n", global_config.matrix_directory);
    fprintf(file, "use_openmp=%d\n", global_config.use_openmp);
    fprintf(file, "max_matrices=%d\n", global_config.max_matrices);
    fprintf(file, "process_pool_size=%d\n", global_config.process_pool_size);
    fprintf(file, "max_processes=%d\n", global_config.max_processes);  
    
    fprintf(file, "\n# Performance Settings\n");
    fprintf(file, "performance_test_size=%d\n", global_config.performance_test_size);
    fprintf(file, "openmp_threads=%d\n", global_config.openmp_threads);
    fprintf(file, "enable_process_pool=%d\n", global_config.enable_process_pool);
    
    fprintf(file, "\n# UI Settings\n");
    fprintf(file, "show_timings=%d\n", global_config.show_timings);
    fprintf(file, "auto_save_interval=%d\n", global_config.auto_save_interval);
    fprintf(file, "auto_load_on_startup=%d\n", global_config.auto_load_on_startup);
    fprintf(file, "create_backups=%d\n", global_config.create_backups);
    
    fprintf(file, "\n# Memory Settings\n");
    fprintf(file, "enable_memory_check=%d\n", global_config.enable_memory_check);
    fprintf(file, "cache_size=%d\n", global_config.cache_size);
    
    fprintf(file, "\n# Algorithm Settings\n");
    fprintf(file, "eigen_tolerance=%.12f\n", global_config.eigen_tolerance);
    fprintf(file, "eigen_max_iterations=%d\n", global_config.eigen_max_iterations);
    fprintf(file, "determinant_method=%d\n", global_config.determinant_method);
    fprintf(file, "multiplication_method=%d\n", global_config.multiplication_method);
    
    if (global_config.custom_menu) {
        fprintf(file, "\n# Menu Settings\n");
        fprintf(file, "reorder=");
        for (int i = 0; i < MENU_ITEMS; i++) {
            fprintf(file, "%d", global_config.menu_order[i]);
            if (i < MENU_ITEMS-1) fprintf(file, ",");
        }
        fprintf(file, "\n");
    }
    
    fclose(file);
    printf("Configuration saved to '%s'\n", config_file);
}

void print_config() {
    printf("\n=== Current Configuration ===\n");
    printf("Matrix Directory: %s\n", global_config.matrix_directory);
    printf("Use OpenMP: %s\n", global_config.use_openmp ? "Yes" : "No");
    printf("Max Matrices: %d\n", global_config.max_matrices);
    printf("Process Pool Size: %d\n", global_config.process_pool_size);
    printf("Max Processes: %d\n", global_config.max_processes);  
    printf("Custom Menu: %s\n", global_config.custom_menu ? "Yes" : "No");
    
    printf("\nPerformance Settings:\n");
    printf("  Test Size: %d\n", global_config.performance_test_size);
    printf("  OpenMP Threads: %d\n", global_config.openmp_threads);
    printf("  Process Pool: %s\n", global_config.enable_process_pool ? "Enabled" : "Disabled");
    
    printf("\nUI Settings:\n");
    printf("  Show Timings: %s\n", global_config.show_timings ? "Yes" : "No");
    printf("  Auto Save: %d minutes\n", global_config.auto_save_interval);
    printf("  Auto Load: %s\n", global_config.auto_load_on_startup ? "Yes" : "No");
    printf("  Create Backups: %s\n", global_config.create_backups ? "Yes" : "No");
    
    printf("\nMemory Settings:\n");
    printf("  Memory Check: %s\n", global_config.enable_memory_check ? "Enabled" : "Disabled");
    printf("  Cache Size: %d\n", global_config.cache_size);
    
    printf("\nAlgorithm Settings:\n");
    printf("  Eigen Tolerance: %.2e\n", global_config.eigen_tolerance);
    printf("  Eigen Max Iterations: %d\n", global_config.eigen_max_iterations);
    printf("  Determinant Method: %d\n", global_config.determinant_method);
    printf("  Multiplication Method: %d\n", global_config.multiplication_method);
    
    if (global_config.custom_menu) {
        printf("Menu Order: ");
        for (int i = 0; i < MENU_ITEMS; i++) {
            printf("%d", global_config.menu_order[i]);
            if (i < MENU_ITEMS-1) printf(", ");
        }
        printf("\n");
    }
    printf("==============================\n");
}

const char* get_menu_item_name(int menu_number) {
    static const char* menu_names[] = {
        "Enter a matrix",
        "Display a matrix", 
        "Delete a matrix",
        "Modify a matrix",
        "Read matrix from file",
        "Read matrices from folder",
        "Save matrix to file",
        "Save all matrices to folder",
        "Display all matrices",
        "Add two matrices",
        "Subtract two matrices",
        "Multiply two matrices",
        "Find determinant",
        "Find eigenvalues & eigenvectors",  
        "Generate random matrices",
        "Performance comparison",
        "Toggle OpenMP",
        "Exit"
    };
    
    return (menu_number >= 1 && menu_number <= MENU_ITEMS) ? menu_names[menu_number-1] : "Unknown option";
}

int has_custom_menu() {
    return global_config.custom_menu;
}
