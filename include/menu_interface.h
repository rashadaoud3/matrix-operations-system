#ifndef MENU_INTERFACE_H
#define MENU_INTERFACE_H

#include "config.h"
#include "matrix_operations.h"  

void display_main_menu();
int handle_menu_choice(int choice);

void handle_matrix_input();
void handle_matrix_display();
void handle_matrix_deletion();
void handle_matrix_modification();
void handle_matrix_operations(int op_type);
void handle_determinant_calculation();
void handle_eigen_calculation();  
void handle_performance_comparison();
void handle_openmp_toggle();
void handle_random_matrix_generation();

void list_files_in_directory(const char* path);
void delete_matrix_from_folder_helper(matrix_t* matrix, const char* folder_path, const char* folder_type);

int get_user_choice(const char* prompt, int min, int max);
void clear_input_buffer();
void press_enter_to_continue();

#endif
