#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define NUM_ACCOUNTS 2
#define INITIAL_BALANCE 1000.0

typedef struct {
    int account_id;
    double balance;
    pthread_mutex_t lock;
} Account;

Account accounts[NUM_ACCOUNTS];
int transactions_completed = 0;
pthread_mutex_t count_lock = PTHREAD_MUTEX_INITIALIZER;

// TODO 1: Implement transfer function that causes deadlock
void transfer_deadlock(int from_id, int to_id, double amount) {
    printf("Thread %lu: Attempting transfer %d -> %d\n", (unsigned long)pthread_self(), from_id, to_id);

    // Lock first account
    pthread_mutex_lock(&accounts[from_id].lock);
    printf("Thread %lu: Locked Account %d\n", (unsigned long)pthread_self(), from_id);

    // Artificial delay to ensure the other thread grabs its first lock
    usleep(100000);

    // Lock second account
    printf("Thread %lu: Waiting for Account %d...\n", (unsigned long)pthread_self(), to_id);
    pthread_mutex_lock(&accounts[to_id].lock);

    // Critical section
    if (accounts[from_id].balance >= amount) {
        accounts[from_id].balance -= amount;
        accounts[to_id].balance += amount;

        pthread_mutex_lock(&count_lock);
        transactions_completed++;
        pthread_mutex_unlock(&count_lock);
    }

    pthread_mutex_unlock(&accounts[to_id].lock);
    pthread_mutex_unlock(&accounts[from_id].lock);
}

// TODO 2: Thread functions to create circular wait
void* thread_a(void* arg) {
    transfer_deadlock(0, 1, 50.0); // Locks 0, wants 1
    return NULL;
}

void* thread_b(void* arg) {
    transfer_deadlock(1, 0, 50.0); // Locks 1, wants 0
    return NULL;
}

int main() {
    pthread_t t1, t2;

    // Initialize
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        accounts[i].account_id = i;
        accounts[i].balance = INITIAL_BALANCE;
        pthread_mutex_init(&accounts[i].lock, NULL);
    }

    printf("=== Phase 3: Deadlock Detection ===\n");
    pthread_create(&t1, NULL, thread_a, NULL);
    pthread_create(&t2, NULL, thread_b, NULL);

    // TODO 3: Deadlock detection logic
    int timeout = 0;
    while (timeout < 5) {
        sleep(1);
        timeout++;

        pthread_mutex_lock(&count_lock);
        if (transactions_completed >= 2) {
            pthread_mutex_unlock(&count_lock);
            break;
        }
        pthread_mutex_unlock(&count_lock);

        printf("Monitoring... (%d seconds elapsed)\n", timeout);
    }

    if (transactions_completed < 2) {
        printf("\n DEADLOACK DETECTED!\n");
        printf("Threads are stuck in Circular Wait. Main will now exit.\n");
    } else {
        printf("Transfers completed successfully. \n");
    }

    return 0;
}


// Key Concepts
// Four Coffman conditions for deadlock
// Mutual exclusion, hold and wait, no preemption, circular wait is all aprt of the four Coffman conditions
// Resouces allocation graphs
// Is a grpahical tool to visualize the allocation of resources to processes for deadlock analysis
// Circular wait condition
// Occurs when a closed chain of processes exists where each process holds a resource required by the next proccess

























