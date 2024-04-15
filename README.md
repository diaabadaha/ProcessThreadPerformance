# ProcessThreadPerformance

## Overview
This repository contains a C program that explores process and thread management using POSIX and Pthreads, with a focus on the implementation of process-based and threaded solutions for parallelizable computational tasks. It aims to compare their performance and analyze the trade-offs involved.

## Repository Contents
- `parallelProcessesThreads.c`: The main C source file that implements process management, multithreaded processing, and performance measurement as described in the project task.
- `Task.pdf`: The task description outlining the objectives and requirements of the project, including detailed explanations of each part.
- `Report.pdf`: A comprehensive report that presents explanations, results, analysis, and learnings from the project.

## Project Parts
- **Part 1: Process Management**: Uses `fork()` for creating child processes and implements inter-process communication (IPC).
- **Part 2: Multithreaded Processing**: Uses Pthreads to coordinate threads for task execution and manages shared data access.
- **Part 3: Performance Measurement**: Compares the process-based and threaded solutions by measuring and analyzing execution times.
- **Part 4: Thread Management**: Explores joining and detaching of threads and their impact on throughput and performance.
