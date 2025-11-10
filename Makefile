# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pedantic
OMP_FLAGS = -fopenmp
DEBUG_FLAGS = -g
OPT_FLAGS = -O2

# Directories
SRC_DIR = src
INC_DIR = include
BUILD_DIR = build
MATRICES_DIR = matrices
CONFIG_DIR = config

# Source files - إضافة matrix_generator.c
SRCS = $(SRC_DIR)/main.c \
       $(SRC_DIR)/matrix_operations.c \
       $(SRC_DIR)/process_management.c \
       $(SRC_DIR)/file_operations.c \
       $(SRC_DIR)/openmp_utils.c \
       $(SRC_DIR)/menu_interface.c \
       $(SRC_DIR)/config.c \
       $(SRC_DIR)/matrix_generator.c

OBJS = $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# Target executable
TARGET = matrix_ops

# Default target (with OpenMP)
all: CFLAGS += $(OMP_FLAGS) $(OPT_FLAGS)
all: $(TARGET)

# Debug target
debug: CFLAGS += $(OMP_FLAGS) $(DEBUG_FLAGS)
debug: $(TARGET)

# Without OpenMP
no_omp: CFLAGS += $(OPT_FLAGS)
no_omp: $(TARGET)

# Create directories
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(MATRICES_DIR):
	mkdir -p $(MATRICES_DIR)

$(CONFIG_DIR):
	mkdir -p $(CONFIG_DIR)

# Compile source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

# Link executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ -lm

# Install sample files WITH CUSTOM MENU ORDER
install: $(MATRICES_DIR) $(CONFIG_DIR)
	@echo "Installing sample matrices and configuration..."
	
	@echo "Matrix: Sample1 2x2" > $(MATRICES_DIR)/sample1.txt
	@echo "1.0 2.0" >> $(MATRICES_DIR)/sample1.txt
	@echo "3.0 4.0" >> $(MATRICES_DIR)/sample1.txt
	
	@echo "Matrix: Sample2 2x2" > $(MATRICES_DIR)/sample2.txt
	@echo "5.0 6.0" >> $(MATRICES_DIR)/sample2.txt
	@echo "7.0 8.0" >> $(MATRICES_DIR)/sample2.txt
	
	@echo "Matrix: Sample3 3x3" > $(MATRICES_DIR)/sample3.txt
	@echo "1.0 0.0 0.0" >> $(MATRICES_DIR)/sample3.txt
	@echo "0.0 2.0 0.0" >> $(MATRICES_DIR)/sample3.txt
	@echo "0.0 0.0 3.0" >> $(MATRICES_DIR)/sample3.txt
	
	@echo "matrix_directory=./matrices" > $(CONFIG_DIR)/config.txt
	@echo "use_openmp=1" >> $(CONFIG_DIR)/config.txt
	@echo "max_matrices=50" >> $(CONFIG_DIR)/config.txt
	@echo "process_pool_size=5" >> $(CONFIG_DIR)/config.txt
	@echo "reorder=14,13,12,1,2,3,4,5,6,7,8,9,10,11,15,16" >> $(CONFIG_DIR)/config.txt
	
	@echo "✓ Sample files installed successfully!"

# Clean build
clean:
	rm -rf $(BUILD_DIR) $(TARGET)

# Run with configuration
run: $(TARGET)
	./$(TARGET) config/config.txt

# Run without config (use defaults)
run-default: $(TARGET)
	./$(TARGET)

# Performance test
test: $(TARGET)
	@echo "Running performance tests..."
	./$(TARGET) config/config.txt

# Build and show info
info: $(TARGET)
	@echo "=== Matrix Operations System ==="
	@echo "Target: $(TARGET)"
	@echo "OpenMP: Enabled"
	@echo "Config: Custom menu reordering"
	@echo "================================"

.PHONY: all debug no_omp install clean run run-default test info