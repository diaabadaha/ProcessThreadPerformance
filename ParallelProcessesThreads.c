#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>

#define SIZE 100 // Size of matrices

// Thread struct used in both joinable & detached threads
typedef struct {
    int start;
    int end;
    int (*matrix1)[SIZE];
    int (*matrix2)[SIZE];
    int (*result)[SIZE];
    int isDetatched;
} Thread;

// Function to print matrices (used for testing while writing the code)
void printMatrix(int matrix[SIZE][SIZE]) {
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            printf("%d ", matrix[i][j]);
        }
        printf("\n");
    }
}

// Naive multiplication function (without child processes or threads)
void naiveMultiplication(int result[SIZE][SIZE], int matrix1[SIZE][SIZE], int matrix2[SIZE][SIZE]) {
    
    // Matrix multiplication logic
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            for (int k = 0; k < SIZE; k++) {
                result[i][j] += matrix1[i][k] * matrix2[k][j];
            }
        }
    }
}

// Child process function contains the actual implementation for child process (multiply part of matrices)
void childProcess(int result[SIZE][SIZE], int matrix1[SIZE][SIZE], int matrix2[SIZE][SIZE], int start, int end) {
    
    // Matrix multiplication logic
    for (int i = start; i < end; i++) {
        for (int j = 0; j < SIZE; j++) {
            result[i][j] = 0;
            for (int k = 0; k < SIZE; k++) {
                result[i][j] += matrix1[i][k] * matrix2[k][j];
            }
        }
    }
}

// Thread function contains thread code (multiply part of matrices) used in both joinable and detached threads
void* threadFunc(void* arg) {
    // Start recording time with time struct
    struct timespec start_thread;
    timespec_get(&start_thread, TIME_UTC);
    
    Thread *th = (Thread*) arg; // Casting the argument

    // Matrix multiplication logic
    for(int i = th->start; i < th->end; i++) {
        for(int j = 0; j < SIZE; j++) {
            th->result[i][j] = 0;

            for(int k = 0; k < SIZE; k++) {
                th->result[i][j] += th->matrix1[i][k] * th->matrix2[k][j];
            }
        }
    }
    // End recording time with time struct
    struct timespec end_thread;
    timespec_get(&end_thread, TIME_UTC);
    
    // If the thread is detached, calculate & print its excution time
    if (th->isDetatched) {
        double time_thread = (end_thread.tv_sec - start_thread.tv_sec) + (end_thread.tv_nsec - start_thread.tv_nsec) / 1000000000.0;
        printf("\nTime spent in thread = %lf sec\n", time_thread);
    }

    pthread_exit(NULL); // End of thread's excution
}

int main() {

    // Matrices initialization
    int matrix1[SIZE][SIZE];
    int matrix2[SIZE][SIZE];
    int multiplicationMatrix[SIZE][SIZE]; // Result matrix

    // Strings to fill matrix1 & matrix2
    char toFill_1[] = "1210478"; // ID
    char toFill_2[] = "2424587434"; // ID * Birth date = 1210478 * 2003

    // Filling matrix1 and matrix2
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            matrix1[i][j] = toFill_1[j % 7] - '0';
            matrix2[i][j] = toFill_2[j % 10] - '0';
            multiplicationMatrix[i][j] = 0;
        }
    }

/************************Naive Approach*****************************/
    
    // Measuring beginning time
    struct timespec begin_naive;
    timespec_get(&begin_naive, TIME_UTC);

    naiveMultiplication(multiplicationMatrix, matrix1, matrix2);

    // Measuring end time
    struct timespec end_naive;
    timespec_get(&end_naive, TIME_UTC);

    // Measuring and printing the whole time of naive approach
    double time_naive = (end_naive.tv_sec - begin_naive.tv_sec) + (end_naive.tv_nsec - begin_naive.tv_nsec) / 1000000000.0;
    printf("\nTime spent in naive approach = %lf sec\n", time_naive);
    printf("Naive approach throughput is %lf\n", 1 / time_naive);


