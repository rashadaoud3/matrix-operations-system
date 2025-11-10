#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include "../include/config.h"
#include "../include/menu_interface.h"
#include "../include/process_management.h"
#include "../include/file_operations.h"
#include "../include/matrix_operations.h"
#include "../include/openmp_utils.h"
#include "../include/matrix_generator.h"

extern config_t global_config;
extern matrix_t* matrix_registry[MAX_MATRICES];
extern int matrix_count;
extern child_process_t process_pool[MAX_PROCESSES];
extern int active_processes;

int use_openmp_flag = 1;

void print_banner() {
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║           Matrix Operations - Pipes, Signals & OpenMP       ║\n");
    printf("║        Birzeit University - ENCS4330 - Fall 2025/2026       ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    printf("\n");
}

void initialize_system(int argc, char* argv[]) {
    printf("Initializing system...\n");
    
    if (argc > 1) {
        load_config(argv[1]);
    } else {
        load_config(NULL);
    }
    
    printf("DEBUG: System configured with max_matrices = %d\n", global_config.max_matrices);
    
    use_openmp_flag = global_config.use_openmp;
    if (use_openmp_flag) {
        enable_openmp();
        printf("OpenMP: Enabled\n");
    } else {
        disable_openmp();
        printf("OpenMP: Disabled\n");
    }
    
    for (int i = 0; i < MAX_MATRICES; i++) {
        matrix_registry[i] = NULL;
    }
    
    initialize_process_pool();
    setup_signal_handlers();
    
    printf("System initialized successfully.\n");
    printf("Matrix directory: %s\n", global_config.matrix_directory);
    printf("Process pool size: %d\n", global_config.process_pool_size);
    printf("Max matrices: %d\n", global_config.max_matrices);
    
    if (global_config.custom_menu) {
        printf("Custom menu order: Enabled\n");
    }
}

void cleanup_system() {
    printf("Cleaning up system...\n");
    clear_matrix_registry();
    cleanup_process_pool();
    printf("System cleanup completed.\n");
}

int main(int argc, char* argv[]) {
    print_banner();
    initialize_system(argc, argv);
    
    int loaded_count = load_matrix_directory();
    if (loaded_count > 0) {
        printf("Automatically loaded %d matrices from '%s'\n", 
               loaded_count, global_config.matrix_directory);
    } else {
        printf("No matrices found in '%s'. You can enter matrices manually.\n",
               global_config.matrix_directory);
    }
    
    int running = 1;
    while (running) {
        display_main_menu();
        int choice = get_user_choice("Enter your choice", 1, 18);
        int should_exit = handle_menu_choice(choice);
        
        if (should_exit) {
            running = 0;
        } else if (choice != 17 && !(global_config.custom_menu && global_config.menu_order[choice-1] == 17)) {
            press_enter_to_continue();
        }
    }
    
    cleanup_system();
    printf("Thank you for using Matrix Operations!\n");
    return 0;
}
