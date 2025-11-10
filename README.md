# Real-Time Matrix Operations System

A high-performance multi-processing application for matrix operations using Pipes, Signals, Process Pools, and OpenMP under Linux.

---

## 📋 Project Overview

This project is developed as part of **Real-Time Applications & Embedded Systems (ENCS4330)** at **Birzeit University**.

It implements a complete matrix operations system capable of running computations using:
- **Multi-processing** (multiple child processes)
- **Inter-process communication** (Pipes)
- **Signal handling**
- **Parallel processing** with OpenMP

The system includes a menu-driven interface and supports loading, saving, and managing multiple matrices in memory.

---

## 🚀 Features

### ✅ Core Matrix Operations
- Matrix Addition
- Matrix Subtraction
- Matrix Multiplication
- Determinant Calculation (Sequential, Multiprocessing, OpenMP)
- Eigenvalues & Eigenvectors

### ✅ System & Performance Features
- **Process Pool** for fast parallel computation
- **Pipes** for communication between the parent and child processes
- **Signal Handling** for cleanup and process management
- **OpenMP Acceleration** for large-matrix operations
- **Execution Time Measurement** to compare sequential vs parallel execution
- **Configurable Menu** through an external config file
- **File I/O** (Save / Load individual or all matrices)

---

## 🛠️ Technologies Used

- C Programming Language
- Linux System Calls
- Pipes (IPC)
- Signals
- Forked Multi-Processing
- OpenMP
- Modular Build System using Makefile

---

## 📁 Project Structure

```
Project/
│
├── config/
│   └── config.txt
│
├── include/
│   ├── config.h
│   ├── file_operations.h
│   ├── matrix_generator.h
│   ├── matrix_operations.h
│   ├── menu_interface.h
│   ├── openmp_utils.h
│   └── process_management.h
│
├── src/
│   ├── config.c
│   ├── file_operations.c
│   ├── main.c
│   ├── matrix_generator.c
│   ├── matrix_operations.c
│   ├── menu_interface.c
│   ├── openmp_utils.c
│   └── process_management.c
│
├── matrices/
│   └── (matrix text files)
│
├── Makefile
└── README.md
```

---

## ▶️ How to Build & Run

### 1. Compile the project

```bash
make
```

### 2. Run the executable

```bash
make run
```

---

# ✅ Authors
- Rasha Daoud
- Leyan Buirat
- Nadia Thaer
- Lina Abufarha

# 📧 Contact
- Rasha Daoud
- Email: [Rasha Daoud](rashadaoud134@gmail.com)
- GitHub: [rashadaoud3](https://github.com/rashadaoud3)
-  LinkedIn: [Rasha Daoud](https://www.linkedin.com/in/rasha-daoud/)



---

## 📚 License

This project is developed for academic purposes under **Birzeit University — ENCS4330**.

If you like this project, feel free to ⭐ **star the repository**!