/************************Child Processes Approach*****************************/

    // Child processes multiplication
    int processesNumber = 6;
    int pipes[processesNumber][2]; // File directors (pipes)

    struct timespec begin_processes; // Measuring beginning time
    timespec_get(&begin_processes, TIME_UTC);

    // A loop for processes
    for (int i = 0; i < processesNumber; i++) {
        // Create pipes
        if (pipe(pipes[i]) == -1) {
            printf("\nError: Program cannot make a pipe!\n");
            return 1;
        }

        // Fork child processes
        int id = fork();

        if (id == -1) {
            printf("\nError: Program cannot make a child process!\n");
            return 2;
        } else if (id == 0) { // Child process
            // Find start and end rows for the child process
            int start = i * SIZE / processesNumber;
            int end = (i + 1) * SIZE / processesNumber;

            // Submatrix multiplication
            childProcess(multiplicationMatrix, matrix1, matrix2, start, end);

            close(pipes[i][0]); // Close reading from pipes
            // Writing data on pipes
            write(pipes[i][1], multiplicationMatrix[start], sizeof(int) * (end - start) * SIZE); 
            
            close(pipes[i][1]); // Close writing on pipes

            exit(0);
        }
    }

    // A loop for rading from pipes
    for (int i = 0; i < processesNumber; i++) {
        close(pipes[i][1]); // Close writing on pipes

        // Find start and end rows for the child process
        int start = i * SIZE / processesNumber;
        int end = (i + 1) * SIZE / processesNumber;

        // The parent process reads the partial results from pipes
        read(pipes[i][0], multiplicationMatrix[start], sizeof(int) * (end - start) * SIZE);
        
        close(pipes[i][0]); // Close reading from pipes
    }
    
    // Waits for all child processes to finish.
    for (int i = 0; i < processesNumber; i++) {
        wait(NULL);
    }
    
    // Measure processes end time
    struct timespec end_processes;
    timespec_get(&end_processes, TIME_UTC);
    
    // Calculate and print duration for child processes approach
    double time_processes = (end_processes.tv_sec - begin_processes.tv_sec) + (end_processes.tv_nsec - begin_processes.tv_nsec) / 1000000000.0;
    printf("\nTime spent in child processes (%d processes) approach = %lf sec\n", processesNumber, time_processes);
    printf("Child processes approach throughput is %lf\n", 1 / time_processes);


/************************Joinable Threads Approach*****************************/
    
    // Measuring beginning time
    struct timespec begin_joinable;
    timespec_get(&begin_joinable, TIME_UTC);
    
    int joinableThreadsNumber = 6;

    pthread_t joinableThreads[joinableThreadsNumber]; // THread array
    Thread args[joinableThreadsNumber]; // Thread arguments array

    // Joinable thread creation
    for(int i = 0; i < joinableThreadsNumber; i++) {
        // Calculating start and end indices for submatrix
        args[i].start = i * SIZE / joinableThreadsNumber;
        args[i].end = (i + 1) * SIZE / joinableThreadsNumber;

        args[i].matrix1 = matrix1;
        args[i].matrix2 = matrix2;
        args[i].result = multiplicationMatrix;
        args[i].isDetatched = 0; // Joinable thread

        // Create thread
        if(pthread_create(&joinableThreads[i], NULL, threadFunc, (void*)&args[i])) {
            printf("\nError: Program cannot create a thread!\n");
            return 3;   
        }
    }

    // Thread joining loop
    for (int i = 0; i < joinableThreadsNumber; i++) {
        if(pthread_join(joinableThreads[i], NULL)) {
            printf("\nError: program cannot join threads!\n");
            return 4;
        }
    }

    // Measuring end time
    struct timespec end_joinable;
    timespec_get(&end_joinable, TIME_UTC);

    // Calculate and print duration for joinable threads approach
    double time_joinable = (end_joinable.tv_sec - begin_joinable.tv_sec) + (end_joinable.tv_nsec - begin_joinable.tv_nsec) / 1000000000.0;
    printf("\nTime spent in joinable threads (%d threads) approach = %lf sec\n", joinableThreadsNumber, time_joinable);
    
    // Calculate and print throuput
    printf("Joinable threads approach throughput is %lf\n", 1 / time_joinable);

/************************Detached Threads Approach*****************************/

    // Measuring beginning time
    struct timespec begin_detached; 
    timespec_get(&begin_detached, TIME_UTC);

    int detachedThreadsNumber = 6;
    pthread_t detachedThreads[detachedThreadsNumber]; // Thread array

    // Thread creation loop
    for(int i = 0; i < detachedThreadsNumber; i++) {
        Thread *args = malloc(sizeof(Thread)); // Dynamically allocate memory for thread arguments
        if (!args) {
            perror("Failed to allocate memory for thread arguments");
            return 5;
        }

    // Calculating start and end indices for submatrix
    args->start = i * SIZE / detachedThreadsNumber;
    args->end = (i + 1) * SIZE / detachedThreadsNumber;

    args->matrix1 = matrix1;
    args->matrix2 = matrix2;
    args->result = multiplicationMatrix;
    args->isDetatched = 1; // Detached thread

    // Thread creation
    if(pthread_create(&detachedThreads[i], NULL, threadFunc, args)) {
        printf("\nError: Program cannot create a thread!\n");
        return 6;   
    }

    // Detach threads
    pthread_detach(detachedThreads[i]);
}

    // Wait mechanism used to obtain a correct matrix multiplication
    // Also used to let each thread print its excution time
   //sleep(1);

    // Measuring end time
    struct timespec end_detached;
    timespec_get(&end_detached, TIME_UTC);

    // Calculate and print duration for detached threads creation    
    double time_detached = (end_detached.tv_sec - begin_detached.tv_sec) + (end_detached.tv_nsec - begin_detached.tv_nsec) / 1000000000.0;
    printf("\nDetached thread creation time = %lf sec\n", time_detached);

    return 0;
}