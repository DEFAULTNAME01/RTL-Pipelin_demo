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

DataStructure A_register, B_register, C_even_register,C_odd_register;
DataStructure A_iregister, B_iregister, C_iregister;
DataStructure D_register;
DataStructure D_even_iregister, D_odd_iregister;

int input_data[INPUT_SIZE] = { 2, 3, 18, 9, 1, 34, 52, 11, 26, 8, 12, 44, 8, 97, 10, 22, 31, 4, 6, 48, 5, 0, 13, 49, 7 };
bool continuous_run = false;
bool program_terminated = false;
sem_t myclk;
pthread_mutex_t mutex_D;

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
        printf("First element of structure at \nA:%d B:%d D:%d C1:%d C2:%d\n", A_register.data[0], B_register.data[0], D_register.data[0], C_odd_register.data[0], C_even_register.data[0]);
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
            C_iregister.data[i] = B_register.data[i]-1;
        }
        B_register.valid = false;
        if (C_iregister.data[1]% 2 == 0)
        {
                    C_even_register = C_iregister;
                    C_even_register.valid = true;
        }
        else{
             C_odd_register = C_iregister;
             C_odd_register.valid = true;
        }
        
      
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
             if (chr == '\n') {
                usleep(100);
                sem_post(&myclk);
           
            }else if (chr == 's' || chr == 'S') {
            program_terminated = true;
            sem_post(&myclk);
            break;
            }
            
        }
        break;
        
    }
    }
    return NULL;
}

void* function_D_even(void* args) {
    while (!program_terminated) {
        if (!C_even_register.valid) {
            continue;
        }
        sem_wait(&myclk);
        
        for (int i = 0; i < ARRAY_SIZE; ++i) {
            D_even_iregister.data[i] = C_even_register.data[i] - 1;  // decrement by 1 for even values
        }
        C_even_register.valid = false;
        C_odd_register.valid = false;
     pthread_mutex_lock(&mutex_D);  // Lock the mutex
        D_register = D_even_iregister;
        D_register.valid = true;
        pthread_mutex_unlock(&mutex_D);  // Unlock the mutex
    }
    return NULL;
}

void* function_D_odd(void* args) {
    while (!program_terminated) {
        if (!C_odd_register.valid) {
            continue;
        }
        sem_wait(&myclk);
        for (int i = 0; i < ARRAY_SIZE; ++i) {
            D_odd_iregister.data[i] = C_odd_register.data[i] + 1;  // increment by 1 for odd values
        }
        C_odd_register.valid = false;
        C_even_register.valid = false;
        pthread_mutex_lock(&mutex_D);  // Lock the mutex
        D_register = D_odd_iregister;
        D_register.valid = true;
        pthread_mutex_unlock(&mutex_D);  // Unlock the mutex
    }
    return NULL;
}

int main() {
    pthread_t thread_A, thread_B, thread_C, control_and_clocks_thread, thread_D_even, thread_D_odd;

    // Initialize semaphore
    sem_init(&myclk, 0, 0);
    pthread_mutex_init(&mutex_D, NULL);

    // Create threads
    pthread_create(&thread_A, NULL, function_A, NULL);
    pthread_create(&thread_B, NULL, function_B, NULL);
    pthread_create(&thread_C, NULL, function_C, NULL);
    pthread_create(&control_and_clocks_thread, NULL, control_and_clocks, NULL);
    pthread_create(&thread_D_even, NULL, function_D_even, NULL);
    pthread_create(&thread_D_odd, NULL, function_D_odd, NULL);

    // Wait for threads to finish
    pthread_join(thread_A, NULL);
    pthread_join(thread_B, NULL);
    pthread_join(thread_C, NULL);
    pthread_join(control_and_clocks_thread, NULL);
    pthread_join(thread_D_even, NULL);
    pthread_join(thread_D_odd, NULL);

    // Destroy semaphore
    sem_destroy(&myclk);
    pthread_mutex_destroy(&mutex_D);

    return 0;
}
