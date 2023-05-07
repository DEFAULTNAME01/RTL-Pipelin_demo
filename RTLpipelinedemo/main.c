#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <time.h>

typedef struct {
    int data[5];
    bool valid;
} DataStructure;

DataStructure pipeline_register;
sem_t sem_A, sem_B;
bool continuous_run = false;
bool program_terminated = false;

void* function_A(void* args) {
    while (continuous_run) {
        sem_wait(&sem_A);
        for (int i = 0; i < 5; ++i) {
            pipeline_register.data[i] = rand() % 100;
        }
        pipeline_register.valid = true;
        printf("Data in pipeline is now valid\n");
        sem_post(&sem_B);
    }
    return NULL;
}

void* function_B(void* args) {
    while (continuous_run) {
        while (!pipeline_register.valid) {
            // Wait for data to become valid
        }
        printf("First element of structure at B: %d\n", pipeline_register.data[0]);
        pipeline_register.valid = false;
        sem_post(&sem_A);
    }
    return NULL;
}

void* control_function(void* arg) {
    while (true) {
        int ch = getchar();
        if (ch == 's' || ch == 'S') {
            continuous_run = false;
            program_terminated = true;
            break;
        }
        if (ch == 'p' || ch == 'P') {
            continuous_run = true;
            sem_post(&sem_A);
        }
    }
    return NULL;
}

int main() {
    pthread_t thread_A, thread_B, thread_control;
    int ch;
    time_t start_time = clock();

    // Initialize semaphores
    sem_init(&sem_A, 0, 0);
    sem_init(&sem_B, 0, 0);

    // Wait for user input to start the program
    while (true) {
        ch = getchar();
        if (ch == 'p' || ch == 'P') {
            continuous_run = true;
            sem_post(&sem_A);
            break;
        }
    }

    // Create threads
    pthread_create(&thread_A, NULL, function_A, NULL);
    pthread_create(&thread_B, NULL, function_B, NULL);
    pthread_create(&thread_control, NULL, control_function, NULL);

    // Wait for threads to finish
    pthread_join(thread_A, NULL);
    pthread_join(thread_B, NULL);
    pthread_join(thread_control, NULL);

    // Record end time and calculate elapsed time
    time_t end_time = clock();
    double elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC * 1000.0;

    // Display results
    printf("running_times : %.4f ms\n", elapsed_time);
    sem_destroy(&sem_A);
    sem_destroy(&sem_B);

    return 0;
}