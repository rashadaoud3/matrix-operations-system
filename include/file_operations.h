#ifndef FILE_OPERATIONS_H
#define FILE_OPERATIONS_H

#include "matrix_operations.h"
#define MAX_FILENAME 256

matrix_t* read_matrix_from_file(const char* filename);
int write_matrix_to_file(const matrix_t* matrix, const char* filename);
int read_matrices_from_folder(const char* folder_path);
int write_matrices_to_folder(const char* folder_path);
int load_matrix_directory();
void save_matrix_directory();
void get_matrix_files(const char* folder_path, char files[][MAX_FILENAME], int* count);

#endif
