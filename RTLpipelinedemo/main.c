#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <stdbool.h>
#include <unistd.h>

#define ARRAY_SIZE 5
#define INPUT_SIZE 25

typedef struct {
    int data[ARRAY_SIZE];
    bool valid;
} DataStructure;

DataStructure A_register, B_register, C_register;
DataStructure A_iregister, B_iregister, C_iregister;
int input_data[INPUT_SIZE] = { 2, 3, 18, 9, 1, 34, 52, 11, 26, 8, 12, 44, 8, 97, 10, 22, 31, 4, 6, 48, 5, 0, 13, 49, 7 };
bool continuous_run = false;
bool program_terminated = false;
sem_t myclk;

void* function_A(void* args) {
    int index = 0;
    while (!program_terminated) {
        

        for (int i = 0; i < ARRAY_SIZE; ++i) {
            A_iregister.data[i] = input_data[index % INPUT_SIZE];
            index++;
        }
        sem_wait(&myclk);
        A_register = A_iregister;
        A_register.valid = true;
        printf("First element of structure at \nA:%d B:%d C:%d\n", A_register.data[0], B_register.data[0], C_register.data[0]);
    }
    return NULL;
}

void* function_B(void* args) {
    while (!program_terminated) {
       

        if (!A_register.valid) {
            continue;
        }
        sem_wait(&myclk);
        for (int i = 0; i < ARRAY_SIZE; ++i) {
            B_iregister.data[i] = A_register.data[i] - 1;
        }
        A_register.valid = false;
      
        

        B_register = B_iregister;
        B_register.valid = true;
    }
    return NULL;
}

void* function_C(void* args) {
    while (!program_terminated) {
       

        if (!B_register.valid) {
            continue;
        }
        sem_wait(&myclk);
        for (int i = 0; i < ARRAY_SIZE; ++i) {
            C_iregister.data[i] = B_register.data[i] - 1;
        }
        B_register.valid = false;
        
        C_register = C_iregister;
        C_register.valid = true;
    }
    return NULL;
}

void* control_and_clocks(void* arg) {
    int ch,chr; 
    ch = getchar();
    while (!program_terminated) {
       

        
        if (ch == 'p' || ch == 'P') {
           
            chr = getchar();
            while(true){usleep(100);
            
            sem_post(&myclk);
            if (chr == 's' || chr == 'S') {
            program_terminated = true;
            break;
            }
            
            }break;
        }
        if (ch == 'l' || ch == 'L') {
            
            
            while(true){
                chr= getchar();
             if (chr == '\n') {usleep(100);
            sem_post(&myclk);
           
            }
            if (chr == 's' || chr == 'S') {
            program_terminated = true;
            break;
            }
            
        }
        break;
        
    }
    }
    return NULL;
}



int main() {
    pthread_t thread_A, thread_B, thread_C, control_and_clocks_thread;

    // Initialize semaphore
    sem_init(&myclk, 0, 0);

    // Create threads
    pthread_create(&thread_A, NULL, function_A, NULL);
    pthread_create(&thread_B, NULL, function_B, NULL);
    pthread_create(&thread_C, NULL, function_C, NULL);
    pthread_create(&control_and_clocks_thread, NULL, control_and_clocks, NULL);

    // Wait for threads to finish
    pthread_join(thread_A, NULL);
    pthread_join(thread_B, NULL);
    pthread_join(thread_C, NULL);
    pthread_join(control_and_clocks_thread, NULL);

    // Destroy semaphore
    sem_destroy(&myclk);

    return 0;
}



