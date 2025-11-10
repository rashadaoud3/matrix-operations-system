#ifndef CONFIG_H
#define CONFIG_H

#define MAX_MATRICES 200
#define MAX_PROCESSES 20
#define MAX_FILENAME 256
#define MAX_MATRIX_SIZE 100
#define BUFFER_SIZE 1024
#define PROCESS_TIMEOUT 300
#define MENU_ITEMS 18

typedef struct {
    // Basic Settings
    char matrix_directory[MAX_FILENAME];
    int max_matrices;
    int process_pool_size;
    int use_openmp;
    int menu_order[MENU_ITEMS];
    int custom_menu;
    
    int max_processes;
    // Performance Settings
    int performance_test_size;
    int openmp_threads;
    int enable_process_pool;
    
    // UI Settings
    int show_timings;
    int auto_save_interval;
    int auto_load_on_startup;
    int create_backups;
    
    // Memory Settings
    int enable_memory_check;
    int cache_size;
    
    // Algorithm Settings
    double eigen_tolerance;
    int eigen_max_iterations;
    int determinant_method;
    int multiplication_method;
    
} config_t;

void initialize_default_config();
void load_config(const char* filename);
void save_config(const char* filename);
void print_config();
const char* get_menu_item_name(int menu_number);
int has_custom_menu();

extern config_t global_config;

#endif
